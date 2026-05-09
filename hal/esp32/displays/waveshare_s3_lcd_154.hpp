#pragma once

#define LGFX_USE_V1
#include "Arduino.h"
#include <LovyanGFX.hpp>
#include "pins.h"

/* Waveshare ESP32-S3-LCD-1.54 — ST7789 240x240, optional touch (CST816S compatible). */

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Bus_SPI _bus_instance;

#if defined(I2C_SDA) && (I2C_SDA != -1) && defined(I2C_SCL) && (I2C_SCL != -1) && defined(TP_INT) && (TP_INT != -1)
  lgfx::Touch_CST816S _touch_instance;
#define WAVESHARE_S3_LCD_154_HAS_TOUCH 1
#else
#define WAVESHARE_S3_LCD_154_HAS_TOUCH 0
#endif

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI;
      cfg.spi_mode = 0;
      cfg.freq_write = 80000000;
      cfg.freq_read = 6000000;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = SCLK;
      cfg.pin_mosi = MOSI;
      cfg.pin_miso = MISO;
      cfg.pin_dc = DC;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = CS;
      cfg.pin_rst = RST;
      cfg.pin_busy = -1;

      cfg.panel_width = SCREEN_WIDTH;
      cfg.panel_height = SCREEN_HEIGHT;
      cfg.memory_width = SCREEN_WIDTH;
      cfg.memory_height = SCREEN_HEIGHT;
      cfg.offset_x = OFFSET_X;
      cfg.offset_y = OFFSET_Y;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = RGB_ORDER;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = BL;
      cfg.invert = false;
      cfg.freq = 12000;
      cfg.pwm_channel = 0;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }

#if WAVESHARE_S3_LCD_154_HAS_TOUCH
    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 0;
      cfg.x_max = SCREEN_WIDTH;
      cfg.y_min = 0;
      cfg.y_max = SCREEN_HEIGHT;
      cfg.pin_int = TP_INT;
      cfg.pin_rst = TP_RST;
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;
      cfg.i2c_port = 0;
      cfg.i2c_addr = 0x15;
      cfg.pin_sda = I2C_SDA;
      cfg.pin_scl = I2C_SCL;
      cfg.freq = 400000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }
#endif

    setPanel(&_panel_instance);
  }
};

LGFX tft;

