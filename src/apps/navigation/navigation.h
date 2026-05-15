
/*
    Copyright (c) 2024 Felix Biego. All rights reserved.
    This work is licensed under the terms of the MIT license.  
    For a copy, see <https://opensource.org/licenses/MIT>.
*/

#ifndef _NAVIGATION_APP_H
#define _NAVIGATION_APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#include "app_hal.h"
#include "../../common/app_manager.h"

#ifdef ENABLE_APP_NAVIGATION


LV_IMG_DECLARE( ui_img_arrow_png);   // assets/arrow.png
LV_IMG_DECLARE( ui_img_chronos_logo_png);   // assets/chronos_logo.png



void ui_app_load(lv_obj_t **screen, void (*screen_init)(void));
void ui_app_exit(void);


#endif

    void ui_navScreen_screen_init();

    void navigateInfo(const char* text, const char* title, const char *directions);
#ifdef ENABLE_APP_NAVIGATION
    /** Navigation V2 status bar: updates clock only (other args ignored); no-op on other builds */
    void navigation_refresh_status_bar(const char *clock_hm, int notif_cnt, int call_cnt, bool ble_ok, uint8_t phone_batt_pct, bool wifi_ok);
#if defined(ESP32_TOUCH_LCD_35) && !defined(NAVIGATION_UI_LEGACY)
    /** Trip row (duration / distance / ETA time only); call after navigateInfo when nav packet fields are known */
    void navigation_ws35_set_trip_row(const char *eta_raw, const char *duration, const char *distance);
#endif
#endif
    void navIconState(bool show);
    void setNavIconPx(uint16_t x, uint16_t y, bool on);

    lv_obj_t *get_nav_screen(void);
    

    void onGameOpened(void);
    void onGameClosed(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif