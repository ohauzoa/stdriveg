#include <Arduino.h>
#include <driver/ledc.h>
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "led.h"
#include <math.h>
#include "esp_err.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_HIGH_SPEED_MODE
#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_4_BIT // Set duty resolution to 13 bits
// 40Mhz (1) 20Mhz(2) 10Mhz(3) 5Mhz(4) 2.5Mhz(5) 1.25Mhz(6) 625Khz(7) 312.5Khz(8) 156Khz(9)
// 78Khz(10) 39Khz(11) 19Khz(12) 9.765Khz(13) 4.882(14)
#define LEDC_DUTY               (1) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (4000000) // Frequency in Hertz. Set frequency at 5 kHz

static ledc_timer_config_t ledc_timer;

typedef struct {
    int duty;               // LEDC Duty parameter
    int frequency;          // LEDC Frequency parameter
    int enable;             // LEDC Enable parameter
} led_ctrl_parameter_t;

static struct
{
    struct arg_dbl *duty;
    struct arg_dbl *frequency;
    struct arg_dbl *enable;
    struct arg_end *end;
} led_ctrl_args;

static led_ctrl_parameter_t led_runtime_param = {
    .duty = 1,
    .frequency = 4000000,
    .enable = 0,
};

static int do_led_ctrl_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&led_ctrl_args);
    int frequency = 80000000;   // Max Frequency 40Mhz - Duty Bit 1
    int duty_bit = 0;

    if (nerrors != 0)
    {
        arg_print_errors(stderr, led_ctrl_args.end, argv[0]);
        return 0;
    }

    if (led_ctrl_args.duty->count)
    {
        led_runtime_param.duty = led_ctrl_args.duty->dval[0];
        // Set duty to 50%
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, led_runtime_param.duty));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
    }

    if (led_ctrl_args.frequency->count)
    {
        led_runtime_param.frequency = led_ctrl_args.frequency->dval[0];
        for(int i = 1; i < 22; i++)
        {
            if(led_runtime_param.frequency <= (frequency >> i))
            {
                duty_bit++;               
            }
            else
            {
                int val = pow(2, duty_bit) - 1;
                printf("calculate Duty Bit:%d, Duty Max Value:%d\n", (ledc_timer_bit_t)(duty_bit), val);
                ledc_timer.freq_hz = led_runtime_param.frequency;
                ledc_timer.duty_resolution = (ledc_timer_bit_t)(duty_bit);
                ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
                break;
            }
        }
        //ESP_ERROR_CHECK(ledc_set_freq(LEDC_MODE, LEDC_TIMER, led_runtime_param.frequency));
    }

    if (led_ctrl_args.enable->count)
    {
        led_runtime_param.enable = led_ctrl_args.enable->dval[0];
    }

    return 0;
}

void register_led_console_command(void)
{
    led_ctrl_args.duty         = arg_dbl0("t", NULL, "<dt>", "Set Duty value of LEDC");
    led_ctrl_args.frequency    = arg_dbl0("f", NULL, "<fr>", "Set Frequency value of LEDC");
    led_ctrl_args.enable       = arg_dbl0("e", NULL, "<en>", "Set Enable value of LEDC");
    led_ctrl_args.end          = arg_end(2);
    const esp_console_cmd_t led_ctrl_cmd = 
    {
        .command = "led",
        .help = "Set LED parameters",
        .hint = NULL,
        .func = &do_led_ctrl_cmd,
        .argtable = &led_ctrl_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&led_ctrl_cmd));
}


void led_init(void)
{
    // Set the LEDC peripheral configuration
    // Prepare and then apply the LEDC PWM timer configuration
        ledc_timer.speed_mode       = LEDC_MODE;
        ledc_timer.timer_num        = LEDC_TIMER;
        ledc_timer.duty_resolution  = LEDC_DUTY_RES;
        ledc_timer.freq_hz          = LEDC_FREQUENCY;  // Set output frequency at 5 kHz
        ledc_timer.clk_cfg          = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel;

        ledc_channel.speed_mode             = LEDC_MODE;
        ledc_channel.channel                = LEDC_CHANNEL;
        ledc_channel.timer_sel              = LEDC_TIMER;
        ledc_channel.intr_type              = LEDC_INTR_DISABLE;
        ledc_channel.gpio_num               = LEDC_OUTPUT_IO;
        ledc_channel.duty                   = 0; // Set duty to 0%
        ledc_channel.hpoint                 = 0;
        ledc_channel.flags.output_invert    = 0;

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // Set duty to 50%
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}