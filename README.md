# esp32-c3-mini

ESP32 firmware (ESP32-S3, ESP32-C3, ESP32 Classic) focused on **Google Maps navigation**, with **time** and **weather** sync **via BLE and WiFi**. Companion Android app: [VanTC-Navi](https://github.com/vantechcorner/vantc-navi).

**Tiếng Việt:** [readme-vn.md](readme-vn.md)

## Full feature set (upstream + Chronos)

This fork emphasizes navigation, time, and weather on selected boards. For the **complete** watch experience (notifications, music control, contacts, QR codes, watchfaces, and more), use the **upstream** [esp32-c3-mini](https://github.com/fbiego/esp32-c3-mini) firmware with the official **[Chronos](https://chronos.ke/app?id=c3-mini)** app over **BLE**.

| Use case | Recommendation |
|----------|----------------|
| Navigation + time + weather (VanTC-Navi, BLE or WiFi) | This repo |
| All Chronos features | Upstream repo + Chronos app (BLE) |

## Core features (this fork)

- **Navigation** — turn-by-turn data from Google Maps (via phone app)
- **Time & weather** — synced from the companion app via **BLE** or **WiFi**
- **Transport** — BLE (standard) or **WiFi TCP** (optional, for devices without reliable BLE)

## Supported hardware (focus)

| Device | BLE profile | WiFi profile |
|--------|-------------|--------------|
| Waveshare ESP32-S3-Touch-LCD-1.28 | `lolin_s3_mini_1_28` | `lolin_s3_mini_1_28_wifi` |
| Waveshare ESP32-S3-LCD-1.54 | `waveshare_s3_lcd_1_54` | — (not yet) |
| Waveshare ESP32-Touch-LCD-3.5 | `esp32_touch_lcd_3_5` | `esp32_touch_lcd_3_5_wifi` |
| LilyGo TTGO T-Display (ESP32 Classic, 4 MB) | `ttgo_tdisplay` | `ttgo_tdisplay_wifi` |

**Flash warning:** Do **not** flash `esp32_touch_lcd_3_5*` on a TTGO T-Display (different display driver, resolution, and flash layout).

## Pre-built firmware (test builds)

Test firmware binaries for the boards above are published on the **[GitHub Releases](https://github.com/vantechcorner/esp32-c3-mini/releases)** page. Download the `.bin` that matches your board and flash it with a web flasher (for example [ESPWebTool](https://esptool.spacehuhn.com/)) or `esptool`. Use the flash offset in the filename (e.g. `_0x0.bin` → address `0x0`, `_0x1000.bin` → `0x1000`).

To build from source instead, see [Building with PlatformIO](#building-with-platformio) below.

### Waveshare ESP32-Touch-LCD-3.5 — Navigation UI

On the **3.5"** board (320×480), navigation layout is chosen at **compile time** in [`src/apps/navigation/navigation.c`](src/apps/navigation/navigation.c):

| PlatformIO environment | Navigation UI |
|------------------------|---------------|
| `esp32_touch_lcd_3_5` | **Navigation V2** (default) — landscape: status bar (**clock**), icon column + instruction column, trip row (duration / distance / ETA time), progress bar placeholder |
| `esp32_touch_lcd_3_5_wifi` | Same **V2** UI as above, with WiFi transport |
| `esp32_touch_lcd_3_5_nav_legacy` | **Legacy** — Chronos-style full panel: ETA row + turn icon + title + directions (`-D NAVIGATION_UI_LEGACY=1`) |

WiFi build for this board requires **16 MB** flash (`esp32_touch_lcd_3_5_wifi`). Status-bar clock refresh runs only on **V2**, not legacy.

## Building with PlatformIO

### Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or [VS Code extension](https://platformio.org/install/ide?install=vscode))
- USB cable and the correct serial driver for your board (CP210x, CH9102, etc.)
- Clone this repo and open the project folder

### 1. Choose an environment

Each board has a PlatformIO **environment** (`env`) in [`platformio.ini`](platformio.ini). Either:

- Uncomment one line under `default_envs = ...` at the top of `platformio.ini`, **or**
- Pass `-e <env_name>` on every command (recommended when switching boards)

Examples:

| Board | BLE | WiFi |
|-------|-----|------|
| Touch LCD 3.5" | `esp32_touch_lcd_3_5` | `esp32_touch_lcd_3_5_wifi` |
| S3 Touch 1.28" | `lolin_s3_mini_1_28` | `lolin_s3_mini_1_28_wifi` |
| TTGO T-Display | `ttgo_tdisplay` | `ttgo_tdisplay_wifi` |

### 2. Build only

```bash
pio run -e esp32_touch_lcd_3_5
```

First build downloads toolchains and libraries; it can take several minutes.

### 3. Upload (flash)

Connect the board, find the COM port (`pio device list` on CLI), then:

```bash
pio run -e esp32_touch_lcd_3_5_wifi -t upload --upload-port COM21
```

Build + upload in one step (omit `-t upload` if you only built already):

```bash
pio run -e ttgo_tdisplay_wifi -t upload --upload-port COM12
```

### 4. Serial monitor (optional)

```bash
pio device monitor -p COM12 -b 115200
```

### Troubleshooting

- **Wrong firmware on wrong hardware** — e.g. `esp32_touch_lcd_3_5*` on TTGO → black screen or boot loop; use the env from the [hardware table](#supported-hardware-focus).
- **Windows: cannot access `firmware.bin`** — antivirus or another process locks `.pio/build/`; close lockers or exclude the project folder.
- **Touch LCD 3.5: backlight on, no image** — see [bring-up note](#waveshare-esp32-touch-lcd-35-bring-up-note) (TCA9554 IO expander).

## VanTC-Navi

[VanTC-Navi](https://github.com/vantechcorner/vantc-navi) sends Chronos-compatible packets for navigation, time, and weather.

- **BLE** — use a non-`_wifi` env; `watch.begin()` runs as usual.
- **WiFi** — use a `*_wifi` env; see [WiFi transport](#wifi-transport-alternative-to-ble) below.

## WiFi transport (alternative to BLE)

Some Android devices (e.g. certain head units) lack BLE. This fork adds **WiFi TCP transport**: the phone is a WiFi hotspot + TCP server; the ESP32 is a station + TCP client.

| PlatformIO environment | Board | Transport |
|------------------------|-------|-----------|
| `lolin_s3_mini_1_28_wifi` | Waveshare S3 1.28" | WiFi TCP (BLE disabled) |
| `esp32_touch_lcd_3_5_wifi` | Waveshare ESP32-Touch-LCD-3.5 (16 MB flash) | WiFi TCP (BLE disabled) |
| `ttgo_tdisplay_wifi` | TTGO T-Display ESP32 (4 MB) | WiFi TCP (BLE disabled) |

**How it works:**

1. VanTC-Navi starts a TCP server on port **8423** on the phone hotspot.
2. The ESP32 connects to the AP and opens `gateway:8423`.
3. Packets are length-prefixed `[2-byte BE length][payload]` — same Chronos binary format as BLE.
4. Payload is injected into `ChronosESP32`, so existing UI/getters work unchanged.

**Configuration:** [`hal/esp32/wifi_transport.cpp`](hal/esp32/wifi_transport.cpp) reads NVS `Preferences` keys `wifi_ssid`, `wifi_pass`, `wifi_en` (defaults in code). In WiFi mode, **BLE is disabled** (`btStop()`) to free heap for the WiFi stack.

Protocol details: [`docs/CHRONOS_TECHNICAL_KEYNOTE.md`](docs/CHRONOS_TECHNICAL_KEYNOTE.md) (sections 8 and 12).

## Vietnamese fonts and ESP32 LVGL notes

LVGL Montserrat covers basic Latin only. Navigation and notifications use **bitmap fonts** in `src/apps/navigation/` (generated with [lv_font_conv](https://github.com/lvgl/lv_font_conv), `bpp=4`, `--no-compress`):

| File | Use |
|------|-----|
| [`lv_font_nav_vn_16.c`](src/apps/navigation/lv_font_nav_vn_16.c) | Navigation & notification body |
| [`lv_font_nav_vn_20.c`](src/apps/navigation/lv_font_nav_vn_20.c) | Navigation (larger lines) |
| [`lv_font_nav_vn_30.c`](src/apps/navigation/lv_font_nav_vn_30.c) | Navigation (distance / title) |

Unicode ranges: `0x20-0x7F`, `0xA0-0xFF`, `0x100-0x24F`, `0x1EA0-0x1EFF`. Runtime does **not** load TTF from `support/`; optional source `support/fonts/Montserrat-Regular.ttf` is gitignored — place it locally to regenerate:

```bash
npx lv_font_conv --font support/fonts/Montserrat-Regular.ttf -r 0x20-0x7F,0xA0-0xFF,0x100-0x24F,0x1EA0-0x1EFF --size 16 --bpp 4 --format lvgl --no-compress -o src/apps/navigation/lv_font_nav_vn_16.c --lv-font-name lv_font_nav_vn_16 --lv-fallback lv_font_montserrat_16
```

Repeat for sizes 20 and 30 with matching output names and `--lv-fallback`.

**BLE / NimBLE:** Do not call LVGL from the NimBLE callback thread. [`hal/esp32/app_hal.cpp`](hal/esp32/app_hal.cpp) sets `pendingNotificationAlert` in `notificationCallback` and runs `showAlert()` from `hal_loop()` after `lv_timer_handler()`.

**Display buffers (LVGL 9):** Draw buffers must be aligned; `lvBuffer` uses `__attribute__((aligned(32)))` in `app_hal.cpp`.

## Waveshare ESP32-Touch-LCD-3.5 bring-up note

On [`ESP32_TOUCH_LCD_35`](hal/esp32/displays/pins.h), LCD power/reset go through a **TCA9554** IO expander (`0x20`). The driver must init TCA9554 and enable **P0..P2** before `gfx->begin()` in [`hal/esp32/displays/esp32_touch_lcd_35.hpp`](hal/esp32/displays/esp32_touch_lcd_35.hpp).

If this is skipped, you may get **backlight on but a black screen** while BLE/WiFi still connects.

## References

- Upstream: [fbiego/esp32-c3-mini](https://github.com/fbiego/esp32-c3-mini)
- Chronos app: [chronos.ke](https://chronos.ke/app?id=c3-mini) · [ChronosESP32](https://github.com/fbiego/chronos-esp32)
- VanTC-Navi: [vantechcorner/vantc-navi](https://github.com/vantechcorner/vantc-navi)
