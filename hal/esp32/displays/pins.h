/*
   MIT License

  Copyright (c) 2023 Felix Biego

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ______________  _____
  ___  __/___  /_ ___(_)_____ _______ _______
  __  /_  __  __ \__  / _  _ \__  __ `/_  __ \
  _  __/  _  /_/ /_  /  /  __/_  /_/ / / /_/ /
  /_/     /_.___/ /_/   \___/ _\__, /  \____/
                              /____/

*/

#pragma once

#ifdef ELECROW_C3

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 4
#define I2C_SCL 5
#define TP_INT 0
#define TP_RST -1

// display
#define SPI SPI2_HOST

#define SCLK 6
#define MOSI 7
#define MISO -1
#define DC 2
#define CS 10
#define RST -1

#define BL -1 // unused (connected on IO extender)
#define VIBRATION_PIN 0 // dummy (connected on IO extender)

#define BUZZER_PIN 3

#define MAX_FILE_OPEN 10

#elif ELECROW_S3

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 296
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 4
#define I2C_SCL 3
#define TP_INT 2
#define TP_RST 5

// display
// #define SPI SPI2_HOST

#define SCLK 7
#define MOSI 8
#define MISO -1
#define DC 10
#define CS 9
#define RST 6
#define LCD_EN 40

#define BL 13 
#define VIBRATION_PIN 45

#define BUZZER_PIN -1

#define ENCODER_A 20
#define ENCODER_B 19
#define BUTTON_HOME 0

#define MAX_FILE_OPEN 10

#elif ESPC3

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 4
#define I2C_SCL 5
#define TP_INT 0
#define TP_RST 1

// display
#define SPI SPI2_HOST

#define SCLK 6
#define MOSI 7
#define MISO -1
#define DC 2
#define CS 10
#define RST -1

#define BL 3

#define BUZZER_PIN -1

#define MAX_FILE_OPEN 10

#elif ESPS3_1_28

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 6
#define I2C_SCL 7
#define TP_INT 5
#define TP_RST 13

// display
#define SPI SPI2_HOST

#define SCLK 10
#define MOSI 11
#define MISO 12
#define DC 8
#define CS 9
#define RST 14

#define BL 2

#define BUZZER_PIN -1

#define MAX_FILE_OPEN 50

#elif ESPS3_1_69

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 280
#define OFFSET_X 0
#define OFFSET_Y 20
#define RGB_ORDER true

// touch
#define I2C_SDA 11
#define I2C_SCL 10
#define TP_INT 14
#define TP_RST 13

// display
#define SPI SPI2_HOST

#define SCLK 6
#define MOSI 7
#define MISO -1
#define DC 4
#define CS 5
#define RST 8

#define BL 15

#define BUZZER_PIN -1 //33

#define MAX_FILE_OPEN 20

#elif ESPS3_2_06

#define SCREEN_WIDTH 410
#define SCREEN_HEIGHT 502

#define LCD_CS 12
#define LCD_SCK 11
#define LCD_SD0 4
#define LCD_SD1 5
#define LCD_SD2 6
#define LCD_SD3 7
#define LCD_RST 8

#define TOUCH_SDA 15
#define TOUCH_SCL 14
#define TOUCH_RST 9
#define TOUCH_IRQ 38

#define MAX_FILE_OPEN 10

#elif M5_STACK_DIAL

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define BUZZER_PIN 3

#define MAX_FILE_OPEN 10

#elif ESPS3_1_75

#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466

#define LCD_CS 12
#define LCD_SCK 38
#define LCD_SD0 4
#define LCD_SD1 5
#define LCD_SD2 6
#define LCD_SD3 7
#define LCD_RST 39

#define TOUCH_SDA 15
#define TOUCH_SCL 14
#define TOUCH_RST 40
#define TOUCH_IRQ 11


#define MAX_FILE_OPEN 10


#elif M5_STACK_DIAL

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define BUZZER_PIN 3

#define MAX_FILE_OPEN 10

#elif VIEWE_SMARTRING
#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466

#define LCD_CS 7
#define LCD_SCK 13
#define LCD_SD0 12
#define LCD_SD1 8
#define LCD_SD2 14
#define LCD_SD3 9
#define LCD_RST 11
#define LCD_EN 40

#define TOUCH_SDA 41
#define TOUCH_SCL 45
#define TOUCH_RST 46
#define TOUCH_IRQ 42

#define MAX_FILE_OPEN 10

