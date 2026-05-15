# esp32-c3-mini
A demo LVGL Watch project for ESP32 C3 mini 240*240 touch display development board. Can also be built natively to test the LVGL UI.

## Screens Preview

![Preview](preview.png?raw=true "preview")

![Preview2](preview_2.png?raw=true "preview2")

## Screens
 - Time (Time, Date, Day, Weather[Icon, Temp]) + Custom Watchfaces
 - ~~Installable custom watchfaces from Chronos app~~ ToDo - LVGL 9
 - Weather (City,Icon, Temp, Update time) (1 week forecast [Day, Icon, Temp]) (Hourly forecast [+wind, ])
 - Notifications (Icon, Time, Text) (List [Icon, Text] - 10 notifications) (Incoming call); on ESP32, labels use Vietnamese-capable fonts (see [Vietnamese fonts](#vietnamese-fonts-and-esp32-lvgl-notes))
 - Settings (Brightness, Timeout, Battery, About)
 - Control (Music Control, Find Phone, Bluetooth State) (Camera Capture)
 - QR Codes, Contacts
 - Games - Simon Says, Racing (Need to enable)
 - Navigation (Google Maps directions on ESP32); route strings use the same Vietnamese font subset (see [Vietnamese fonts](#vietnamese-fonts-and-esp32-lvgl-notes)). On **Waveshare ESP32-Touch-LCD-3.5**, you can pick **Navigation V2** vs **legacy** rectangular UI via PlatformIO environment (see [below](#waveshare-esp32-touch-lcd-35-navigation-ui)).
 - Create custom apps with LVGL [sample](src/apps/sample/)

 ## Building

 Select your build environment in platformio.ini by uncommenting only one `default_envs`

 **Waveshare ESP32-Touch-LCD-3.5 — Navigation UI:** same board, two firmware flavors:

 | PlatformIO environment | Navigation UI |
 |------------------------|----------------|
 | `esp32_touch_lcd_3_5` | **V2** (status bar, two-column body, footer) — default |
 | `esp32_touch_lcd_3_5_nav_legacy` | **Legacy** full-panel layout (ETA row + icon + title + directions), build flag `-D NAVIGATION_UI_LEGACY=1` |

 Example: `pio run -e esp32_touch_lcd_3_5 -t upload` vs `pio run -e esp32_touch_lcd_3_5_nav_legacy -t upload` (add `--upload-port COMx` as needed).

 On Windows, if the build fails with *cannot access* `.pio/build/.../firmware.bin`, another process is locking the file (antivirus, IDE). Close lockers, exclude the project folder from real-time scanning, or flash from `firmware.elf` using the PlatformIO bundled `esptool.py` and `write_flash` at the usual offsets for your partition table.

 When building for native check that you have configured SDL according to your platform. Follow the instructions here
 https://github.com/lvgl/lv_platformio?tab=readme-ov-file#install-sdl-drivers

 The SDL path might be different depending on your configuration and you will need to update [`platformio.ini`](platformio.ini) accordingly

 ### Prebuilt Native 

 The prebuilt native applications have been included in the [`test folder`](test/), however you might still require SDL installed before running them.
 
 You can also find binary files for various boards.

 ### Web Flasher (ESP32)

 You can also flash ESP32 boards using the web tool available at https://chronos.ke/c3-ui#install

## Supported Boards

![Boards 2](boards_2.png?raw=true "boards2")
![Boards](boards.png?raw=true "boards")

- [Viewe SmartRing AMOLED 1.8 466x466](https://viewedisplay.com/product/esp32-1-8-inch-round-amoled-touch-display-arduino-lvgl-wifi-voice-assistant-ai-smart-displays/)
- [Viewe Touch Knob AMOLED 1.5 466x466](https://viewedisplay.com/product/esp32-1-5-inch-466x466-round-amoled-knob-display-touch-screen-arduino-lvgl/)
- [CrowPanel ESP32 Display-1.28(R) 240x240](https://www.elecrow.com/crowpanel-esp32-display-1-28-r-inch-240-240-round-ips-display-capacitive-touch-spi-screen.html)
- [M5 Stack Dial 240x240](https://docs.m5stack.com/en/core/M5Dial)
- [ESP32 C3 Mini 1.28 240x240](https://www.aliexpress.com/item/1005006451631422.html)
- [TTGO T-Display 1.14 240x135](https://github.com/Xinyuan-LilyGO/TTGO-T-Display)
- [Waveshare S3 1.28 240x240](https://www.waveshare.com/product/esp32-s3-touch-lcd-1.28.htm)
- [Waveshare S3 1.69 240x280](https://www.waveshare.com/esp32-s3-touch-lcd-1.69.htm)
- [Waveshare ESP32-S3-LCD-1.54 240x240](https://www.waveshare.com/esp32-s3-lcd-1.54.htm)
- [Waveshare ESP32-Touch-LCD-3.5 320x480](https://www.waveshare.com/esp32-touch-lcd-3.5.htm)
- [Waveshare RP2040 1.28 240x240](https://www.waveshare.com/rp2040-touch-lcd-1.28.htm)
- [Waveshare RP2040 1.69 240x280](https://www.waveshare.com/product/rp2040-touch-lcd-1.69.htm)
- [Waveshare S3 1.75 466x466](https://www.waveshare.com/product/esp32-s3-touch-amoled-1.75.htm)
- [Waveshare S3 2.06 410x502](https://www.waveshare.com/product/esp32-s3-touch-amoled-2.06.htm)

 ## Watchfaces

This project supports two types of watchfaces in addition to the default one:

#### 1. External Precompiled Binary Watchfaces

These watchfaces are binary files converted into LVGL code and compiled along with the main code. To add or remove these watchfaces, you need to recompile and flash the firmware.

- Check out the [`esp32-lvgl-watchface`](https://github.com/fbiego/esp32-lvgl-watchface) project for details on converting watchfaces from binary to LVGL code.
- You can add more watchfaces, but be mindful of the ESP32's flash size limitations. Prioritize compiling only your favorite watchfaces.
- Links to pre-built binary watchfaces are included. Enable them in `app_hal.h` according to your build platform.

#### 2. External Installable Binary Watchfaces

- [x] Custom watchfaces works on LVGL 8. [Checkout this branch](https://github.com/fbiego/esp32-c3-mini/tree/lvgl_8)
- [ ] Work in progress for LVGL 9

- Ensure there is sufficient storage space on the ESP32 flash. Using the FFAT partition is recommended.

> [!IMPORTANT]
> This feature is experimental and may not work 100% reliably.
> Ensure your partition is mounted successfully for proper functionality.

> [!WARNING]  
> This has issues running on ESP32 C3 Mini due to smaller SRAM size
> Not ported to LVGL 9 yet. Work in progess

## Chronos App
This is needed for additional functions on esp32 hardware as listed below.

[<img src="chronos.png?raw=true" width=100 align=left>](https://chronos.ke/app?id=c3-mini)
<br><br><br><br>

[ChronosESP32 Website](https://chronos.ke/esp32)


### App functions (ESP32)
[ChronosESP32](https://github.com/fbiego/chronos-esp32) library handles communication with the Chronos app over BLE
- Sync time
- ~~Install additional watchfaces~~ LVGL 9 Work in progress
- Send notifications and call alerts
- Sync weather info
- Sync QR Links, & Contacts
- Music control, find phone & Camera
- Send Navigation instructions

### WiFi transport (alternative to BLE)

Some Android head units (e.g. OLEDPRO X4S Eco) only support Bluetooth Serial and lack BLE, so the stock Chronos app cannot run. This fork adds a **WiFi TCP transport** as an alternative: the Android device acts as a WiFi hotspot + TCP server, and the ESP32 connects as a station + TCP client.

| PlatformIO environment | Board | Transport |
|------------------------|-------|-----------|
| `lolin_s3_mini_1_28_wifi` | Waveshare S3 1.28" | WiFi TCP (BLE disabled) |
| `esp32_touch_lcd_3_5_wifi` | Waveshare ESP32-Touch-LCD-3.5 | WiFi TCP (BLE disabled) |

**How it works:**
1. The Android companion app ([vantc-navi](https://github.com/vantechcorner/vantc-navi)) starts a TCP server on port `8423` over its WiFi hotspot.
2. The ESP32 connects to the AP and opens a TCP socket to `gateway:8423`.
3. Chronos protocol packets (time, weather, navigation) are sent length-prefixed over TCP — identical binary format to BLE.
4. Packets are injected into the `ChronosESP32` parser, so all UI code works unchanged.

**Configuration:** WiFi credentials are hardcoded in [`hal/esp32/wifi_transport.cpp`](hal/esp32/wifi_transport.cpp) (defaults via `Preferences` NVS keys `wifi_ssid`, `wifi_pass`, `wifi_en`). When WiFi mode is active, BLE is disabled (`btStop()`) to free heap memory.

See [`docs/CHRONOS_TECHNICAL_KEYNOTE.md`](docs/CHRONOS_TECHNICAL_KEYNOTE.md) section 8 and 12 for protocol details.

## Vietnamese fonts and ESP32 LVGL notes

Built-in Montserrat fonts in LVGL only cover basic Latin. The ESP32 build also links **bitmap fonts** generated with [lv_font_conv](https://github.com/lvgl/lv_font_conv) from Montserrat Regular (plain bitmap, `bpp=4`, `--no-compress` so `LV_USE_FONT_COMPRESSED` can stay off in `lv_conf.h`):

| File | Use |
|------|-----|
| [`src/apps/navigation/lv_font_nav_vn_16.c`](src/apps/navigation/lv_font_nav_vn_16.c) | Navigation & notification list/detail body |
| [`src/apps/navigation/lv_font_nav_vn_20.c`](src/apps/navigation/lv_font_nav_vn_20.c) | Navigation (larger lines) |
| [`src/apps/navigation/lv_font_nav_vn_30.c`](src/apps/navigation/lv_font_nav_vn_30.c) | Navigation (distance / title line) |

Unicode ranges used: `0x20-0x7F`, `0xA0-0xFF`, `0x100-0x24F`, `0x1EA0-0x1EFF`, with fallback to the stock Montserrat size. Source TTF is ignored by git under `support/fonts/Montserrat*.ttf` (see [`.gitignore`](.gitignore)).

**Regenerate** (after placing `support/fonts/Montserrat-Regular.ttf`):

```bash
npx lv_font_conv --font support/fonts/Montserrat-Regular.ttf -r 0x20-0x7F,0xA0-0xFF,0x100-0x24F,0x1EA0-0x1EFF --size 16 --bpp 4 --format lvgl --no-compress -o src/apps/navigation/lv_font_nav_vn_16.c --lv-font-name lv_font_nav_vn_16 --lv-fallback lv_font_montserrat_16
```

Repeat for `--size 20` / `30` and matching output names and `--lv-fallback`.

**Notifications:** `ui_messageTime`, `ui_messageContent`, list rows, and `ui_alertText` in [`src/ui/ui.c`](src/ui/ui.c) use `lv_font_nav_vn_16`.

**BLE / NimBLE:** Incoming notifications must not call LVGL from the NimBLE host task (stack and thread-safety). [`hal/esp32/app_hal.cpp`](hal/esp32/app_hal.cpp) sets `pendingNotificationAlert` in `notificationCallback` and runs `showAlert()` from `hal_loop()` after `lv_timer_handler()`.

**Display buffers (LVGL 9):** `lv_display_set_buffers` requires draw buffers to be address-aligned. [`hal/esp32/app_hal.cpp`](hal/esp32/app_hal.cpp) declares `lvBuffer` (and the optional rotation scratch buffer) with `__attribute__((aligned(32)))`.

### Waveshare ESP32-Touch-LCD-3.5 bring-up note

For [`ESP32_TOUCH_LCD_35`](hal/esp32/displays/pins.h), the LCD power/reset lines are gated by a `TCA9554` IO expander. The display driver initializes `TCA9554` (`0x20`) and enables `P0..P2` **before** `gfx->begin()` in [`hal/esp32/displays/esp32_touch_lcd_35.hpp`](hal/esp32/displays/esp32_touch_lcd_35.hpp).

If this step is skipped, the board may show **backlight on but no image** (black screen), even though BLE still works.

### Waveshare ESP32-Touch-LCD-3.5 Navigation UI

Navigation layout for this board is selected at **compile time** (see [Building](#building)):

- **`esp32_touch_lcd_3_5`** — **Navigation V2**: landscape layout with a status row (clock, notification/call badges, battery, Bluetooth), main content column plus map/icon column, and a footer line for remaining distance.
- **`esp32_touch_lcd_3_5_nav_legacy`** — **Legacy** Chronos-style rectangular screen: single column with ETA text, turn icon, route title, and directions (similar spirit to the smaller rectangular Navigation layouts on other boards).

Implementation: [`src/apps/navigation/navigation.c`](src/apps/navigation/navigation.c); macro `NAVIGATION_UI_LEGACY` gates the legacy branch. Status-bar refresh in [`hal/esp32/app_hal.cpp`](hal/esp32/app_hal.cpp) applies only to **V2**, not legacy.

