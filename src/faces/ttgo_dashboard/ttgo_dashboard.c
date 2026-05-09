/*
 * TTGO T-Display: simple home — 1/3 time (HH:MM) | 2/3 weekday + date
 */

#include "ttgo_dashboard.h"
#include "ttgo_dash_internals.h"
#include "ui/ui.h"

#define COL_BG 0x000000
#define COL_CYAN 0x00BFFF
#define COL_WHITE 0xFFFFFF

lv_obj_t *ui_ttgoHome;
lv_obj_t *ttgo_dash_lbl_time;
lv_obj_t *ttgo_dash_lbl_weekday;
lv_obj_t *ttgo_dash_lbl_date;

void init_ttgo_dashboard(void (*registerWatchface)(const char *name, const lv_image_dsc_t *preview, lv_obj_t **watchface,
                                                    lv_obj_t **seconds))
{
    lv_coord_t dtw = 240, dth = 135;
    lv_display_t *disp = lv_display_get_default();
    if (disp)
    {
        dtw = lv_display_get_horizontal_resolution(disp);
        dth = lv_display_get_vertical_resolution(disp);
    }

    /* Slightly wider left column so large HH:MM (or 12h) fits */
    const lv_coord_t w_left = (lv_coord_t)(dtw * 9 / 20);
    const lv_coord_t w_right = dtw - w_left;

    ui_ttgoHome = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_ttgoHome, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(ui_ttgoHome, dtw, dth);
    lv_obj_set_pos(ui_ttgoHome, 0, 0);
    lv_obj_set_style_bg_color(ui_ttgoHome, lv_color_hex(COL_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_ttgoHome, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_ttgoHome, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_ttgoHome, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(ui_ttgoHome, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ui_ttgoHome, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    /* One row, two columns: fixed pixel widths (avoids % / flex_grow glitches on 240x135) */
    lv_obj_t *col_l = lv_obj_create(ui_ttgoHome);
    lv_obj_set_size(col_l, w_left, dth);
    lv_obj_set_flex_flow(col_l, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col_l, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(col_l, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(col_l, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(col_l, 2, LV_PART_MAIN);

    ttgo_dash_lbl_time = lv_label_create(col_l);
    lv_label_set_text(ttgo_dash_lbl_time, "12:00");
    /* Montserrat is regular only; large size reads heavier (“đậm” hơn) */
    lv_obj_set_style_text_font(ttgo_dash_lbl_time, &lv_font_montserrat_40, LV_PART_MAIN);
    lv_obj_set_style_text_color(ttgo_dash_lbl_time, lv_color_hex(COL_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_align(ttgo_dash_lbl_time, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_long_mode(ttgo_dash_lbl_time, LV_LABEL_LONG_CLIP);

    lv_obj_t *col_r = lv_obj_create(ui_ttgoHome);
    lv_obj_set_size(col_r, w_right, dth);
    lv_obj_set_flex_flow(col_r, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col_r, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(col_r, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(col_r, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(col_r, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_row(col_r, 4, LV_PART_MAIN);

    ttgo_dash_lbl_weekday = lv_label_create(col_r);
    lv_label_set_text(ttgo_dash_lbl_weekday, "Friday");
    lv_obj_set_width(ttgo_dash_lbl_weekday, w_right - 8);
    lv_obj_set_style_text_font(ttgo_dash_lbl_weekday, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(ttgo_dash_lbl_weekday, lv_color_hex(COL_CYAN), LV_PART_MAIN);
    lv_obj_set_style_text_align(ttgo_dash_lbl_weekday, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_label_set_long_mode(ttgo_dash_lbl_weekday, LV_LABEL_LONG_CLIP);

    ttgo_dash_lbl_date = lv_label_create(col_r);
    lv_label_set_text(ttgo_dash_lbl_date, "24/04/2026");
    lv_obj_set_width(ttgo_dash_lbl_date, w_right - 8);
    lv_obj_set_style_text_font(ttgo_dash_lbl_date, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(ttgo_dash_lbl_date, lv_color_hex(COL_WHITE), LV_PART_MAIN);
    lv_obj_set_style_text_align(ttgo_dash_lbl_date, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_label_set_long_mode(ttgo_dash_lbl_date, LV_LABEL_LONG_CLIP);

    registerWatchface("TTGO", &digital_preview, &ui_ttgoHome, NULL);
}
