#include <Arduino.h>
#include "BluetoothSerial.h"
#include "BluetoothA2DPSink.h"
#include "WiFi.h"

#define WIFI_NETWORK "KT_GiGA_F069"
#define WIFI_PASSWORD "eec52cz398"
#define WIFI_TIMEOUT_MS 20000 // 20 second WiFi connection timeout
#define WIFI_RECOVER_TIME_MS 30000 // Wait 30 seconds after a failed connection attempt

TaskHandle_t Task3;
TaskHandle_t Task4;
TaskHandle_t Task5;


//Task3code: blinks an LED every 1000 ms
void Task3code( void * pvParameters ){


  for(;;){
  Serial.print("Task3 running on core ");
  Serial.println(xPortGetCoreID());
    //delay(1000);
    //digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  } 
}

//Task4code: blinks an LED every 700 ms
void Task4code( void * pvParameters ){

  for(;;){
  Serial.print("Task4 running on core ");
  Serial.println(xPortGetCoreID());
    ////digitalWrite(led2, HIGH);
    delay(700);
    //digitalWrite(led2, LOW);
    delay(700);
  }
}

void keepWiFiAlive(void * parameter){
    for(;;){
        if(WiFi.status() == WL_CONNECTED){
            //vTaskDelay(10000 / portTICK_PERIOD_MS);
            vTaskDelay(200);
            continue;
        }

        Serial.println("[WIFI] Connecting");
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

        unsigned long startAttemptTime = millis();

        // Keep looping while we're not connected and haven't reached the timeout
        while (WiFi.status() != WL_CONNECTED && 
                millis() - startAttemptTime < WIFI_TIMEOUT_MS){
                  vTaskDelay(200);
                }

        // When we couldn't make a WiFi connection (or the timeout expired)
		  // sleep for a while and then retry.
        if(WiFi.status() != WL_CONNECTED){
            Serial.println("[WIFI] FAILED");
            //vTaskDelay(WIFI_RECOVER_TIME_MS / portTICK_PERIOD_MS);
            vTaskDelay(200);
			  continue;
        }

        Serial.println("[WIFI] Connected: " + WiFi.localIP());
    delay(100);
    }
}

void TaskInit(void)
{
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task3code,   /* Task function. */
                    "Task3",     /* name of task. */
                    2000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    15,           /* priority of the task */
                    &Task3,      /* Task handle to keep track of created task */
                    PRO_CPU_NUM);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task4code,   /* Task function. */
                    "Task4",     /* name of task. */
                    2000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    14,           /* priority of the task */
                    &Task4,      /* Task handle to keep track of created task */
                    APP_CPU_NUM);          /* pin task to core 1 */
  delay(500); 

  xTaskCreatePinnedToCore(
                    keepWiFiAlive,
                    "keepWiFiAlive",  // Task name
                    2000,             // Stack size (bytes)
                    NULL,             // Parameter
                    18,                // Task priority
                    &Task5,             // Task handle
                    PRO_CPU_NUM);

}