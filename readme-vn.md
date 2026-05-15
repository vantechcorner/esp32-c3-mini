# esp32-c3-mini

Firmware ESP32 (ESP32-S3, ESP32-C3, ESP32 Classic) tập trung **điều hướng Google Maps**, đồng bộ **thời gian** và **thời tiết**. App Android đồng hành: [VanTC-Navi](https://github.com/vantechcorner/vantc-navi).

**English:** [README.md](README.md)

## Trải nghiệm đầy đủ (bản gốc + Chronos)

Fork này ưu tiên điều hướng, giờ và thời tiết trên các board đã liệt kê. Để dùng **đủ** tính năng đồng hồ (thông báo, điều khiển nhạc, danh bạ, QR, mặt đồng hồ, …), hãy dùng firmware **upstream** [esp32-c3-mini](https://github.com/fbiego/esp32-c3-mini) với app **[Chronos](https://chronos.ke/app?id=c3-mini)** qua **BLE**.

| Nhu cầu | Khuyến nghị |
|---------|-------------|
| Điều hướng + giờ + thời tiết (VanTC-Navi, BLE hoặc WiFi) | Repo này |
| Toàn bộ tính năng Chronos | Bản gốc + app Chronos (BLE) |

## Tính năng chính (fork này)

- **Điều hướng** — dữ liệu lộ trình từ Google Maps (qua app trên điện thoại)
- **Thời gian & thời tiết** — đồng bộ từ app đồng hành
- **Kết nối** — BLE (chuẩn) hoặc **WiFi TCP** (tùy chọn, cho thiết bị không có BLE ổn định)

## Phần cứng hỗ trợ (tập trung)

| Thiết bị | Profile BLE | Profile WiFi |
|----------|-------------|--------------|
| Waveshare ESP32-S3-Touch-LCD-1.28 | `lolin_s3_mini_1_28` | `lolin_s3_mini_1_28_wifi` |
| Waveshare ESP32-S3-LCD-1.54 | `waveshare_s3_lcd_1_54` | — (chưa có) |
| Waveshare ESP32-Touch-LCD-3.5 | `esp32_touch_lcd_3_5` | `esp32_touch_lcd_3_5_wifi` |
| LilyGo TTGO T-Display (ESP32 Classic, 4 MB) | `ttgo_tdisplay` | `ttgo_tdisplay_wifi` |

**Cảnh báo nạp firmware:** **Không** nạp `esp32_touch_lcd_3_5*` lên TTGO T-Display (driver màn hình, độ phân giải và bố cục flash khác).

### Waveshare ESP32-Touch-LCD-3.5 — giao diện Navigation

| Môi trường PlatformIO | Giao diện Navigation |
|------------------------|---------------------|
| `esp32_touch_lcd_3_5` | **V2** (thanh trạng thái có đồng hồ, hai cột nội dung, hàng trip) — mặc định |
| `esp32_touch_lcd_3_5_nav_legacy` | **Legacy** (hàng ETA + icon + title + hướng), `-D NAVIGATION_UI_LEGACY=1` |

## Build và nạp firmware

Chọn env đúng board trong [`platformio.ini`](platformio.ini) (đặt `default_envs` hoặc truyền `-e`).

```bash
pio run -e ttgo_tdisplay_wifi -t upload --upload-port COM12
```

Trên Windows, nếu lỗi *cannot access* `firmware.bin`, đóng chương trình khóa `.pio/build/` hoặc loại trừ thư mục project khỏi quét antivirus.

## VanTC-Navi

[VanTC-Navi](https://github.com/vantechcorner/vantc-navi) gửi gói tương thích Chronos cho điều hướng, giờ và thời tiết.

- **BLE** — dùng env **không** có hậu tố `_wifi`; `watch.begin()` chạy bình thường.
- **WiFi** — dùng env `*_wifi`; xem [WiFi transport](#wifi-transport-thay-thế-ble) bên dưới.

## WiFi transport (thay thế BLE)

Một số thiết bị Android (ví dụ một số màn hình ô tô) không có BLE. Fork này thêm **WiFi TCP**: điện thoại làm hotspot + TCP server; ESP32 là station + TCP client.

| Môi trường PlatformIO | Board | Kết nối |
|------------------------|-------|---------|
| `lolin_s3_mini_1_28_wifi` | Waveshare S3 1.28" | WiFi TCP (BLE tắt) |
| `esp32_touch_lcd_3_5_wifi` | Waveshare ESP32-Touch-LCD-3.5 (flash 16 MB) | WiFi TCP (BLE tắt) |
| `ttgo_tdisplay_wifi` | TTGO T-Display ESP32 (4 MB) | WiFi TCP (BLE tắt) |

**Cách hoạt động:**

1. VanTC-Navi mở TCP server cổng **8423** trên hotspot điện thoại.
2. ESP32 kết nối AP và mở socket tới `gateway:8423`.
3. Gói có tiền tố độ dài `[2 byte BE][payload]` — cùng định dạng Chronos như BLE.
4. Payload được inject vào `ChronosESP32`, UI/getter hiện có dùng lại không đổi.

**Cấu hình:** [`hal/esp32/wifi_transport.cpp`](hal/esp32/wifi_transport.cpp) đọc NVS `Preferences`: `wifi_ssid`, `wifi_pass`, `wifi_en`. Ở chế độ WiFi, **BLE bị tắt** (`btStop()`) để giải phóng heap cho WiFi.

Chi tiết giao thức: [`docs/CHRONOS_TECHNICAL_KEYNOTE.md`](docs/CHRONOS_TECHNICAL_KEYNOTE.md) (mục 8 và 12).

## Font tiếng Việt và ghi chú ESP32 LVGL

Montserrat trong LVGL chỉ hỗ trợ Latin cơ bản. Điều hướng và thông báo dùng **font bitmap** trong `src/apps/navigation/` (tạo bằng [lv_font_conv](https://github.com/lvgl/lv_font_conv), `bpp=4`, `--no-compress`):

| File | Dùng cho |
|------|----------|
| [`lv_font_nav_vn_16.c`](src/apps/navigation/lv_font_nav_vn_16.c) | Navigation & nội dung thông báo |
| [`lv_font_nav_vn_20.c`](src/apps/navigation/lv_font_nav_vn_20.c) | Navigation (dòng lớn hơn) |
| [`lv_font_nav_vn_30.c`](src/apps/navigation/lv_font_nav_vn_30.c) | Navigation (khoảng cách / tiêu đề) |

Dải Unicode: `0x20-0x7F`, `0xA0-0xFF`, `0x100-0x24F`, `0x1EA0-0x1EFF`. Runtime **không** nạp TTF từ `support/`; file nguồn `support/fonts/Montserrat-Regular.ttf` bị gitignore — đặt file local để tạo lại:

```bash
npx lv_font_conv --font support/fonts/Montserrat-Regular.ttf -r 0x20-0x7F,0xA0-0xFF,0x100-0x24F,0x1EA0-0x1EFF --size 16 --bpp 4 --format lvgl --no-compress -o src/apps/navigation/lv_font_nav_vn_16.c --lv-font-name lv_font_nav_vn_16 --lv-fallback lv_font_montserrat_16
```

Lặp lại với size 20 và 30, tên file và `--lv-fallback` tương ứng.

**BLE / NimBLE:** Không gọi LVGL từ thread callback NimBLE. [`hal/esp32/app_hal.cpp`](hal/esp32/app_hal.cpp) đặt `pendingNotificationAlert` trong `notificationCallback` và gọi `showAlert()` từ `hal_loop()` sau `lv_timer_handler()`.

**Buffer hiển thị (LVGL 9):** Buffer vẽ phải căn chỉnh địa chỉ; `lvBuffer` dùng `__attribute__((aligned(32)))` trong `app_hal.cpp`.

## Ghi chú bring-up Waveshare ESP32-Touch-LCD-3.5

Trên [`ESP32_TOUCH_LCD_35`](hal/esp32/displays/pins.h), nguồn/reset LCD đi qua **TCA9554** (`0x20`). Driver phải khởi tạo TCA9554 và bật **P0..P2** trước `gfx->begin()` trong [`hal/esp32/displays/esp32_touch_lcd_35.hpp`](hal/esp32/displays/esp32_touch_lcd_35.hpp).

Bỏ qua bước này có thể gây **có đèn nền nhưng màn đen**, trong khi BLE/WiFi vẫn kết nối.

## Tham khảo

- Bản gốc: [fbiego/esp32-c3-mini](https://github.com/fbiego/esp32-c3-mini)
- App Chronos: [chronos.ke](https://chronos.ke/app?id=c3-mini) · [ChronosESP32](https://github.com/fbiego/chronos-esp32)
- VanTC-Navi: [vantechcorner/vantc-navi](https://github.com/vantechcorner/vantc-navi)
