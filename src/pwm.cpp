#include <Arduino.h>

#include <pwm.h>
#include <driver/mcpwm.h>

static int dead_time = 5;


//called for ON command
void cmd_deadtime_plus1(SerialCommands* sender)
{
  dead_time++;
  if(dead_time > 100) dead_time = 100;

  mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, dead_time, dead_time);
  sender->GetSerial()->print("DeadTime [");
  sender->GetSerial()->print(dead_time);
  sender->GetSerial()->println("]");
}

//called for OFF command
void cmd_deadtime_minus1(SerialCommands* sender)
{
  dead_time--;
  if(dead_time < 0) dead_time = 0;
 
  mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, dead_time, dead_time);
  sender->GetSerial()->print("DeadTime [");
  sender->GetSerial()->print(dead_time);
  sender->GetSerial()->println("]");
}

esp_err_t set_frequency1(int frequency)
{
	if ((frequency < 0) || (frequency > 1000000))
	{
		//invalid frequency
		return false;
	}
 
  return mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, frequency);
}



void cmd_frequency1(SerialCommands* sender)
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
		if (set_frequency1(pwm))
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


SerialCommand cmd_deadtime_plus1_("up", cmd_deadtime_plus1);
SerialCommand cmd_deadtime_minus1_("dn", cmd_deadtime_minus1);
SerialCommand cmd_frequency1_("freq", cmd_frequency1);

void pwm_init(void)
{
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


  serial_commands_.AddCommand(&cmd_deadtime_plus1_);
  serial_commands_.AddCommand(&cmd_deadtime_minus1_);
  serial_commands_.AddCommand(&cmd_frequency1_);

}