#elif VIEWE_KNOB_15
#define SCREEN_WIDTH 466
#define SCREEN_HEIGHT 466

#define LCD_CS 12
#define LCD_SCK 10
#define LCD_SD0 13
#define LCD_SD1 11
#define LCD_SD2 14
#define LCD_SD3 9
#define LCD_RST 8
#define LCD_EN 17

#define TOUCH_SDA 1
#define TOUCH_SCL 3
#define TOUCH_RST 2
#define TOUCH_IRQ 4

#define ENCODER_A 6
#define ENCODER_B 5
#define BUTTON_HOME 0

#define MAX_FILE_OPEN 10

#elif VIEWE_2_8

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#define DC 41
#define CS 42
#define SCK 40
#define MOSI 45
#define MISO -1
#define RST 39

#define BL 13

#define LCD_IM0 47
#define LCD_IM1 48

#define I2C_SDA 1
#define I2C_SCL 3
#define TOUCH_INT 4
#define TOUCH_RST 2

#define TOUCH_ADDR 0x2E

#define BUZZER_PIN 38

#define MAX_FILE_OPEN 10

#define BUTTON_HOME 0

#elif ESP32_CYD

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA -1
#define I2C_SCL -1
#define TP_INT 36
#define TP_RST -1

#define TP_SCLK 25
#define TP_MOSI 32
#define TP_MISO 39
#define TP_CS 33

// display
#define SPI HSPI_HOST

#define SCLK 14
#define MOSI 13
#define MISO 12
#define DC 2
#define CS 15
#define RST -1

#define BL 21

#define BUZZER_PIN -1
#define BUTTON_HOME 0

#define MAX_FILE_OPEN 10

#elif defined(TTGO_TDISPLAY)

// TTGO T-Display ESP32 — ST7789V 1.14" 135x240, UI landscape 240x135
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135
/* Visible area inside 240x240 GRAM; required or top shows garbage and UI is shifted (LovyanGFX issue #159, TFT_eSPI CGRAM) */
#define OFFSET_X 52
#define OFFSET_Y 40
#define RGB_ORDER false

#define I2C_SDA -1
#define I2C_SCL -1
#define TP_INT -1
#define TP_RST -1

#define SPI VSPI_HOST

#define SCLK 18
#define MOSI 19
#define MISO -1
#define DC 16
#define CS 5
#define RST 23
#define BL 4

#define BUZZER_PIN -1
/* PRG/BOOT=GPIO0, right button=GPIO35 (LilyGo schematic) */
#define TTGO_BUTTON_A 0
#define TTGO_BUTTON_B 35
#define BUTTON_HOME TTGO_BUTTON_A
#define MAX_FILE_OPEN 10

#elif defined(WAVESHARE_S3_LCD_154)

// Waveshare ESP32-S3-LCD-1.54 (square LCD)
// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch (touch variant)
#define I2C_SDA 42
#define I2C_SCL 41
#define TP_INT 48
#define TP_RST 47

// display (SPI)
#define SPI SPI2_HOST
#define SCLK 38
#define MOSI 39
#define MISO -1
#define DC 45
#define CS 21
#define RST 40
#define BL 46

// physical keys
#define KEY_MINUS 0
#define KEY_PLUS 4
#define KEY_PWR 5

#define BUZZER_PIN -1
#define MAX_FILE_OPEN 20

#elif defined(ESP32_TOUCH_LCD_35)

// Waveshare ESP32-Touch-LCD-3.5 (ESP32, ST7796 + FT6336)
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// Touch / PMU I2C
#define I2C_SDA 21
#define I2C_SCL 22
#define TP_INT -1
#define TP_RST -1

// Display SPI
#define SCLK 18
#define MOSI 23
#define MISO 19
#define DC 27
#define CS 5
#define RST -1
#define BL 25
#define KEY_PWR 36

#define BUZZER_PIN -1
#define MAX_FILE_OPEN 20
#define LV_BUFFER_LINES 40

#else

// screen configs
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define OFFSET_X 0
#define OFFSET_Y 0
#define RGB_ORDER false

// touch
#define I2C_SDA 21
#define I2C_SCL 22
#define TP_INT 14
#define TP_RST 5

// display
#define SPI VSPI_HOST

#define SCLK 18
#define MOSI 23
#define MISO -1
#define DC 4
#define CS 15
#define RST 13

#define BL 2

#define BUZZER_PIN -1
#define BUTTON_HOME 0

#define MAX_FILE_OPEN 10

#endif
