# Chronos-Compatible Smartwatch Firmware — Technical Keynote

**Repository layout:** the **vantc-navi** tree hosts the **Android companion** only. Build and flash watch firmware from **[vantechcorner/esp32-c3-mini](https://github.com/vantechcorner/esp32-c3-mini)**. The BLE / packet notes below still describe what that firmware and **ChronosESP32** expect on the wire.

**Purpose:** Deep reference for engineers rewriting the **Android companion app** and **device UI** on **ESP32**, **ESP32-S3**, or **ESP32-P4**, while staying compatible with (or deliberately forking) the **Chronos** ecosystem used by this repository.

**Scope:** Describes the **current firmware stack**, **how phone data reaches the display**, and **canonical source URLs**. Packet-level behavior is summarized from the open-source **ChronosESP32** library (v1.9.x), which is the authoritative BLE parser on the device side.

---

## 1. Primary repositories and links

| Resource | URL | Role |
|----------|-----|------|
| **Watch firmware (VANTC / Navigation fork)** | [https://github.com/vantechcorner/esp32-c3-mini](https://github.com/vantechcorner/esp32-c3-mini) | LVGL 9 UI, board HAL, Navigation app consuming VANTC `0xED` polyline packets from this Android app |
| **This firmware project (upstream)** | [https://github.com/fbiego/esp32-c3-mini](https://github.com/fbiego/esp32-c3-mini) | Upstream LVGL watch project **vantechcorner** fork extends |
| **BLE / phone protocol library** | [https://github.com/fbiego/chronos-esp32](https://github.com/fbiego/chronos-esp32) | `ChronosESP32` class: NimBLE GATT server, packet reassembly, `Navigation` / `Notification` / weather structs, callbacks |
| **Chronos web tools / install** | [https://chronos.ke/c3-ui#install](https://chronos.ke/c3-ui#install) | Web flasher and project landing (as referenced in project README) |
| **Chronos app (Android)** | [https://chronos.ke/app?id=c3-mini](https://chronos.ke/app?id=c3-mini) | Closed-source companion; implements **BLE central** and encodes the binary protocol the library decodes |
| **LVGL** | [https://github.com/lvgl/lvgl](https://github.com/lvgl/lvgl) | UI toolkit (this project pins **9.3.0** in `platformio.ini`) |
| **LovyanGFX** | [https://github.com/lovyan03/LovyanGFX](https://github.com/lovyan03/LovyanGFX) | Display/touch abstraction for many ESP32 boards |
| **Watchface tooling** | [https://github.com/fbiego/esp32-lvgl-watchface](https://github.com/fbiego/esp32-lvgl-watchface) | Converting watchface binaries to LVGL (LVGL 8 lineage; LVGL 9 path still WIP in README) |

If you work from a **fork** of `esp32-c3-mini`, treat **`chronos-esp32`** as the **single source of truth** for byte-level compatibility with the stock Chronos app, unless you intentionally version a new protocol.

---

## 2. Firmware stack (what **esp32-c3-mini** builds)

The **vantc-navi** repo no longer vendors that tree; clone **[vantechcorner/esp32-c3-mini](https://github.com/vantechcorner/esp32-c3-mini)** to build firmware. The following still documents that stack for BLE/protocol work.

### 2.1 Build system and platform

- **PlatformIO** (`platformio.ini`)
- **Platform:** `espressif32@6.9.0` (Arduino-ESP32 core 3.x line bundled with that platform release)
- **Framework:** `arduino` (not ESP-IDF baremetal in the main ESP32 paths)
- **Typical toolchains:** Xtensa (ESP32 / ESP32-S3), RISC-V (ESP32-C3 / some boards)

### 2.2 Core libraries (from `[esp32]` env in `platformio.ini`)

| Library | Registry / version (typical) | Function |
|---------|------------------------------|----------|
| **LVGL** | `lvgl/lvgl@9.3.0` | Screens, widgets, fonts, themes |
| **ChronosESP32** | `fbiego/ChronosESP32@1.9.0` | BLE server + protocol + structured phone data |
| **LovyanGFX** | `lovyan03/LovyanGFX@1.1.16` | Panel init, SPI/RGB, touch |
| **Timber** | `fbiego/Timber@2.0.0` | Logging |
| **ArduinoJson** | `bblanchon/ArduinoJson@7.1.0` | JSON where used |
| **Button2** | `lennarthennigs/Button2@2.4.1` | Physical buttons |

ChronosESP32 **depends on NimBLE** (Arduino-ESP32 NimBLE stack) for GATT — see library implementation.

### 2.3 Project layout (conceptual)

- **`src/`** — Application UI generated / maintained (e.g. `src/ui/ui.c`), feature apps under `src/apps/*`
- **`hal/esp32/`** — **Hardware abstraction**: `app_hal.cpp` (main loop, Chronos integration, LVGL flush, touch), board headers under `hal/esp32/displays/`
- **`include/lv_conf.h`** — LVGL configuration
- **Preprocessor board flags** — e.g. `ESP32_TOUCH_LCD_35`, `WAVESHARE_S3_LCD_154`, `TTGO_TDISPLAY` select drivers and UI branches

### 2.4 Display pipeline (ESP32)

1. **LVGL** renders to a color buffer (size/layout from `lv_conf.h` and `app_hal.cpp`).
2. **Flush callback** pushes pixels to the panel (often **DMA** `pushImageDMA` with LovyanGFX).
3. **Optional software rotation** — some builds rotate the draw buffer in the flush path when the panel is mounted in portrait but UI is logical landscape (or vice versa).
4. **Touch** — `my_touchpad_read()` feeds LVGL: local touch from the panel **or** **remote touch** from the phone when BLE is connected (see §4.4).

---

## 3. Phone connectivity: BLE roles and GATT surface

### 3.1 Roles

- **ESP32 firmware:** BLE **peripheral** (GATT **server**).
- **Android Chronos app:** BLE **central** (GATT **client**): scans, connects, writes commands, subscribes to notifications.

### 3.2 Nordic UART–style service UUIDs (fixed in `ChronosESP32.h`)

From [chronos-esp32 `ChronosESP32.h`](https://github.com/fbiego/chronos-esp32/blob/main/src/ChronosESP32.h):

```text
SERVICE_UUID           = 6e400001-b5a3-f393-e0a9-e50e24dcca9e
CHARACTERISTIC_UUID_RX = 6e400002-b5a3-f393-e0a9-e50e24dcca9e   (phone → watch: WRITE)
CHARACTERISTIC_UUID_TX = 6e400003-b5a3-f393-e0a9-e50e24dcca9e   (watch → phone: NOTIFY)
```

**MTU:** library calls `BLEDevice::setMTU(517)` in `begin()` to allow larger payloads where the phone supports it.

### 3.3 Wire protocol: framing and reassembly

**Entry point:** `ChronosESP32::onWrite` when the central writes the RX characteristic.

**First segment rule** (paraphrased from `ChronosESP32.cpp`):

- If `(data[0] == 0xAB || data[0] == 0xEA)` **and** `(data[3] == 0xFE || data[3] == 0xFF)`:
  - Total length: `_incomingData.length = data[1] * 256 + data[2] + 3`
  - Copy into `_incomingData.data[]`
  - If the written chunk already contains the full packet → call `dataReceived()`

**Continuation segments:**

- Else: continuation bytes are copied at offset `j = 20 + (pData[0] * 19)` into `_incomingData.data` (fragmented transfer).

Any **new Android implementation** must reproduce this **exact framing** if it targets unmodified `ChronosESP32` on the watch.

After assembly, `dataReceived()` dispatches on `_incomingData.data[0]`:

- **`0xAB`** — large family of config/control opcodes (`switch (_incomingData.data[4])`)
- **`0xEA`** — extended family including weather, etc. (`switch (_incomingData.data[4])` in a second branch)

The firmware rarely needs to parse raw bytes itself: it consumes **parsed state** via getters and **callbacks**.

---

## 4. How data becomes pixels (end-to-end)

### 4.1 Lifecycle in firmware (`hal/esp32/app_hal.cpp` pattern)

1. **Construct** `ChronosESP32 watch("Chronos C3");` (name may be replaced with `BOARD_NAME`).
2. **`watch.setScreen(ChronosScreen …)`** on supported boards so the **phone knows logical geometry** for remote touch and watchface tooling (enum `ChronosScreen` in `ChronosESP32.h`).
3. **Register callbacks** (before or after `begin` per API):
   - `setConnectionCallback` — link up/down
   - `setNotificationCallback` — new notification payload
   - `setConfigurationCallback` — **typed config events** (`enum Config`: time, weather, nav, …)
   - `setRingerCallback` — incoming call UI
   - `setDataCallback` / `setRawDataCallback` — raw passthrough (project uses raw for optional watchface chunk protocol)
4. **`watch.begin()`** — starts advertising, creates service/characteristics.
5. **Main loop:** call **`watch.loop()`** every cycle (alongside **`lv_timer_handler()`**).

### 4.2 Time, weather, battery

- **Time sync:** central sends packets that update internal clock; `CF_TIME` fires `configCallback`; UI reads `watch.getHourC()`, `getTime()`, etc.
- **Weather:** `CF_WEATHER` and related branches populate `watch.getWeatherAt(i)`, city, hourly forecast structs.
- **Phone battery:** `CF_PBAT` path updates phone charge state; UI uses `watch.getPhoneBattery()`, `isPhoneCharging()`.

### 4.3 Notifications and calls

- **Notification:** `setNotificationCallback` receives a `Notification` struct (`icon`, `app`, `time`, `title`, `message`).  
  **Critical for Android / LVGL threading:** in this project, the callback **does not** call LVGL directly; it sets flags and the **main loop** runs `showAlert()` after `lv_timer_handler()` (NimBLE host task must not block on UI).
- **Incoming call:** `setRingerCallback(String caller, bool state)` drives the call screen.

### 4.4 Remote touch (phone → watch)

When connected, the phone can inject touch coordinates. The library stores a `RemoteTouch { state, x, y }` updated from opcode **`0xBF` / `0xFE`** (see `ChronosESP32.cpp`).

**Firmware:** `watch.getTouch()` is read inside `my_touchpad_read()`. If `watch.isConnected() && rt.state`, those coordinates override (or merge with) panel touch.

**Implication for new Android UI:** remote touch must send packets in the **same layout** the library expects, and **`watch.setScreen()` must match** the watch’s resolution and rotation model or touches will map incorrectly.

---

## 5. Navigation: data model and binary format (critical for Android Maps integration)

### 5.1 `Navigation` struct (device-side)

From `ChronosESP32.h`:

| Field | Type | Meaning |
|-------|------|---------|
| `active` | `bool` | Session on/off |
| `isNavigation` | `bool` | Turn-by-turn vs generic info |
| `hasIcon` | `bool` | Whether a 48×48 1-bpp icon is present |
| `distance` | `String` | Distance to next / destination (app-defined text) |
| `duration` | `String` | Time-to-destination style text |
| `eta` | `String` | ETA string |
| `title` | `String` | Often “distance to maneuver” or road name |
| `directions` | `String` | Instruction / street / place line |
| `speed` | `String` | Optional (e.g. OsmAnd) |
| `icon` | `uint8_t[288]` | **48×48 pixels, 1 bit per pixel** (`ICON_DATA_SIZE = 48*48/8`) |
| `iconCRC` | `uint32_t` | Change detection |

**There is no vector map, polyline, or lat/lng stream** in this protocol — only **strings + optional 1-bpp bitmap**. Rich map UI on the watch must be **locally synthesized** or the protocol must be **extended** on both app and firmware.

### 5.2 Navigation **text** packet — opcode family `0xAB`, case `0xEF`, sub `0xFE`

When `_incomingData.data[0] == 0xAB` and the inner switch hits **`0xEF`** with `_incomingData.data[3] == 0xFE`, the library parses **status byte** `_incomingData.data[5]`:

| `data[5]` | Behavior |
|-----------|----------|
| `0x00` | **Inactive:** clears navigation, sets placeholder strings (“Start navigation on Google maps”, etc.) |
| `0xFF` | **Disabled in app:** active but user-facing “check app settings” placeholders |
| `0x80` | **Live update:** reads flags, CRC, then **null-terminated UTF-8(ish) strings** in order: `title`, `duration`, `distance`, `eta`, `directions`, `speed` |

For `0x80`, additional header bytes before strings (from library):

- `data[6]` → `hasIcon` (1 = true)
- `data[7]` → `isNavigation` (1 = true)
- `data[8..11]` → big-endian `iconCRC`

Then **C-style string fields** separated by `0` bytes until all fields consumed.

After parsing, the library invokes:

```cpp
configurationReceivedCallback(CF_NAV_DATA, _navigation.active ? 1 : 0, 0);
```

### 5.3 Navigation **icon** packet — `0xEE` / `0xFE`

Icon data arrives in **chunks**:

- `data[6]` = chunk index `pos`
- `data[7..10]` = CRC (big-endian) for that transfer
- `data[11..106]` = **96 bytes** written into `_navigation.icon[pos * 96 + i]`

Total bitmap: **288 bytes** = 48×48 @ 1 bpp.

Callback:

```cpp
configurationReceivedCallback(CF_NAV_ICON, pos, crc);
```

**Note:** `ChronosESP32` passes **`configurationReceivedCallback(CF_NAV_ICON, pos, crc)`** where **`pos`** is the chunk index (0-based; **96 bytes per chunk**, **288 bytes total** = three chunks, indices **0, 1, 2**). In `esp32-c3-mini`’s `app_hal.cpp`, **`navIcChanged` is set only when `a == 2`**, i.e. after the **final** chunk of the 48×48 icon arrives, then the full bitmap is copied to the LVGL canvas.

### 5.4 How this repository turns Navigation into LVGL

1. `configCallback` sets `navChanged` / `navIcChanged` on `CF_NAV_DATA` / `CF_NAV_ICON`.
2. Main loop copies `nav = watch.getNavigation()`.
3. Builds a **summary label** `navText = nav.eta + "\n" + nav.duration + " " + nav.distance` for the first line block.
4. Calls `navigateInfo(navText, nav.title, nav.directions)` in `src/apps/navigation/navigation.c`.
5. Paints **48×48** bits into an LVGL **1-bpp canvas** via `setNavIconPx`.

Board-specific **Navigation V2** (e.g. large landscape panel) vs **legacy** layout is selected at compile time (`NAVIGATION_UI_LEGACY`); the **phone payload is identical**.

---

## 6. Other notable device → phone / phone → device flows

### 6.1 Music and controls

`watch.musicControl(MUSIC_TOGGLE | MUSIC_NEXT | …)` sends commands defined in `enum Control` (`0x9Dxx`, `0x99xx` ranges in `ChronosESP32.h`). The Android app must implement the corresponding handling.

### 6.2 Find phone / camera / QR / contacts

All are driven by the same **`0xAB` / `0xEA`** opcode matrix inside `dataReceived()`. For a full reimplementation, trace each `case` in:

- [https://github.com/fbiego/chronos-esp32/blob/main/src/ChronosESP32.cpp](https://github.com/fbiego/chronos-esp32/blob/main/src/ChronosESP32.cpp)  
  Function: **`ChronosESP32::dataReceived()`**

### 6.3 Custom watchface binary transfer (optional)

`rawDataCallback` in this project recognizes **`0xB0`** (chunk metadata) and **`0xAF`** (chunk payload) for streaming a file to flash. This is **orthogonal** to Chronos “stock” features but shows how **`sendCommand`** acks can pace transfers.

---

## 7. `ChronosScreen` — why it matters for new hardware

`watch.setScreen(ChronosScreen)` identifies **resolution class** to the app (watch info UI, remote touch scaling, future watchface loads). Examples from `ChronosESP32.h`:

- `CS_135x240_114_RTF` — TTGO T-Display style
- `CS_240x296_191_RTF` — some 240×296 panels
- `CS_410x494_200_RTF` — large rectangular
- `CS_466x466_143_CTF` — round 466×466
- `CF_ESP32_240x240`, `CF_WATCHY_200x200`, … — extended IDs

**ESP32-Touch-LCD-3.5** in this fork may rely on **default** screen id if no `setScreen` is defined for that board — check `app_hal.cpp` for your branch.

**For ESP32-P4 or custom boards:** you will likely add a **new enum value** and teach the **Android** side the resolution and touch mapping, or accept generic behavior.

---

## 8. WiFi TCP Transport — specification for firmware implementation

### 8.1 Architecture overview

For devices without BLE (e.g. OLEDPRO X4S ECO), the Android app can communicate with ESP32 watches over **WiFi TCP** instead of BLE NUS.

```
┌──────────────────────────────┐     WiFi AP     ┌───────────────────────┐
│  Android (OLEDPRO / Phone)   │  ◄──────────►   │  ESP32 (WiFi client)  │
│                              │                  │                       │
│  WiFi Hotspot (AP mode)      │                  │  Connects to AP       │
│  TCP Server on port 8423     │                  │  TCP Client → GW:8423 │
│                              │   TCP stream     │                       │
│  WifiTransport.writePacket() │ ──────────────►  │  Read loop → parser   │
└──────────────────────────────┘                  └───────────────────────┘
```

**Roles:**
- **Android device** = WiFi Access Point (hotspot) + TCP **server** (listens on port `8423`).
- **ESP32** = WiFi **station** (connects to the AP) + TCP **client** (connects to gateway IP on port `8423`).

The Android device IS the AP, so the gateway IP is always known (typically `192.168.43.1` on Android hotspot, or `192.168.x.1` on custom APs). No mDNS or manual IP configuration is needed.

### 8.2 TCP connection flow

1. User enables WiFi hotspot on the Android device.
2. User taps **"Start Server"** in the app → `ServerSocket(8423)` binds and listens.
3. ESP32 firmware connects to the AP's SSID/password (configurable in firmware settings).
4. ESP32 opens a TCP socket to `gateway_ip:8423`.
5. Android `ServerSocket.accept()` returns → connection established.
6. Android sends Chronos protocol packets (same `ByteArray` payloads as BLE) over the TCP stream.
7. ESP32 reads packets and feeds them to the `ChronosESP32` parser (same `onWrite` path).

### 8.3 Packet framing (length-prefixed)

BLE NUS has natural packet boundaries (each GATT write is one PDU). TCP is a **byte stream** with no message boundaries. Therefore, packets are **length-prefixed**:

```
┌──────────────┬──────────────────────────┐
│  2 bytes     │  N bytes                 │
│  Length (BE) │  Payload                 │
├──────────────┼──────────────────────────┤
│  0x00 0x0E   │  AB 00 0B FE 93 00 00   │ ← example: 14-byte time sync packet
│              │  07 F6 05 0D 0E 1E 00   │
└──────────────┴──────────────────────────┘
```

**Length field:** 2-byte big-endian unsigned integer = number of payload bytes that follow.  
**Maximum payload:** 65535 bytes (theoretical; practical Chronos packets are < 500 bytes).

**Firmware read loop pseudocode (Arduino / ESP-IDF):**

```cpp
// After TCP client.connect(gateway, 8423) succeeds:
WiFiClient client;

void tcpReadLoop() {
    while (client.connected()) {
        // 1. Read 2-byte length header
        if (client.available() < 2) { delay(1); continue; }
        uint8_t hdr[2];
        client.readBytes(hdr, 2);
        uint16_t pktLen = (hdr[0] << 8) | hdr[1];
        if (pktLen == 0 || pktLen > 2048) continue; // sanity check

        // 2. Read exactly pktLen bytes of payload
        uint8_t buf[2048];
        size_t received = 0;
        unsigned long t0 = millis();
        while (received < pktLen && (millis() - t0) < 5000) {
            if (client.available()) {
                size_t n = client.readBytes(buf + received, pktLen - received);
                received += n;
            } else {
                delay(1);
            }
        }
        if (received != pktLen) continue; // incomplete, discard

        // 3. Feed into ChronosESP32 parser — same path as BLE onWrite
        //    The payload is a complete Chronos packet (0xAB... or 0xEA...),
        //    NO Chronos fragmentation needed (unlike BLE MTU splitting).
        watch.rawReceive(buf, pktLen);
    }
}
```

**Critical difference from BLE:** Over TCP, each payload is a **complete** Chronos packet. The BLE-specific multi-segment fragmentation (`offset = 20 + pData[0] * 19`) is **NOT used**. The firmware should call the packet parser directly with the full buffer, bypassing the `onWrite` reassembly logic.

If `ChronosESP32` does not expose a public `rawReceive()` method, you can:
1. Call `dataReceived()` directly after copying into `_incomingData` (requires library modification).
2. Or simulate a single BLE write where `len >= total packet length` (the existing `onWrite` handles this case as a single-segment packet and calls `dataReceived()` immediately).

### 8.4 What packets are sent over WiFi

**Exactly the same `ByteArray` payloads as BLE.** The `protocol/` package builders produce transport-agnostic packets. The only difference is the framing:

| Feature | BLE (NUS GATT) | WiFi (TCP) |
|---------|----------------|------------|
| Framing | GATT write = 1 PDU; large packets split with Chronos fragmentation (`j = 20 + idx*19`) | Length-prefixed: `[2B BE len][payload]` |
| MTU | Negotiated (23–517); fragments aligned to MTU-3 | No MTU limit; full packet in one frame |
| Inter-packet delay | 12–40 ms (BLE controller buffer) | Not strictly needed; 5 ms recommended |
| Connection init | GATT connect + MTU negotiate + service discovery | TCP connect to `gateway:8423` |

### 8.5 Firmware WiFi configuration

The ESP32 firmware needs the following configurable parameters (suggest storing in NVS/preferences):

| Parameter | Default | Description |
|-----------|---------|-------------|
| `wifi_ssid` | (none) | AP SSID to connect to |
| `wifi_password` | (none) | AP password |
| `tcp_host` | `"0.0.0.0"` | Gateway IP; `0.0.0.0` = auto-detect gateway from DHCP |
| `tcp_port` | `8423` | TCP server port on the Android device |
| `wifi_enabled` | `false` | Enable WiFi transport (disable BLE advertising when active, or run both) |

**Auto-detect gateway:** After `WiFi.begin(ssid, password)` and `WiFi.waitForConnectResult()`, the gateway IP is available via `WiFi.gatewayIP()`.

### 8.6 BLE disabled in WiFi mode

In practice, ESP32-S3 does **not** have enough heap to run WiFi + NimBLE (BLE) simultaneously with LVGL — `BLE_INIT: Malloc failed` crashes the device. The firmware therefore uses **WiFi-only mode** when `ENABLE_WIFI_TRANSPORT` is defined:

1. `btStop()` is called early in `hal_setup()` to release the BLE radio controller memory.
2. `watch.begin()` (which initializes NimBLE) is **skipped**.
3. The `ChronosESP32` object still exists — its callbacks, getters, and time functions work normally. `sendCommand()` is a safe no-op when `_inited == false`.
4. `wifi_transport_early_init()` runs **before** display and BLE init (right after `prefs.begin()`) to ensure WiFi driver allocates heap first.

This frees ~40 KB of heap for the WiFi driver stack.

### 8.7 Android-side source files

| File | Role |
|------|------|
| [`transport/WatchTransport.kt`](android/app/src/main/java/io/github/vantc/navi/transport/WatchTransport.kt) | Interface: `writePacket()`, `eventFlow`, `isConnected`, `disconnect()` |
| [`transport/WifiTransport.kt`](android/app/src/main/java/io/github/vantc/navi/transport/WifiTransport.kt) | TCP server on port 8423, length-prefixed framing |
| [`transport/BleTransport.kt`](android/app/src/main/java/io/github/vantc/navi/transport/BleTransport.kt) | BLE NUS GATT with Chronos fragmentation |
| [`transport/WatchCommands.kt`](android/app/src/main/java/io/github/vantc/navi/transport/WatchCommands.kt) | Shared commands: `syncTimeFromPhone()`, `writeNavigationWithIcons()`, `writeWeatherSequence()` |
| [`VantcNaviApp.kt`](android/app/src/main/java/io/github/vantc/navi/VantcNaviApp.kt) | Holds `bleTransport`, `wifiTransport`, `activeTransport` based on mode |

---

## 9. Weather protocol — packet details for firmware

### 9.1 Packets sent by the Android app

The app fetches weather from Open-Meteo (no API key) and sends these packets in order:

| # | Opcode path | Purpose | Size |
|---|-------------|---------|------|
| 1 | `0xEA` / `0xFE` / `0x7E` / `0x01` | City name (UTF-8, max 96 bytes) | 7 + strlen |
| 2 | `0xAB` / `0xFE` / `0x7E` | Weekly forecast: 7 days × (icon, temp) | 6 + 14 = 20 |
| 3 | `0xAB` / `0xFE` / `0x88` | Hi/lo temps: 7 days × (high, low) | 6 + 14 = 20 |
| 4 | `0xAB` / `0xFE` / `0x8A` | UV index (%) + pressure (hPa BE 16-bit) | 9 |
| 5 | `0xEA` / `0xFE` / `0x7E` / `0x02` | Hourly forecast: up to 24 hours × (icon, temp) | 7 + 48 = 55 |

Inter-packet delay: 30–40 ms (BLE); 5 ms (WiFi).

### 9.2 Weekly forecast encoding (0x7E)

```
Byte 0:    0xAB
Byte 1-2:  innerLen (BE) = bodyLen - 3
Byte 3:    0xFE
Byte 4:    0x7E
Byte 5:    0x00 (reserved)
Byte 6+:   pairs of 2 bytes per day:
             byte A = (icon << 4) | (temp < 0 ? 1 : 0)
             byte B = abs(temp) clamped to 0..127
```

`icon` = Chronos weather icon ID (0–7), mapped from WMO weather codes.

### 9.3 Hi/Lo encoding (0x88)

Same envelope as 0x7E but with opcode `0x88`. Each pair: `(enc(high), enc(low))` where `enc(v) = abs(v) | (v < 0 ? 0x80 : 0x00)`.

### 9.4 Hourly forecast encoding (0xEA / 0x7E / 0x02)

```
Byte 0:    0xEA
Byte 1-2:  innerLen (BE)
Byte 3:    0xFE
Byte 4:    0x7E
Byte 5:    0x02
Byte 6:    startHour (0–23)
Byte 7+:   pairs of (icon<<4|sign, magnitude) — same encoding as weekly
```

Up to 24 entries. The firmware uses `startHour` to label each entry as `(startHour + i) % 24`.

### 9.5 WMO → Chronos icon mapping

| WMO codes | Chronos icon | Description |
|-----------|--------------|-------------|
| 0, 1 | 0 | Clear / mainly clear |
| 2 | 1 | Partly cloudy |
| 3 | 2 | Overcast |
| 45–48 | 3 | Fog |
| 51–57 | 4 | Drizzle |
| 61–67, 80–82 | 5 | Rain |
| 71–77 | 6 | Snow |
| 95–99 | 7 | Thunderstorm |

---


## 10. Implications for rewriting the Android app

1. **Protocol-first:** Treat `chronos-esp32` `onWrite` + `dataReceived()` as the specification. Mirror packet formats **byte-for-byte** unless you fork both sides.
2. **Transport abstraction:** The Android app uses `WatchTransport` interface. All protocol packets are `ByteArray` produced by `protocol/` builders. Transport-specific framing (BLE MTU fragmentation vs TCP length-prefix) is handled inside each transport implementation.
3. **Threading:** Notifications and BLE callbacks must not touch UI toolkit APIs on Android BLE thread without marshaling to a main/UI thread.
4. **Navigation:** If you need **live maps or polylines**, plan a **new opcode** (e.g. under `0xEA`) and extend `Navigation` or add parallel state.
5. **Testing:** Use a **BLE sniffer** (or TCP packet capture for WiFi) while exercising against stock firmware, then diff against your implementation.
6. **Versioning:** `CF_APP` exposes `getAppCode()` / `getAppVersion()` to gate features.

---

## 11. Implications for ESP32-S3, ESP32, and ESP32-P4 UI work

### 11.1 ESP32 / ESP32-S3 (supported today in this tree)

- **HAL:** Duplicate an existing env in `platformio.ini`, set `board`, pins in `hal/esp32/displays/`, LovyanGFX panel config.
- **LVGL:** Adjust `LV_MEM_SIZE`, buffer size, and rotation to match panel.
- **Chronos:** Call `setScreen()` with the closest `ChronosScreen` or add a new enum in a **forked** library if the app must distinguish the product.

### 11.2 ESP32-P4 (not first-class in this repo as of this document)

- **Toolchain / Arduino support** may differ from ESP32-S3; verify **LovyanGFX** and **arduino-esp32** P4 support before porting.
- **Conceptual split unchanged:** display driver -> LVGL flush -> `ChronosESP32` unchanged if NimBLE + GATT compile on P4.
- Expect **new PIO environment**, **linker script**, and **pin mux** work; BLE radio specifics may differ from classic ESP32.

---

## 12. Firmware-side WiFi transport implementation

The firmware now includes a **WiFi TCP client** that connects to the Android AP and receives Chronos packets. This is implemented as a compile-time option via `ENABLE_WIFI_TRANSPORT`.

### 12.1 Files

| File | Role |
|------|------|
| `hal/esp32/wifi_transport.h` | Public API: `wifi_transport_early_init()`, `wifi_transport_init()`, `wifi_transport_loop()`, `wifi_transport_connected()` |
| `hal/esp32/wifi_transport.cpp` | WiFi STA connection, TCP client, length-prefixed packet reader, injection into `ChronosESP32` |

### 12.2 Injection mechanism

WiFi-received packets are injected directly into the `ChronosESP32` instance's internal parser:

1. Complete payload is copied into `watch._incomingData.data[]`
2. `watch._incomingData.length` is set from the packet header
3. `watch.dataReceived()` is called — triggering all existing callbacks (`configCallback`, `notificationCallback`, etc.)

This means **all existing firmware code** (navigation, weather, time sync, watchface updates) works unchanged over WiFi.

### 12.3 Configuration

WiFi credentials are stored in NVS via `Preferences`:

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `wifi_ssid` | String | `""` | AP SSID to connect to |
| `wifi_pass` | String | `""` | AP password |
| `wifi_en` | bool | `false` | Enable WiFi transport |

Set these before flashing or via serial console. When `wifi_en` is true and SSID is non-empty, the firmware connects to the AP at startup and opens a TCP connection to `gateway:8423`.

### 12.4 PlatformIO environments

| Environment | Board | Flag |
|-------------|-------|------|
| `lolin_s3_mini_1_28_wifi` | ESP32-S3 1.28" | `-D ENABLE_WIFI_TRANSPORT=1` |
| `esp32_touch_lcd_3_5_wifi` | ESP32 Classic 3.5" | `-D ENABLE_WIFI_TRANSPORT=1` |

BLE is **disabled** (`btStop()`) when WiFi is active to free heap (see §8.6). The helper `isPhoneConnected()` in `app_hal.cpp` returns `true` if either transport is connected.

### 12.5 Init order (critical for ESP32-S3)

```
prefs.begin()
  → wifi_transport_early_init()    ← WiFi radio starts FIRST
    → display / LVGL init
      → btStop()                   ← release BLE radio memory
      → skip watch.begin()         ← no NimBLE init
        → hal_loop: wifi_transport_loop()  ← TCP connect + packet read
```

WiFi driver **must** allocate heap before NimBLE. Reversing this order causes `BLE_INIT: Malloc failed` → crash on ESP32-S3.

---

## 13. Quick reference: files to read first

| Goal | File(s) |
|------|---------|
| BLE + structs | [chronos-esp32 `ChronosESP32.h` / `.cpp`](https://github.com/fbiego/chronos-esp32) |
| Transport abstraction | `android/.../transport/WatchTransport.kt`, `BleTransport.kt`, `WifiTransport.kt` |
| WiFi TCP spec | This document, section 8 |
| WiFi firmware impl | `hal/esp32/wifi_transport.h`, `hal/esp32/wifi_transport.cpp` |
| Weather protocol | This document, section 9 |
| Loop integration, LVGL, flags | `hal/esp32/app_hal.cpp` |
| Navigation UI | `src/apps/navigation/navigation.c`, `navigation.h` |
| Board pins / display init | `hal/esp32/displays/*.hpp`, `pins.h` |
| Build matrix | `platformio.ini` |
| LVGL config | `include/lv_conf.h` |

---

## 14. Document maintenance

- **Library version:** This keynote aligns with **ChronosESP32 1.9.0** lines in `platformio.ini`. When upgrading the dependency, re-diff `ChronosESP32.cpp` for opcode changes.
- **Firmware fork:** If your team maintains a private fork, link it in your internal wiki and note **drift** from `fbiego/esp32-c3-mini` and `fbiego/chronos-esp32`.
- **Transport updates:** WiFi TCP transport spec (section 8) and weather protocol (section 9) are maintained alongside the Android app source in this repo.

---

*End of technical keynote.*
