# Cursor handoff — esp32-c3-mini

Tài liệu nội bộ cho agent/dev tiếp theo. Docs công khai: [`README.md`](README.md) (EN), [`readme-vn.md`](readme-vn.md) (VI).

## Mục tiêu fork (2026)

- **Điều hướng Google Maps** trên ESP32 (S3 / C3 / Classic), đồng bộ **thời gian + thời tiết** qua **BLE hoặc WiFi**.
- App Android: **[VanTC-Navi](https://github.com/vantechcorner/vantc-navi)** (gói Chronos-compatible).
- **Đủ tính năng đồng hồ** (notifications, music, watchfaces, …): khuyến nghị user dùng **upstream** [fbiego/esp32-c3-mini](https://github.com/fbiego/esp32-c3-mini) + app **[Chronos](https://chronos.ke/app?id=c3-mini)** qua BLE.

## Stack

- **LVGL** 9.x (`lvgl/lvgl@9.3.0`), `include/lv_conf.h`
- **PlatformIO** + **ChronosESP32** (`watch` trong `hal/esp32/app_hal.cpp`)
- **HAL:** `hal/esp32/app_hal.cpp`, display per-board trong `hal/esp32/displays/`

## Phần cứng đang tập trung — bảng env

| Thiết bị | BLE | WiFi (`ENABLE_WIFI_TRANSPORT`) | Ghi chú |
|----------|-----|--------------------------------|---------|
| Waveshare **ESP32-S3-Touch-LCD-1.28** | `lolin_s3_mini_1_28` | `lolin_s3_mini_1_28_wifi` | 16 MB, tròn 240×240 |
| Waveshare **ESP32-S3-LCD-1.54** | `waveshare_s3_lcd_1_54` | — chưa có | |
| Waveshare **ESP32-Touch-LCD-3.5** | `esp32_touch_lcd_3_5` | `esp32_touch_lcd_3_5_wifi` | **16 MB** bắt buộc cho WiFi env; Navigation **V2** |
| LilyGo **TTGO T-Display** (Classic) | `ttgo_tdisplay` | `ttgo_tdisplay_wifi` | **4 MB** flash; ST7789 135×240; **không** dùng env `esp32_touch_lcd_3_5*` |

**Lệnh build/flash mẫu:**

```bash
pio run -e lolin_s3_mini_1_28_wifi -t upload --upload-port COM6
pio run -e esp32_touch_lcd_3_5_wifi -t upload --upload-port COM21
pio run -e ttgo_tdisplay_wifi -t upload --upload-port COM12
```

`platformio.ini`: `esp32_touch_lcd_3_5_wifi` có `upload_port = COM21` (đổi nếu cần). Rule Cursor: `.cursor/rules/touch-lcd-firmware-flash.mdc` — sau sửa firmware Touch LCD WiFi nên `pio run -t upload`.

## WiFi TCP transport

- **Mục đích:** Thiết bị/Android không có BLE ổn định; phone = hotspot + TCP **8423**, ESP = STA + client `gateway:8423`.
- **Packet:** `[2-byte BE length][Chronos payload]` → inject `ChronosESP32` (`hal/esp32/wifi_transport.cpp`, `chronos_inject` → `_incomingData` + `dataReceived()`).
- **NVS:** `wifi_ssid`, `wifi_pass`, `wifi_en` (defaults trong `wifi_transport.cpp`, ví dụ `CarLinkAP467` / `carlink324`).
- **BLE tắt khi WiFi:** `btStop()`, skip `watch.begin()`; `isPhoneConnected()` = BLE **hoặc** `wifi_transport_connected()`.
- **Init order:** `wifi_transport_early_init()` ngay sau `prefs.begin()` → display init → `btStop()` (nếu WiFi) → `wifi_transport_loop()` trong `hal_loop()`. WiFi **trước** NimBLE để tránh `BLE_INIT: Malloc failed` trên S3.
- **Env WiFi hiện có:** `lolin_s3_mini_1_28_wifi`, `esp32_touch_lcd_3_5_wifi`, `ttgo_tdisplay_wifi`.

## Navigation — ESP32-Touch-LCD-3.5 (V2)

- **Macro:** Không có `NAVIGATION_UI_LEGACY` → **V2** (`esp32_touch_lcd_3_5`, `esp32_touch_lcd_3_5_wifi`). Legacy: `esp32_touch_lcd_3_5_nav_legacy`.
- **Layout V2** (`navigation.c`, `#if ESP32_TOUCH_LCD_35 && !NAVIGATION_UI_LEGACY`):
  - Status bar: **chỉ đồng hồ** (đã bỏ net OK / badge khác).
  - Cột trái: `ui_navV2_map_box` (nền đen `#000000`, viền xám `#5F6368`) + canvas icon 48×48.
  - Cột phải: `ui_navTitle` (xanh), `ui_navDirection` (wrap, tách 2 dòng nếu có *"về hướng"*).
  - Hàng trip 3 cột: duration / distance / ETA (**chỉ giờ** `HH:MM`, bỏ chữ "Dự kiến").
  - Progress bar placeholder (chưa có field từ app).
- **Trip row — nguồn dữ liệu (quan trọng):** Không parse `navText` newline nữa. Sau `navigateInfo(...)`, HAL gọi `navigation_ws35_set_trip_row(eta, duration, distance, title, directions, clock_h, clock_m)` (`app_hal.cpp`). Cột 1 = `duration` (2 dòng nếu có giờ+phút); cột 3 = ETA `HH:MM` qua `nav_ws35_format_eta_time()`. Workaround tạm: nếu `duration` chỉ có phút mà ETA−clock ≥ 60 phút → tính giờ/phút từ ETA (cần giờ ESP sync).
- **Status clock:** `navigation_refresh_status_bar()` ~1 Hz từ `hal_loop` khi Nav là màn active (`millis()`, không chỉ `sec_tick`).
- **HAL Nav:** Không `update_faces()` khi Nav top; `lv_screen_load` instant (không fade) trên Touch 3.5.

## Navigation — các board khác

- **TTGO / 1.54 / mặc định:** layout chữ nhật / cột trái icon; `my_touchpad_read` dùng `isPhoneConnected()` cho remote touch (WiFi mode).
- **Legacy Touch 3.5:** một cột ETA + icon + title + directions.

## Tiếng Việt (font)

- Font bitmap: `lv_font_nav_vn_16/20/30.c` — generate từ Montserrat, `--no-compress`, `bpp=4`.
- Range: `0x20-0x7F,0xA0-0xFF,0x100-0x24F,0x1EA0-0x1EFF`. TTF nguồn: `support/fonts/` (gitignore `*.ttf`).
- `navigation.c` + `ui.c` (notifications). Runtime **không** load TTF từ `support/`.

## BLE / NimBLE

- `notificationCallback`: không gọi LVGL; `pendingNotificationAlert` → `showAlert()` trong `hal_loop()` sau `lv_timer_handler()`.

## LVGL 9

- `lvBuffer` (+ `rotated_buf` nếu có): `__attribute__((aligned(32)))`.

## ESP32-Touch-LCD-3.5 — TCA9554 (màn đen, có backlight)

- LCD enable qua **TCA9554 @ 0x20** trước `gfx->begin()` — `hal/esp32/displays/esp32_touch_lcd_35.hpp` (P0..P2). Thiếu bước này → backlight sáng, màn đen, BLE/WiFi vẫn có thể OK.

## Pitfall đã gặp

| Triệu chứng | Nguyên nhân | Cách xử lý |
|-------------|-------------|------------|
| Boot loop `Detected size(4096k) smaller than ... 16384k` | Nạp `esp32_touch_lcd_3_5_wifi` lên **TTGO 4 MB** | Dùng `ttgo_tdisplay_wifi` |
| Màn đen sau flash | Sai env / sai driver; hoặc TCA9554 chưa enable | Đúng env; xem TCA9554 |
| ETA cột 3 sai / thiếu | Parse `navText` khi `distance` rỗng | Dùng `navigation_ws35_set_trip_row` từ field riêng |
| Cột duration chỉ “XX phút” | VanTC-Navi gửi `duration` thiếu phần giờ | Sửa app — xem mục **VanTC-Navi handoff** bên dưới; firmware có workaround ETA |
| Windows build fail | Lock `.pio/build/firmware.bin` | Defender exclude; đóng IDE lock file |

## Repo / gitignore

- Ignore: `/firmware/`, `/test/`, `.cursor/rules/`, `support/**/*.zip`, `support/**/*.txt`, `support/fonts/*.ttf`.
- **Giữ trong git:** `support/*.py` (PlatformIO `header_gen.py`, `hardware_build_extra.py`, …).

## VanTC-Navi handoff — navigation `duration` field (English)

**Live test date: 17 May 2026** — ESP32-Touch-LCD-3.5, env `esp32_touch_lcd_3_5_wifi`, TCP to VanTC-Navi on Android (WiFi port 8423). Serial log on COM21.

Use this section when fixing **[VanTC-Navi](https://github.com/vantechcorner/vantc-navi)**. Full protocol context: [`docs/CHRONOS_TECHNICAL_KEYNOTE.md`](docs/CHRONOS_TECHNICAL_KEYNOTE.md) §5.6.

### Transport

- **WiFi:** `[2-byte BE length][Chronos payload]` — same bytes as BLE NUS; ESP injects into `ChronosESP32::dataReceived()`.
- **BLE:** identical navigation payload.

### Live navigation packet (`0xAB` / `0xEF` / `0xFE`, status `data[5] == 0x80`)

Null-terminated UTF-8 strings, **fixed order** (ChronosESP32 1.9.0):

```text
title \0
duration \0
distance \0
eta \0
directions \0
speed \0
```

Each `0x80` packet **replaces all six strings** (no per-field merge on the device).

### How ESP32-Touch-LCD-3.5 Navigation V2 uses each field

| Chronos field | Trip UI / panel |
|---------------|-----------------|
| `title` | Large green line — distance to next maneuver (e.g. `0 m`, `84 km`) |
| **`duration`** | **Trip column 1 — remaining travel time** (user-reported bug here) |
| `distance` | Trip column 2 — remaining distance (e.g. `84 km`) |
| `eta` | Trip column 3 — arrival time only (`HH:MM` extracted; prefix “Dự kiến” stripped) |
| `directions` | Main instruction block (right column) |
| `speed` | Not shown on V2 Touch 3.5 |
| `icon` (48×48 1 bpp) | Left column; 3×96-byte chunks via `0xEE` / `0xFE` |

### Observed payload from VanTC-Navi (17 May 2026)

**Google Maps UI:** `1 hour 25 minutes` (remaining time).  
**ESP32 trip column 1:** `25 minutes` only.

Serial (throttled ~every 2 s while `nav.active`):

```text
[INFO]: NAV dur='25 phút' eta='Dự kiến 23:56' dist='84 km' clk=22:31
[INFO]: NAV dur='26 phút' eta='Dự kiến 23:56' dist='84 km' clk=22:31
```

| Field received | Example | Contains full “1 hour …”? |
|----------------|---------|----------------------------|
| `duration` | `25 phút`, `26 phút` | **No** — minutes portion only |
| `eta` | `Dự kiến 23:56` | Arrival time only, not remaining duration |
| `distance` | `84 km` | No |
| `title` | `0 m`, `84 km` | Maneuver/trip distance, not time |

**Conclusion:** Not an LVGL clipping issue. **VanTC-Navi must send the full remaining-time string in `duration`.** No other field contained `1 hour 25 minutes` during the test.

### Expected fix on VanTC-Navi

| Field | Should contain |
|-------|----------------|
| **`duration`** | Full remaining time as shown on Maps, e.g. `1 giờ 25 phút`, `1 hr 25 min`, or total minutes `85 phút` (≥60 → firmware splits hours/minutes) |
| **`eta`** | Arrival time only, e.g. `Dự kiến 23:56` or `23:56` |

**Do not** send only `25 phút` when Maps shows `1 giờ 25 phút`. Avoid parsing that drops the hour segment (e.g. regex capturing only `(\d+)\s*phút`).

When building the `0x80` packet, assign the Maps notification **remaining travel time** string to **`duration`** (second string in the list above).

### Temporary firmware workaround (esp32-c3-mini)

If `duration` has no hour marker and parsed ETA minus synced clock is **≥ 60 minutes**, firmware displays hours + minutes from that delta. Requires correct time sync via Chronos time packets. **Proper fix remains on the Android app.**

### Post-fix verification checklist

1. Maps `1 hour XX min` → device log `NAV dur='1 giờ XX phút'` (or equivalent).
2. Maps `< 1 hour` → `duration='45 phút'` (single line OK).
3. Each live update sends all six strings; avoid empty `duration` on partial updates.

---

## Changelog handoff

### 2026-05-17 — VanTC-Navi `duration` field documented (live test)

- **Test:** 17 May 2026, Touch LCD 3.5 WiFi, Maps `1 h 25 min` vs ESP `25 phút`; serial `NAV dur=...` logging in `app_hal.cpp`.
- **Docs:** VanTC-Navi handoff (EN) in this file + §5.6 in `docs/CHRONOS_TECHNICAL_KEYNOTE.md`.
- **Firmware:** `navigation_ws35_set_trip_row` + duration from ETA when `duration` is minutes-only and trip ≥ 60 min; trip column wrap / 2-line display.

### 2026-05-15 — README song ngữ, TTGO WiFi, Navigation V2 fixes

- **README.md** (EN) + **readme-vn.md** (VI): tập trung navigation/time/weather; bảng env; PlatformIO build; khuyến nghị upstream+Chronos; giữ mục WiFi / font VN / TCA9554.
- **Env `ttgo_tdisplay_wifi`:** 4 MB, `TTGO_TDISPLAY`, `ENABLE_WIFI_TRANSPORT`.
- **Navigation V2 Touch 3.5:** `navigation_ws35_set_trip_row`; `nav_ws35_format_eta_time` (HH:MM); status bar chỉ clock; `ui_navV2_map_box` nền đen + viền; bỏ nhãn "ETA" cột 3.
- **Remote touch:** `isPhoneConnected()` thay `watch.isConnected()` trong `my_touchpad_read`.
- **Flash:** TTGO phải `ttgo_tdisplay_wifi`, không `esp32_touch_lcd_3_5_wifi`.

### 2026-05-13 — WiFi TCP (S3 1.28 + Touch 3.5)

- `wifi_transport.cpp` / env `lolin_s3_mini_1_28_wifi`, `esp32_touch_lcd_3_5_wifi`.
- Init order WiFi trước BLE; `btStop()` khi WiFi mode.

### 2026-05-10 — Navigation V2 vs legacy (Touch 3.5)

- `NAVIGATION_UI_LEGACY` → `esp32_touch_lcd_3_5_nav_legacy`.
- V2: status, 2 cột, footer/trip; không `update_faces` dưới Nav.

## Việc có thể làm tiếp

- Env **`waveshare_s3_lcd_1_54_wifi`** nếu cần WiFi trên 1.54".
- WiFi: UI cài SSID trên watch; icon WiFi trên status (V2 đã bỏ badge — cân nhắc lại nếu cần).
- Navigation progress bar khi app gửi field progress.
- `Montserrat-Bold` cho title Nav nếu cần đậm hơn.

## Lệnh generate font (nhắc nhanh)

```bash
npx lv_font_conv --font support/fonts/Montserrat-Regular.ttf -r 0x20-0x7F,0xA0-0xFF,0x100-0x24F,0x1EA0-0x1EFF --size 16 --bpp 4 --format lvgl --no-compress -o src/apps/navigation/lv_font_nav_vn_16.c --lv-font-name lv_font_nav_vn_16 --lv-fallback lv_font_montserrat_16
```

Lặp `--size 20` / `30` với tên file và `--lv-fallback` tương ứng.
