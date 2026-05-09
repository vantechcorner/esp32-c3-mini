#pragma once

#include <Arduino.h>
#include <Wire.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <Arduino_GFX_Library.h>

#include "pins.h"

class DisplayWrapper
{
public:
    Arduino_GFX *gfx;
    lgfx::Light_PWM light;
    static constexpr uint16_t TOUCH_RAW_W = 320;
    static constexpr uint16_t TOUCH_RAW_H = 480;

    DisplayWrapper()
    {
        static Arduino_DataBus *bus = new Arduino_ESP32SPI(
            DC /* DC */, CS /* CS */, SCLK /* SCK */, MOSI /* MOSI */, MISO /* MISO */);
        gfx = new Arduino_ST7796(bus, RST /* RST */, 0 /* rotation */, true /* IPS */);

        auto cfg = light.config();
        cfg.pin_bl = BL;
        cfg.invert = false;
        cfg.freq = 20000;
        cfg.pwm_channel = 1;
        light.config(cfg);
    }

    bool init(void)
    {
        Wire.begin(I2C_SDA, I2C_SCL);
        init_io_expander();
        bool state = gfx->begin();
        light.init(255);
        return state;
    }

    void initDMA(void) {}

    void fillScreen(uint16_t color)
    {
        gfx->fillScreen(color);
    }

    void setRotation(uint8_t rotation)
    {
        gfx->setRotation(rotation);
    }

    void pushImage(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
    {
        gfx->draw16bitBeRGBBitmap(x, y, data, w, h);
    }

    void pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
    {
        gfx->draw16bitBeRGBBitmap(x, y, data, w, h);
    }

    void startWrite(void) {}

    uint32_t getStartCount(void)
    {
        return 0;
    }

    void endWrite(void) {}

    void setBrightness(uint8_t brightness)
    {
        light.setBrightness(brightness);
    }

    void writePixel(int32_t x, int32_t y, const uint16_t color)
    {
        gfx->writePixel(x, y, color);
    }

    bool getTouch(uint16_t *x, uint16_t *y)
    {
        uint8_t points = 0;
        if (!touch_i2c_read(0x02, &points, 1))
        {
            return false;
        }
        if ((points & 0x0F) == 0)
        {
            return false;
        }

        uint8_t data[4] = {0};
        if (!touch_i2c_read(0x03, data, 4))
        {
            return false;
        }

        uint16_t tx = ((uint16_t)(data[0] & 0x0F) << 8) | data[1];
        uint16_t ty = ((uint16_t)(data[2] & 0x0F) << 8) | data[3];

        if (tx >= TOUCH_RAW_W) tx = TOUCH_RAW_W - 1;
        if (ty >= TOUCH_RAW_H) ty = TOUCH_RAW_H - 1;

        switch (gfx->getRotation())
        {
        case 1:
            /* 90deg clockwise: (x,y) = (raw_y, raw_w-1-raw_x) */
            *x = ty;
            *y = TOUCH_RAW_W - 1 - tx;
            break;
        case 2:
            *x = TOUCH_RAW_W - 1 - tx;
            *y = TOUCH_RAW_H - 1 - ty;
            break;
        case 3:
            *x = TOUCH_RAW_H - 1 - ty;
            *y = tx;
            break;
        default:
            *x = tx;
            *y = ty;
            break;
        }
        if (*x >= SCREEN_WIDTH) *x = SCREEN_WIDTH - 1;
        if (*y >= SCREEN_HEIGHT) *y = SCREEN_HEIGHT - 1;
        return true;
    }

private:
    static constexpr uint8_t TCA9554_ADDR = 0x20;
    static constexpr uint8_t FT6336_ADDR = 0x38;

    void init_io_expander()
    {
        // Waveshare's reference firmware enables LCD rails/reset lines via TCA9554 pins 0..2.
        // Keep this local so display init remains deterministic across power cycles.
        if (!write_i2c_reg(TCA9554_ADDR, 0x03, 0xF8)) // configuration: P0..P2 outputs, others inputs
        {
            return;
        }
        write_i2c_reg(TCA9554_ADDR, 0x01, 0x00); // output port: drive low first
        delay(50);
        write_i2c_reg(TCA9554_ADDR, 0x01, 0x07); // then enable P0..P2 high
        delay(50);
    }

    bool write_i2c_reg(uint8_t addr, uint8_t reg_addr, uint8_t value)
    {
        Wire.beginTransmission(addr);
        Wire.write(reg_addr);
        Wire.write(value);
        return Wire.endTransmission() == 0;
    }

    bool touch_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t length)
    {
        Wire.beginTransmission(FT6336_ADDR);
        Wire.write(reg_addr);
        if (Wire.endTransmission() != 0)
        {
            return false;
        }

        uint32_t read_len = Wire.requestFrom((int)FT6336_ADDR, (int)length);
        if (read_len != length)
        {
            return false;
        }
        Wire.readBytes(data, length);
        return true;
    }
};

DisplayWrapper tft;

