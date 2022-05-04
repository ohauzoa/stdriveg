#include <Arduino.h>
//#include <WiFi.h>
#include <spi.h>

#include <driver/mcpwm.h>
#include <SerialCommands.h>
//#include <testHttp.h>
#include <BluetoothA2DPSink.h>
//#include <NoodleSynth.h>
#include "BluetoothSerial.h"
#include "thread.h"
#include <IRremote.hpp>


int RECV_PIN = 4;

IRrecv irrecv(RECV_PIN);

decode_results results;


BluetoothA2DPSink a2dp_sink;
BluetoothSerial SerialBT;


const int led_1 = 5;
//const int led_2 = 15;

char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");
int dead_time = 5;

//This is the default handler, and gets called when no other command matches. 
void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}


//called for ON command
void cmd_deadtime_plus(SerialCommands* sender)
{
  dead_time++;
  if(dead_time > 100) dead_time = 100;

  mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, dead_time, dead_time);
  sender->GetSerial()->print("DeadTime [");
  sender->GetSerial()->print(dead_time);
  sender->GetSerial()->println("]");
}

//called for OFF command
void cmd_deadtime_minus(SerialCommands* sender)
{
  dead_time--;
  if(dead_time < 0) dead_time = 0;
 
  mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, dead_time, dead_time);
  sender->GetSerial()->print("DeadTime [");
  sender->GetSerial()->print(dead_time);
  sender->GetSerial()->println("]");
}

bool set_frequency(int frequency)
{
	if ((frequency < 0) || (frequency > 1000000))
	{
		//invalid frequency
		return false;
	}
 
  mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, frequency);
}



void cmd_frequency(SerialCommands* sender)
{
	//Note: Every call to Next moves the pointer to next parameter

	char* frequency = sender->Next();
	if (frequency == NULL)
	{
		sender->GetSerial()->println("ERROR NO_frequency");
		return;
	}
	int pwm = atoi(frequency);

	char* frequency_str;
	while ((frequency_str = sender->Next()) != NULL)
	{
		if (set_frequency(pwm))
		{
			sender->GetSerial()->print("FREQUENCY_STATUS ");
			sender->GetSerial()->print(frequency_str);
			sender->GetSerial()->print(" ");
			sender->GetSerial()->println(pwm);
		}
		else
		{
			sender->GetSerial()->print("ERROR ");
			sender->GetSerial()->println(frequency_str);
		}
	}
}


SerialCommand cmd_deadtime_plus_("up", cmd_deadtime_plus);
SerialCommand cmd_deadtime_minus_("dn", cmd_deadtime_minus);
SerialCommand cmd_frequency_("freq", cmd_frequency);


void show_task_info(void);



TaskHandle_t Task1;
TaskHandle_t Task2;

void Task1code( void * parameter ){
    uint32_t val = *((volatile uint32_t *) (0x3FF5E000));
    *((volatile uint32_t *) (0x3FF5E000)) &= 0x0;
    Serial.print("Task1 is running on core ");
    Serial.println(xPortGetCoreID());

  for(;;){
    //digitalWrite(led_1, HIGH);
      val = *((volatile uint32_t *) (0x3FF5E000));
      Serial.print("t -> 0x");
      Serial.print(val, HEX);
      Serial.println();
    //digitalWrite(5, HIGH);
    delay(500);
    //digitalWrite(5, LOW);
    delay(500);
  }
}

void Task2code( void * parameter ){
  Serial.print("Task2 is running on core ");
  Serial.println(xPortGetCoreID());
  pinMode(led_1, OUTPUT);

  for(;;){
    //  serial_commands_.ReadSerial();
    digitalWrite(led_1, HIGH);
    delay(1000);
    digitalWrite(led_1, LOW);
    delay(1000);
  }
}


