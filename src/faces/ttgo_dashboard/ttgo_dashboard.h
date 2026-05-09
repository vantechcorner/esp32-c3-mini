#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void init_ttgo_dashboard(
    void (*registerWatchface)(const char *name, const lv_image_dsc_t *preview, lv_obj_t **watchface, lv_obj_t **seconds));
extern lv_obj_t *ui_ttgoHome;

#ifdef __cplusplus
}
#endif
