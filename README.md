# esp32-c3-mini
A demo LVGL Watch project for ESP32 C3 mini 240*240 touch display development board. Can also be built natively to test the LVGL UI.

## Videos
- [`C3 UI on 466×466 AMOLED`](https://youtu.be/l3tHc5EIgos)
- [`LVGL 9 update, Navigation`](https://youtu.be/qGODX6ALO_U)
- [`ESP32 C3 mini LVGL`](https://youtu.be/u96OkjxC0Ro)
- [`ESP32  Watchfaces LVGL`](https://youtu.be/lvRsTp9v6_k)
- [`Waveshare ESP32 S3 (240x240 1.28” Round & 240x280 1.69” Rectangular)`](https://youtu.be/WXER_NX7LWI)
- [`Installing custom watchfaces`](https://youtu.be/qXx6tj7s6pQ)



https://github.com/user-attachments/assets/3cdc30e3-8383-4227-b88c-32bd1d464c70



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
 - Navigation (Google Maps directions on ESP32); route strings use the same Vietnamese font subset (see [Vietnamese fonts](#vietnamese-fonts-and-esp32-lvgl-notes))
 - Create custom apps with LVGL [sample](src/apps/sample/)

 ## Building

 Select your build environment in platformio.ini by uncommenting only one `default_envs`

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
- [Waveshare S3 1.28 240x240](https://www.waveshare.com/product/esp32-s3-touch-lcd-1.28.htm)
- [Waveshare S3 1.69 240x280](https://www.waveshare.com/esp32-s3-touch-lcd-1.69.htm)
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

