#include <Arduino.h>
#include <WiFi.h>
#include "BasicOTA.hpp"
#include <cli.h>
#include <pid.h>
#include <pwm.h>
#include <led.h>
#include <vco.h>

const char* ssid = "aomr"; //Replace with your own SSID
const char* password = "aomr0104"; //Replace with your own password

BasicOTA OTA;

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <stdio.h>
#include "TFT_eSPI.h"
#define TFT_CS          14
#define TFT_RST         33   // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC          27
#define CS_PIN          12

XPT2046_Touchscreen ts(CS_PIN);
#define TIRQ_PIN  2


//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;

int mode = 0;


//TFT_eSPI tft = TFT_eSPI();

#include "my_lv_ports.h"
#include <TFT_eSPI.h>
#include <lv_demo.h>
#include <lvgl.h>



/*

void mediabuttons() {
  // play
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, ST77XX_RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, ST77XX_RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, ST77XX_GREEN);
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}

void tftAmpare() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 40);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.setTextSize(4);
  tft.println("100 mA");
}
char str[40];

void tftOutput(int val)
{
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(25, 50);
  tft.setTextColor(ST77XX_MAGENTA);
  tft.setTextSize(4);
  sprintf(str,"%d.%d A", val/10, val%10);
  tft.print(str);
  //tft.print(val%10);
  //tft.println("A" + val + "!!");
}

void dacout(int val)
{
  // dac 0 -> 0.078V,   255= 3.3V 128=1.65V
  dacWrite(25, val);
  delay(10);
}

void setup1() {
    // Use this initializer if using a 1.8" TFT screen:
    tft.initR(INITR_GREENTAB);      // Init ST7735S chip, black tab
    //tft.initB();
    tft.setRotation(1);
    
    ts.begin();
    ts.setRotation(1);
    printf("Initialized\n");

    //tft.fillScreen(ST77XX_WHITE);

    //tftAmpare();

    //Serial.begin(115200);
    printf("Startup\n");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        delay(10);printf(".");
        //("Connection Failed! Rebooting..."); delay(5000); ESP.restart();
    }

    OTA.begin(); // Setup settings

    printf("Ready\n");
    printf("\nConnected to %s IP address: %s \n", ssid, WiFi.localIP().toString());

    cli_init();
    //register_pid_console_command();
    //register_pwm_console_command();
    //register_led_console_command();
    //register_vco_console_command();
    //pwm_init();
    //led_init();
    //vco_init();
    rotary_init();
    tftOutput(0);
    dacout(0);
}

void loop1() {
    OTA.handle();
    rotary_loop();  
    delay(50);
}
*/

void setup() {
  Serial.begin(115200); /* prepare for possible serial debug */
  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() +
                  "." + lv_version_patch();

  Serial.println(LVGL_Arduino);
  Serial.println("I am LVGL_Arduino");

  lv_init();
  my_disp_init();

#if 0
  /* Create simple label */
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, LVGL_Arduino.c_str());
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
#else
  /* Try an example from the lv_examples Arduino library
     make sure to include it as written above.
  lv_example_btn_1();
 */

  // uncomment one of these demos
  // lv_demo_widgets(); // OK
  lv_demo_benchmark(); // OK
  // lv_demo_keypad_encoder();     // works, but I haven't an encoder
  // lv_demo_music();              // NOK
  // lv_demo_printer();
  // lv_demo_stress();             // seems to be OK
#endif
  Serial.println("Setup done");
}

void loop() {
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}