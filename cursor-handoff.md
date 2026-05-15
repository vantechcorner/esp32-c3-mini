# Cursor handoff — esp32-c3-mini


## Stack

- **LVGL** 9.x (`lib_deps = lvgl/lvgl@9.3.0`), `include/lv_conf.h`
- **ESP32** qua PlatformIO; ví dụ env: `lolin_s3_mini_1_28` (Waveshare S3 1.28" tròn)

## Tiếng Việt (glyph Montserrat mặc định thiếu)

- Font bitmap custom (Montserrat Regular, **plain** / `--no-compress`, `bpp=4`, LVGL 9):
  - `src/apps/navigation/lv_font_nav_vn_16.c`
  - `src/apps/navigation/lv_font_nav_vn_20.c`
  - `src/apps/navigation/lv_font_nav_vn_30.c`
- Unicode ranges khi generate: `0x20-0x7F,0xA0-0xFF,0x100-0x24F,0x1EA0-0x1EFF` + `--lv-fallback lv_font_montserrat_*` cùng size.
- **Không** dùng font nén nếu `LV_USE_FONT_COMPRESSED == 0` — sẽ crash/assert khi vẽ.
- TTF nguồn: `support/fonts/Montserrat-Regular.ttf` (pattern `support/fonts/Montserrat*.ttf` trong `.gitignore`).

### Gán font trong code

- `src/apps/navigation/navigation.c`: `LV_FONT_DECLARE`, label dùng `lv_font_nav_vn_16/20/30`; đã chỉnh `y` icon (≈76) và cỡ chữ (ETA nhỏ hơn, dòng hướng lớn hơn), `LV_LABEL_LONG_WRAP` cho hướng đi.
- `src/ui/ui.c`: `LV_FONT_DECLARE(lv_font_nav_vn_16)` + style cho notification list/detail + `ui_alertText`.

## Thông báo / BLE — stack NimBLE

- `notificationCallback` (Chronos / NimBLE **host task**) **không** được gọi `showAlert()` / LVGL.
- `hal/esp32/app_hal.cpp`: cờ `pendingNotificationAlert`, set trong callback; `showAlert()` gọi trong `hal_loop()` **sau** `lv_timer_handler()`.
- Log dài trong callback: tránh `Timber.d(full_message)` trên stack nhỏ; có thể log `message.length()` thôi.

## LVGL 9 — buffer màn hình phải căn chỉnh

- `hal/esp32/app_hal.cpp`: `lvBuffer` và `rotated_buf` (SW_ROTATION) dùng `__attribute__((aligned(32)))` + `lvBuffer` là `static`.
- Assert: `buf1 == lv_draw_buf_align(buf1, cf)` nếu địa chỉ mảng global lệch sau các object khác.

## ESP32-Touch-LCD-3.5 — LCD enable qua TCA9554 (quan trọng)

- Với `ESP32_TOUCH_LCD_35`, panel ST7796 có thể bị trạng thái **backlight sáng nhưng màn đen** sau một số lần flash/reset nếu không bật IO expander trước init LCD.
- `hal/esp32/displays/esp32_touch_lcd_35.hpp` đã thêm bước:
  - `Wire.begin(I2C_SDA, I2C_SCL);`
  - cấu hình `TCA9554 @ 0x20`: register `0x03 = 0xF8` (P0..P2 output),
  - kéo mức thấp ngắn (`0x01 = 0x00`) rồi bật P0..P2 (`0x01 = 0x07`),
  - sau đó mới gọi `gfx->begin()`.
- Ghi chú: cấu hình này bám theo hướng init của demo hãng Waveshare cho dòng `ESP32-Touch-LCD-3.5`.
- Nếu gặp lại hiện tượng màn đen nhưng vẫn BLE pair được, kiểm tra đầu tiên là sequence enable TCA9554 này.

## README công khai

- Mục **Vietnamese fonts and ESP32 LVGL notes**, bullet Screens (Navigation Touch 3.5), **Building** (bảng env V2 vs legacy), **Waveshare ESP32-Touch-LCD-3.5 Navigation UI**, và ghi chú Windows khóa file `.pio` — xem `README.md`.

## Changelog (cập nhật handoff)

### 2026-05-13 — WiFi TCP transport cho ESP32-S3-1.28 và ESP32-Touch-LCD-3.5

- **Mục đích:** Thêm kênh truyền dữ liệu qua WiFi (time, navigation, weather) để dùng với các đầu Android (OLEDPRO X4S Eco) chỉ hỗ trợ Bluetooth serial mà không có BLE → app Chronos gốc không chạy được.
- **Kiến trúc:** Android = WiFi AP + TCP server port **8423**. ESP32 = WiFi STA + TCP client → gateway:8423. Packet TCP: `[2-byte BE length][Chronos payload]`, payload giống hệt BLE.
- **Injection vào ChronosESP32:** File `hal/esp32/wifi_transport.cpp` đọc TCP packet, inject trực tiếp vào `_incomingData` + gọi `dataReceived()` → tất cả callback/getter hiện có (navigation, weather, time) hoạt động không cần sửa.
- **Cấu hình WiFi:** Preferences NVS — `wifi_ssid`, `wifi_pass`, `wifi_en` (bool). Đặt trước bằng serial/code, ESP32 tự kết nối AP khi khởi động nếu `wifi_en == true`.
- **BLE tắt khi WiFi:** `btStop()` gọi trước display init khi `ENABLE_WIFI_TRANSPORT` → giải phóng ~40 KB heap cho WiFi driver. `watch.begin()` bị skip, `sendCommand()` là no-op an toàn (`_inited == false`). `isPhoneConnected()` trả `true` nếu BLE hoặc WiFi connected.
- **Init order (quan trọng):** `wifi_transport_early_init()` (WiFi radio) → display init → `btStop()` + skip `watch.begin()` → `wifi_transport_init()` (no-op). WiFi phải init **trước** BLE/NimBLE, nếu không ESP32-S3 crash do hết heap (`BLE_INIT: Malloc failed`).
- **Env PlatformIO:**
  - `lolin_s3_mini_1_28_wifi` — ESP32-S3 1.28" + WiFi (RAM 34.6%, Flash 88.8%)
  - `esp32_touch_lcd_3_5_wifi` — ESP32 Classic 3.5" + WiFi (RAM 35.1%, Flash 31.7%)
- **Files thay đổi:**
  - `hal/esp32/wifi_transport.h` / `.cpp` — WiFi STA + TCP read loop + Chronos injection; `wifi_transport_early_init()` gọi trước BLE
  - `hal/esp32/app_hal.cpp` — `#include wifi_transport.h`, `wifi_transport_early_init()` ngay sau `prefs.begin()`, `btStop()` + skip `watch.begin()`, `wifi_transport_loop()` trong `hal_loop()`, helper `isPhoneConnected()`, guard `watch.getAddress()` cho WiFi mode
  - `platformio.ini` — 2 env mới
- **Android app:** `D:\Github\vantc-navi` — hỗ trợ BLE + WiFi, chọn mode trong UI.

### 2026-05-10 — Navigation Touch 3.5: chọn UI V2 vs legacy qua PlatformIO

- **Mục đích:** Build một firmware **Waveshare ESP32-Touch-LCD-3.5** nhưng chọn giao diện Navigation **V2** (status bar, hai cột, footer) hoặc **legacy** (bố cục chữ nhật kiểu Chronos: ETA + icon + title + hướng, giống tinh thần layout Waveshare 1.54").
- **Cơ chế:** Macro `-D NAVIGATION_UI_LEGACY=1`. Khi **không** định nghĩa macro → **V2** (mặc định env `esp32_touch_lcd_3_5`).
- **Env PlatformIO:**
  - `esp32_touch_lcd_3_5` — Navigation **V2** (mặc định).
  - `esp32_touch_lcd_3_5_nav_legacy` — extends env trên, thêm `NAVIGATION_UI_LEGACY=1`.
- **Code:** `src/apps/navigation/navigation.c` (init + `navigateInfo`); `hal/esp32/app_hal.cpp` chỉ gọi refresh thanh trạng thái ~1 Hz khi **V2** (không legacy).
- **README:** đã thêm mục ngắn trong phần Touch-LCD-3.5 / Building.

### 2026-05-10 — ESP32-Touch-LCD-3.5: watchface “lọ” dưới Navigation + chữ đậm hơn (V2)

- **Triệu chứng:** Cảm giác mặt đồng hồ vẫn hiện phía dưới màn Navigation khi chuyển màn.
- **Xử lý (HAL + UI):** Không gọi `update_faces()` khi màn active là Navigation; dùng `lv_screen_load()` thay cho fade in/out trên board này; nền `ui_navScreen` opaque (`LV_OPA_COVER`).
- **Đồng hồ trên status bar V2:** `navigation_refresh_status_bar` không chỉ dựa vào `sec_tick` — refresh ~1 Hz bằng `millis()` khi Nav là màn hiện tại (`hal_loop`).
- **Typography (V2):** Montserrat 16 trên status; `vn_30` cho title/hướng; `vn_20` footer; tăng chiều cao hàng status/footer tương ứng.

### 2026-05-10 — Windows: PlatformIO không ghi được `firmware.bin` / `.pio/build`

- **Triệu chứng:** `The process cannot access the file` khi build/upload (thường `firmware.bin`, đôi khi `bootloader.bin`).
- **Nguyên nhân:** Tiến trình khác giữ handle (Defender, indexer IDE, v.v.).
- **Workaround:** Đóng app khóa file; loại trừ thư mục project hoặc `.pio` khỏi real-time scan; hoặc đổi ELF → BIN ra `%TEMP%` rồi `esptool write_flash` (bootloader `0x1000`, partitions `0x8000`, otadata `0xe000`, app `0x10000`) bằng Python của PlatformIO: `\.platformio\penv\Scripts\python.exe` + `tool-esptoolpy\esptool.py`, luôn `--chip esp32`.

### 2026-05-09 — Navigation ESP32-Touch-LCD-3.5: hết clip đáy chữ chỉ dẫn

- **Triệu chứng:** Dòng hướng dẫn rẽ (ví dụ *“4th exit”*) bị **mất vài hàng pixel ở mép dưới** — descender / đáy ký tự như bị cắt trong khung label.
- **Nguyên nhân:** Trong `#elif defined(ESP32_TOUCH_LCD_35)` (`src/apps/navigation/navigation.c`), `ui_navDirection` dùng `lv_font_nav_vn_30` và hai dòng wrap, nhưng chiều cao được đặt `H - (ws35_icon_y + 170)`. Với màn landscape **H = 320** còn **~66 px**, không đủ cho hai dòng cỡ 30 → LVGL clip nội dung.
- **Sửa:** Đồng bộ với padding dọc panel (`pad_top` / `pad_bottom` = 14) và vị trí `dir_top = ws35_icon_y + 126`:
  - `lv_obj_set_height(ui_navDirection, H - ws35_pad_tb - ws35_pad_tb - dir_top)` (~**82 px** khi H = 320), lấp đầy vùng nội dung còn lại thay vì hằng số 170.
- **Build / flash:** Env PlatformIO `esp32_touch_lcd_3_5`; upload ví dụ `pio run -e esp32_touch_lcd_3_5 -t upload --upload-port COM21` — đã verify flash OK (hash verified, hard reset).

## Lệnh generate font (nhắc nhanh)

```bash
npx lv_font_conv --font support/fonts/Montserrat-Regular.ttf -r 0x20-0x7F,0xA0-0xFF,0x100-0x24F,0x1EA0-0x1EFF --size 16 --bpp 4 --format lvgl --no-compress -o src/apps/navigation/lv_font_nav_vn_16.c --lv-font-name lv_font_nav_vn_16 --lv-fallback lv_font_montserrat_16
```

Lặp với `--size 20` / `30` và tên file / fallback tương ứng.

## Việc có thể làm tiếp

- Mở rộng subset Unicode nếu Maps/app gửi ký tự ngoài range (ví dụ dấu câu Unicode).
- `LV_FONT_FMT_TXT_LARGE` nếu compiler báo font quá lớn.
- Kiểm tra các màn khác vẫn dùng Montserrat thuần ASCII nếu cần tiếng Việt toàn app.
- Navigation **bold** thật: generate thêm font từ `Montserrat-Bold.ttf` (lv_font_conv) nếu cần đồng bộ visual với V2.
- **WiFi transport:** UI cài đặt SSID/password trên watch (hiện phải đặt qua NVS/code); hiển thị icon WiFi trên status bar khi kết nối WiFi; hỗ trợ mDNS thay hardcode gateway.
