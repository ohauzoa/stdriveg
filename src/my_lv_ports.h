#ifndef _MY_LV_PORTS
#define _MY_LV_PORTS
#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
//#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <lvgl.h>

// 触控
//#include "NS2009.h"
#define ESP32_I2C_SDA 33
#define ESP32_I2C_SCL 25

/*Change to your screen resolution*/
const uint16_t screenWidth = 160;
const uint16_t screenHeight = 128;

void my_disp_init(void); // 挂载lvgl接口，设置buffer
#endif
