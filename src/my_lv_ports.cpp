#include "my_lv_ports.h"
// TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
//TFT_eSPI tft = TFT_eSPI(screenHeight,screenWidth); /* TFT instance */
#define TFT_CS          14
#define TFT_RST         33   // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC          27
#define CS_PIN          12
Adafruit_ST7735 *gfx = new Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);



//Arduino_DataBus *bus = new Arduino_ESP32SPI(2 /* DC */, -1 /* CS */, 4 /* SCK */, 6 /* MOSI */, -1 /* MISO */, FSPI /* spi_num */);
/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
//Arduino_GFX *gfx = new Arduino_ST7789(bus, 1 /* RST */, 3 /* rotation */);

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  int16_t touchX, touchY, touchZ;

  //touchZ = ns2009_read(NS2009_LOW_POWER_READ_Z1); //压力值
  //touchX = ns2009_read(NS2009_LOW_POWER_READ_X);
  //touchY = ns2009_read(NS2009_LOW_POWER_READ_Y);
  //touchX = touchX * SCREEN_X_PIXEL / 4096; // 4096 = 2 ^ 12
  //touchY = SCREEN_Y_PIXEL - touchY * SCREEN_Y_PIXEL / 4096;

  if (touchZ < 30) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchY;
    data->point.y = touchX;
#if LV_USE_LOG != 0
    Serial.printf("Touch: x=%d y=%d\r\n", touchX, touchY);
#endif
  }
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  //gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  gfx->drawRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready(disp);
}

#if LV_USE_LOG != 0
void my_print(const char *buf) { Serial.printf("%s \r\n", buf); }
#endif

void my_disp_init(void) {
  // 绘图缓冲初始化
  static lv_disp_draw_buf_t draw_buf;
  static lv_color_t buf[screenWidth * 30];
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 30);

  // TFT驱动初始化
     // Init Display
  // gfx->begin();
   gfx->initR(INITR_GREENTAB);
   gfx->setRotation(1);
   gfx->fillScreen(ST77XX_BLACK);

  // 设置LVGL显示设备
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // 设置LVGL输入设备（电阻屏）
  // static lv_indev_drv_t indev_drv;
  // lv_indev_drv_init(&indev_drv);
  // indev_drv.type = LV_INDEV_TYPE_POINTER;
  // indev_drv.read_cb = my_touchpad_read;
  // lv_indev_drv_register(&indev_drv);

// 设置LVGL串口输出设备（调试用）
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif
}
