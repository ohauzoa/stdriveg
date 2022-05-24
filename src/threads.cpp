#include <Arduino.h>
#include "BluetoothSerial.h"
#include "BluetoothA2DPSink.h"
#include "pid_ctrl.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "pwm.h"

#define SERIAL_STUDIO_DEBUG           0

TaskHandle_t Task3;
TaskHandle_t Task4;


static pid_ctrl_parameter_t pid_runtime_param = {
    .kp = 0.6,
    .ki = 0.3,
    .kd = 0.12,
    .max_output   = 100,
    .min_output   = -100,
    .max_integral = 1000,
    .min_integral = -1000,
    .cal_type = PID_CAL_TYPE_INCREMENTAL,
};

static bool pid_need_update = false;
static int expect_pulses = 300;
static int real_pulses;



static struct {
    struct arg_dbl *kp;
    struct arg_dbl *ki;
    struct arg_dbl *kd;
    struct arg_end *end;
} pid_ctrl_args;


static int do_pid_ctrl_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&pid_ctrl_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, pid_ctrl_args.end, argv[0]);
        return 0;
    }
    if (pid_ctrl_args.kp->count) {
        pid_runtime_param.kp = pid_ctrl_args.kp->dval[0];
    }
    if (pid_ctrl_args.ki->count) {
        pid_runtime_param.ki = pid_ctrl_args.ki->dval[0];
    }
    if (pid_ctrl_args.kd->count) {
        pid_runtime_param.kd = pid_ctrl_args.kd->dval[0];
    }

    pid_need_update = true;
    return 0;
}

static void register_pid_console_command(void)
{
  pid_ctrl_args.kp   = arg_dbl0("p", NULL, "<kp>", "Set Kp value of PID");
  pid_ctrl_args.ki   = arg_dbl0("i", NULL, "<ki>", "Set Ki value of PID");
  pid_ctrl_args.kd   = arg_dbl0("d", NULL, "<kd>", "Set Kd value of PID");
  pid_ctrl_args.end  = arg_end(2);
  const esp_console_cmd_t pid_ctrl_cmd = {
      .command = "pid",
      .help = "Set PID parameters",
      .hint = NULL,
      .func = &do_pid_ctrl_cmd,
      .argtable = &pid_ctrl_args
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&pid_ctrl_cmd));
}


float ReadBatteryVoltage(void)
{
  // Circuit now has a /2 voltage divider
  // if VBat = 4.2 then GPIO35 voltage = VBat/2 = 2.1v
  // ADC reading scaled to Voltage with 7.445 factor  
  return analogRead(35) / 4096.0 * 7.445;
}

extern void register_pwm_console_command(void);

//Task3code: blinks an LED every 1000 ms
void Task3code( void * pvParameters ){

  printf("init PID control block\r\n");
  pid_ctrl_block_handle_t pid_ctrl;
  pid_ctrl_config_t pid_config = {
      .init_param = pid_runtime_param,
  };
  ESP_ERROR_CHECK(pid_new_control_block(&pid_config, &pid_ctrl));

  printf("install console command line\r\n");
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_config.prompt = "esp32>";
  esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
  register_pid_console_command();
  //register_pwm_console_command();
  ESP_ERROR_CHECK(esp_console_start_repl(repl));

  for(;;){
    vTaskDelay(pdMS_TO_TICKS(100));
    if (pid_need_update) {
        pid_update_parameters(pid_ctrl, &pid_runtime_param);
        pid_need_update = false;
    }
  } 
}

//Task4code: blinks an LED every 700 ms
void Task4code( void * pvParameters ){
  float BatteryVoltage;

  for(;;){
    BatteryVoltage = ReadBatteryVoltage();
    //printf("volts %f\n",BatteryVoltage);
    vTaskDelay(pdMS_TO_TICKS(1000));
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
}