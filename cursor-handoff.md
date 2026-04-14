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

## README công khai

- Mục **Vietnamese fonts and ESP32 LVGL notes** + cập nhật bullet Screens (xem `README.md` đã commit).

## Lệnh generate font (nhắc nhanh)

```bash
npx lv_font_conv --font support/fonts/Montserrat-Regular.ttf -r 0x20-0x7F,0xA0-0xFF,0x100-0x24F,0x1EA0-0x1EFF --size 16 --bpp 4 --format lvgl --no-compress -o src/apps/navigation/lv_font_nav_vn_16.c --lv-font-name lv_font_nav_vn_16 --lv-fallback lv_font_montserrat_16
```

Lặp với `--size 20` / `30` và tên file / fallback tương ứng.

## Việc có thể làm tiếp

- Mở rộng subset Unicode nếu Maps/app gửi ký tự ngoài range (ví dụ dấu câu Unicode).
- `LV_FONT_FMT_TXT_LARGE` nếu compiler báo font quá lớn.
- Kiểm tra các màn khác vẫn dùng Montserrat thuần ASCII nếu cần tiếng Việt toàn app.
