#include <Arduino.h>
#include <SerialCommands.h>

#define GPIO_SYNC0_IN 23       //pin for the external interrupt (Input with Pullup)
#define MOSFET1 15          //pin to trigger the MOSFET (Output) pin15
#define MOSFET2 2          //pin to trigger the MOSFET (Output) pin2



void cmd_deadtime_plus(SerialCommands* sender);
void cmd_deadtime_minus(SerialCommands* sender);
void cmd_frequency(SerialCommands* sender);

void pwm_init(void);

extern SerialCommands serial_commands_;
