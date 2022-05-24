#include <Arduino.h>

typedef struct {
    int deadtime;           // PWM DeadTime parameter
    int frequency;          // PID Frequency parameter
    int enable;             // PID Enable parameter
} pwm_ctrl_parameter_t;

void pwm_init(void);
void register_pwm_console_command(void);