void logMemory() {
  log_d("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
}

#define GPIO_SYNC0_IN 23       //pin for the external interrupt (Input with Pullup)
#define MOSFET1 15          //pin to trigger the MOSFET (Output) pin15
#define MOSFET2 2          //pin to trigger the MOSFET (Output) pin2




/*
synthEngine mixer(20E3);
const char song[] PROGMEM = {"MissionImp:d=16,o=6,b=150:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,16g,8p,16g,8p,16a#,16p,16c1,16p,16g,8p,16g,8p,16f,16p,16f#,16p,16g,8p,16g,8p,16a#,16p,16c1,16p,16g,8p,16g,8p,16f,16p,16f#,16p,16a#,16g,2d,32p,16a#,16g,2c#,32p,16a#,16g,2c,16a#-1,8c,2p,32p,16a#-1,16g-1,2f#,32p,16a#-1,16g-1,2f,32p,16a#-1,16g-1,2e,16d#,8d"};
const char song2[] PROGMEM = {":d=16,o=4,b=150:8d,16d#,2e,16g-1,16a#-1,32p,2f,16g-1,16a#-1,32p,2f#,16g-1,16a#-1,32p,2p,8c,16a#-1,2c,16g,16a#,32p,2c#,16g,16a#,32p,2d,16g,16a#,16p,16f#,16p,16f,8p,16g,8p,16g,16p,16c1,16p,16a#,8p,16g,8p,16g,16p,16f#,16p,16f,8p,16g,8p,16g,16p,16c1,16p,16a#,8p,16g,8p,16g,32g,32f#,32f,32e,32d#,32d,32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d"};
const char song3[] PROGMEM = ":o=6,d=2:16a,16b,8a,4b";
const char song4[] PROGMEM = ":o=5,d=2:32g,32a,32b,32c";
MusicWithoutDelay instrument(song);          //d=4 means that every note without a number in front of the letter is assumed to be a quarter note.
MusicWithoutDelay instrument2(song2);
MusicWithoutDelay instrument3(song3);
MusicWithoutDelay instrument4(song4);
*/


// for esp_a2d_connection_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv426esp_a2d_connection_state_t
void connection_state_changed(esp_a2d_connection_state_t state, void *ptr){
  Serial.println(a2dp_sink.to_str(state));
}

// for esp_a2d_audio_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv421esp_a2d_audio_state_t
void audio_state_changed(esp_a2d_audio_state_t state, void *ptr){
  Serial.println(a2dp_sink.to_str(state));
}

void setup() {
  Serial.begin(115200);
  //pinMode(5, OUTPUT);
  //pinMode(led_2, OUTPUT);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOSFET1);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, MOSFET2);
  mcpwm_pin_config_t pin_config;
  pin_config.mcpwm0a_out_num = MOSFET1;
  pin_config.mcpwm0b_out_num = MOSFET2;
  //pin_config.mcpwm_sync0_in_num  = GPIO_SYNC0_IN;
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 250000;    //frequency = 160 / 2 MHz
  pwm_config.cmpr_a = 0;       //duty cycle of PWMxA = 60.0%
  pwm_config.cmpr_b = 0;       //duty cycle of PWMxA = 60.0%
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

  mcpwm_set_pin(MCPWM_UNIT_0, &pin_config);
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);   //Configure PWM0A & PWM0B with above settings
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 50);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 50);

  mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 5, 5);
  //mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_RED_FED_FROM_PWMXB, 5, 5);

  mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, 250000);
  mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
  //mcpwm_sync_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_SELECT_SYNC0, 0);



  xTaskCreatePinnedToCore(Task1code,"Task1",2000,NULL,1,&Task1,0);
  xTaskCreatePinnedToCore(Task2code,"Task2",2000,NULL,15,&Task2,1);

 // init_HttpServer();
 /*
    static const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = 44100, // corrected by info from bluetooth
        .bits_per_sample = (i2s_bits_per_sample_t) 16, // the DAC module will only take the 8bits from MSB/
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = 0, // default interrupt priority
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false
    };

    a2dp_sink.set_i2s_config(i2s_config);
    a2dp_sink.start("MyMusic");
*/

  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100, // updated automatically by A2DP
      .bits_per_sample = (i2s_bits_per_sample_t)16,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t) (I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = true // avoiding noise in case of data unavailability
};

/* i2s pinout
static const i2s_pin_config_t i2s_pin_config  = {
.bck_io_num = 26,
.ws_io_num = 25,
.data_out_num = 27,
.data_in_num = -1
};
*/
// now configure i2s with constructed pinout and config
//i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
//i2s_set_pin(I2S_NUM_0, &i2s_pin_config);


	//enable MCLK on GPIO0
	REG_WRITE(PIN_CTRL, 0xFF0); 
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
  a2dp_sink.set_on_connection_state_changed(connection_state_changed);
  a2dp_sink.set_on_audio_state_changed(audio_state_changed);

  a2dp_sink.set_i2s_config(i2s_config);  
  a2dp_sink.start("InternalDAC");  

  SerialBT.begin("auzoa70"); //Bluetooth device 이름


  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_deadtime_plus_);
  serial_commands_.AddCommand(&cmd_deadtime_minus_);
  serial_commands_.AddCommand(&cmd_frequency_);

  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Free heap: %d", ESP.getFreeHeap());
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  log_d("Free PSRAM: %d", ESP.getFreePsram());

  logMemory();
  byte* psdRamBuffer = (byte*)ps_malloc(500000);
  logMemory();
  log_d("Free PSRAM: %d", ESP.getFreePsram());
  free(psdRamBuffer);
  log_d("Free PSRAM: %d", ESP.getFreePsram());
  logMemory();
  uint8_t *tt;
  tt = (uint8_t *)ps_malloc(100);
  logMemory();


  // put your setup code here, to run once:
  //instrument.begin(CHB, TRIANGLE, ENVELOPE0, 0);
  //instrument2.begin(TRIANGLE, ENVELOPE0, 0);
  //instrument3.begin(TRIANGLE, ENVELOPE0, 0);
  //instrument4.begin(TRIANGLE, ENVELOPE0, 0);
//    instrument.begin(CHB, TRIANGLE, ENVELOPE0, 0);
//  instrument2.begin(TRIANGLE, ENVELOPE0, 0);

//  instrument.overrideSustain(true); //let's override sustain and make some cool effects :)
//  instrument2.overrideSustain(true);
//  instrument2.reverse(!instrument2.isBackwards());  //plays the song backwards(since it was already backwards to begin with, this function reassures you that the song will sound as if it was playing forwards)

//  unsigned long t = instrument.getTotalTime();  //spits out total time in milliseconds
//  Serial.println(F("Type '1','2','3', or '4' to mute the corresponding instrument."));

//  Serial.print("Total Time: "); Serial.println(t / 1E3);
//  ledcSetup (1,1E5,12);
//  ledcAttachPin(4,1);
//ledcWriteNote(1, melody_note[1], 4);
irrecv.enableIRIn(); // Start the receiver


TaskInit();
  Serial.println("Ready!");
}

bool is_active = true;
   

void loop() {
  // put your main code here, to run repeatedly:
  //serial_commands_.ReadSerial();
    //delay(50);
  if (Serial.available()) {

    SerialBT.write(Serial.read());

  }

  if (SerialBT.available()) {

    Serial.write(SerialBT.read());

  }

  if (irrecv.decode(&results))
  {
    Serial.println(results.value, HEX);
    irrecv.resume();
  }

//  instrument.update();  //but its important to recognize that this library depends
//  instrument2.update();
//  instrument3.update();
//  instrument4.update();

}
