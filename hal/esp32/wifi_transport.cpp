/*
 *  WiFi TCP transport for Chronos protocol.
 *
 *  Architecture:
 *    Android device = WiFi AP + TCP server on port 8423
 *    ESP32          = WiFi STA + TCP client → gateway:8423
 *
 *  TCP framing: [2-byte BE length][payload]
 *  Payload is an assembled Chronos packet (0xAB… or 0xEA…), identical to what
 *  the BLE NUS path would deliver after reassembly.
 *
 *  We inject each complete packet into ChronosESP32's internal parser so that
 *  all existing getters, callbacks, and UI code work unchanged.
 */

#ifdef ENABLE_WIFI_TRANSPORT

/* ── Include system / Arduino headers FIRST (normal access specifiers) ──── */
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <Preferences.h>
#include <NimBLEDevice.h>
#include <ESP32Time.h>
#include <timber.h>

/* ── Expose ChronosESP32 internals for packet injection ─────────────────── */
#define private   public
#define protected public
#include <ChronosESP32.h>
#undef  private
#undef  protected

#include "wifi_transport.h"

/* ── External objects from app_hal.cpp ──────────────────────────────────── */
extern ChronosESP32 watch;
extern Preferences  prefs;

/* ── Constants ─────────────────────────────────────────────────────────── */
static const uint16_t TCP_PORT            = 8423;
static const size_t   MAX_PACKET          = 2048;
static const uint32_t WIFI_RETRY_MS       = 5000;
static const uint32_t TCP_RETRY_MS        = 2000;
static const uint32_t TCP_READ_TIMEOUT_MS = 5000;

/* ── State ─────────────────────────────────────────────────────────────── */
static WiFiClient    tcpClient;
static bool          wifiEnabled    = false;
static bool          tcpConnected   = false;
static uint32_t      lastRetryMs    = 0;
static String        apSsid;
static String        apPass;

/* Packet reassembly buffer (length-prefix state machine) */
static uint8_t       pktBuf[MAX_PACKET];
static uint16_t      pktExpected = 0;
static uint16_t      pktReceived = 0;
static bool          haveHeader  = false;

/* ── Inject a complete Chronos packet into ChronosESP32 ────────────────── */
static void chronos_inject(const uint8_t *data, int len)
{
    if (len <= 0 || len > (int)sizeof(watch._incomingData.data))
        return;

    if (watch.rawDataReceivedCallback != nullptr)
        watch.rawDataReceivedCallback((uint8_t *)data, len);

    bool isFirst = (data[0] == 0xAB || data[0] == 0xEA) &&
                   (data[3] == 0xFE || data[3] == 0xFF);
    if (!isFirst)
        return;

    watch._incomingData.length = data[1] * 256 + data[2] + 3;
    int cpLen = (len < (int)sizeof(watch._incomingData.data))
                    ? len
                    : (int)sizeof(watch._incomingData.data);
    memcpy(watch._incomingData.data, data, cpLen);

    if (watch._incomingData.length <= len)
        watch.dataReceived();
}

/* ── WiFi event handler ───────────────────────────────────────────────── */
static void onWiFiEvent(WiFiEvent_t event)
{
    switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Timber.i("WiFi connected, IP %s  GW %s",
                 WiFi.localIP().toString().c_str(),
                 WiFi.gatewayIP().toString().c_str());
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Timber.w("WiFi disconnected");
        tcpConnected = false;
        tcpClient.stop();
        break;
    default:
        break;
    }
}

/* ── Public API ────────────────────────────────────────────────────────── */

void wifi_transport_early_init(void)
{
    apSsid      = prefs.getString("wifi_ssid", "CarLinkAP467");
    apPass      = prefs.getString("wifi_pass", "carlink324");
    wifiEnabled = prefs.getBool("wifi_en", true);

    if (!wifiEnabled || apSsid.length() == 0) {
        Timber.i("WiFi transport disabled (wifi_en=%d ssid='%s')",
                 wifiEnabled, apSsid.c_str());
        return;
    }

    Timber.i("WiFi radio init → SSID '%s'", apSsid.c_str());
    WiFi.onEvent(onWiFiEvent);
    WiFi.mode(WIFI_STA);
    WiFi.begin(apSsid.c_str(), apPass.c_str());
    lastRetryMs = millis();
}

void wifi_transport_init(void)
{
    /* no-op: radio already started by early_init, TCP handled in loop */
}

bool wifi_transport_connected(void)
{
    return tcpConnected && tcpClient.connected();
}

void wifi_transport_loop(void)
{
    if (!wifiEnabled || apSsid.length() == 0)
        return;

    uint32_t now = millis();

    /* ---- WiFi STA reconnect ---- */
    if (WiFi.status() != WL_CONNECTED) {
        tcpConnected = false;
        if (now - lastRetryMs > WIFI_RETRY_MS) {
            lastRetryMs = now;
            WiFi.disconnect();
            WiFi.begin(apSsid.c_str(), apPass.c_str());
        }
        return;
    }

    /* ---- TCP connect ---- */
    if (!tcpClient.connected()) {
        tcpConnected = false;
        if (now - lastRetryMs > TCP_RETRY_MS) {
            lastRetryMs = now;
            IPAddress gw = WiFi.gatewayIP();
            Timber.i("TCP connecting → %s:%u", gw.toString().c_str(), TCP_PORT);
            if (tcpClient.connect(gw, TCP_PORT)) {
                tcpConnected = true;
                haveHeader   = false;
                pktExpected  = 0;
                pktReceived  = 0;
                Timber.i("TCP connected");
            } else {
                Timber.w("TCP connect failed");
            }
        }
        return;
    }

    /* ---- Read length-prefixed packets (non-blocking) ---- */
    while (tcpClient.available()) {
        if (!haveHeader) {
            if (tcpClient.available() < 2)
                break;
            uint8_t hdr[2];
            tcpClient.readBytes(hdr, 2);
            pktExpected = ((uint16_t)hdr[0] << 8) | hdr[1];
            if (pktExpected == 0 || pktExpected > MAX_PACKET) {
                Timber.w("WiFi bad pktLen %u — skip", pktExpected);
                pktExpected = 0;
                continue;
            }
            pktReceived = 0;
            haveHeader  = true;
        }

        if (haveHeader) {
            int avail = tcpClient.available();
            if (avail <= 0)
                break;
            int want = pktExpected - pktReceived;
            if (avail < want)
                want = avail;
            int got = tcpClient.readBytes(pktBuf + pktReceived, want);
            pktReceived += got;

            if (pktReceived >= pktExpected) {
                chronos_inject(pktBuf, pktExpected);
                haveHeader  = false;
                pktExpected = 0;
                pktReceived = 0;
            }
        }
    }
}

#endif /* ENABLE_WIFI_TRANSPORT */
