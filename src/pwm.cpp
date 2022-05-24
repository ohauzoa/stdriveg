#include <Arduino.h>
#include <pwm.h>
#include <driver/mcpwm.h>
#include "esp_console.h"
#include "argtable3/argtable3.h"

#define GPIO_SYNC0_IN 23        //pin for the external interrupt (Input with Pullup)
#define MOSFET1 15              //pin to trigger the MOSFET (Output) pin15
#define MOSFET2 2               //pin to trigger the MOSFET (Output) pin2


static struct {
  struct arg_dbl *deadtime;
  struct arg_dbl *frequency;
  struct arg_dbl *enable;
  struct arg_end *end;
} pwm_ctrl_args;

static pwm_ctrl_parameter_t pwm_runtime_param = {
  .deadtime = 3,
  .frequency = 4000000,
  .enable = 0,
};


//called for deadtime command
static void do_deadtime_cmd(void)
{
  if(pwm_runtime_param.deadtime > 100) pwm_runtime_param.deadtime = 100;
  if(pwm_runtime_param.deadtime < 0)   pwm_runtime_param.deadtime = 0;

  mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, pwm_runtime_param.deadtime, pwm_runtime_param.deadtime);
  printf("DeadTime [%d]\n", pwm_runtime_param.deadtime);
}

//called for frequency command
static void do_frequency_cmd(void)
{
  if(pwm_runtime_param.frequency > 8000000) pwm_runtime_param.frequency = 8000000;
  if(pwm_runtime_param.frequency < 0)   pwm_runtime_param.frequency = 0;

  mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, pwm_runtime_param.frequency);
  //printf("Frequency [%d]\n", pwm_runtime_param.frequency);
}



static int do_pwm_ctrl_cmd(int argc, char **argv)
{
  int nerrors = arg_parse(argc, argv, (void **)&pwm_ctrl_args);
  if (nerrors != 0) {
      arg_print_errors(stderr, pwm_ctrl_args.end, argv[0]);
      return 0;
  }
  if (pwm_ctrl_args.deadtime->count) {
      pwm_runtime_param.deadtime = pwm_ctrl_args.deadtime->dval[0];
      mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, pwm_runtime_param.deadtime, pwm_runtime_param.deadtime);
      printf("DeadTime [%d]\n", pwm_runtime_param.deadtime);
      //do_deadtime_cmd();
  }
  if (pwm_ctrl_args.frequency->count) {
      pwm_runtime_param.frequency = pwm_ctrl_args.frequency->dval[0];
      mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, pwm_runtime_param.frequency);
      printf("Frequency [%d]\n", pwm_runtime_param.frequency);
      //do_frequency_cmd();
  }
  if (pwm_ctrl_args.enable->count) {
      pwm_runtime_param.enable = pwm_ctrl_args.enable->dval[0];
      if(pwm_runtime_param.enable){
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
        mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
      }
      else{
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_HAL_GENERATOR_MODE_FORCE_LOW);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_HAL_GENERATOR_MODE_FORCE_LOW);
        mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
      }
      printf("Enable [%d]\n", pwm_runtime_param.enable);
  }

  return 0;
}

void register_pwm_console_command(void)
{
  pwm_ctrl_args.deadtime  = arg_dbl0("t", NULL, "<deadtime>", "Set DeadTime value of PWM");
  pwm_ctrl_args.frequency = arg_dbl0("f", NULL, "<frequency>", "Set Frequency value of PWM");
  pwm_ctrl_args.enable    = arg_dbl0("e", NULL, "<enable>", "Set Enable value of PWM");
  pwm_ctrl_args.end       = arg_end(2);
  const esp_console_cmd_t pwm_ctrl_cmd = {
    .command = "pwm",
    .help = "Set PWM parameters",
    .hint = NULL,
    .func = &do_pwm_ctrl_cmd,
    .argtable = &pwm_ctrl_args
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&pwm_ctrl_cmd));
}

void pwm_init(void)
{
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOSFET1);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, MOSFET2);
  mcpwm_pin_config_t pin_config;
  pin_config.mcpwm0a_out_num = MOSFET1;
  pin_config.mcpwm0b_out_num = MOSFET2;
  //pin_config.mcpwm_sync0_in_num  = GPIO_SYNC0_IN;
  mcpwm_config_t pwm_config;
  pwm_config.frequency = pwm_runtime_param.frequency;    //frequency = 160 / 2 MHz
  pwm_config.cmpr_a = 0;       //duty cycle of PWMxA = 60.0%
  pwm_config.cmpr_b = 0;       //duty cycle of PWMxA = 60.0%
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

  mcpwm_set_pin(MCPWM_UNIT_0, &pin_config);
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);   //Configure PWM0A & PWM0B with above settings
  mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
  mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);

  mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, pwm_runtime_param.deadtime, pwm_runtime_param.deadtime);
  //mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_RED_FED_FROM_PWMXB, 5, 5);
  //mcpwm_timer_set_resolution(MCPWM_UNIT_0, MCPWM_TIMER_0, 100000000);
  mcpwm_group_set_resolution(MCPWM_UNIT_0, SOC_MCPWM_BASE_CLK_HZ);

  mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, pwm_runtime_param.frequency);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 50);
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 50);

  //mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
  //mcpwm_sync_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_SELECT_SYNC0, 0);
  
  register_pwm_console_command();
}
