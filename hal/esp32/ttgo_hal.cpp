#if defined(TTGO_TDISPLAY)

#include "Arduino.h"
#include <ChronosESP32.h>
#include "ttgo_hal.h"
#include "faces/ttgo_dashboard/ttgo_dash_internals.h"

extern ChronosESP32 watch;

extern "C" void ttgo_hal_dashboard_loop(void)
{
  if (ttgo_dash_lbl_time == NULL)
  {
    return;
  }

  String tline;
  if (watch.is24Hour())
  {
    tline = watch.getTime("%H:%M");
  }
  else
  {
    tline = watch.getHourZ() + String(":") + watch.getTime("%M") + String(" ") + watch.getAmPmC(false);
  }
  lv_label_set_text(ttgo_dash_lbl_time, tline.c_str());
  lv_label_set_text(ttgo_dash_lbl_weekday, watch.getTime("%A").c_str());
  lv_label_set_text(ttgo_dash_lbl_date, watch.getTime("%d/%m/%Y").c_str());
}
#endif
