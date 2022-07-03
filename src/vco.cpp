#include <Arduino.h>
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "esp_err.h"
#include "si5351.h"
#include "vco.h"


#define I2C_MASTER_NUM	0
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_SCL_IO 22


typedef struct {
    int frequency;          // VCO Frequency parameter
    int enable;             // VCO Enable parameter
} vco_ctrl_parameter_t;

static struct
{
    struct arg_dbl *frequency;
    struct arg_dbl *enable;
    struct arg_end *end;
} vco_ctrl_args;

static vco_ctrl_parameter_t vco_runtime_param = {
    .frequency = 4000000,
    .enable = 0,
};

Si5351 synth;

void changeFrequency( int currentFrequency )
{
    uint64_t freq = currentFrequency * 100ULL;

    synth.set_freq(freq, SI5351_CLK0);
}

static int do_vco_ctrl_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&vco_ctrl_args);
    int frequency = 80000000;   // Max Frequency 40Mhz - Duty Bit 1
    int duty_bit = 0;

    if (nerrors != 0)
    {
        arg_print_errors(stderr, vco_ctrl_args.end, argv[0]);
        return 0;
    }


    if (vco_ctrl_args.frequency->count)
    {
        vco_runtime_param.frequency = vco_ctrl_args.frequency->dval[0];
        //synth.set_freq(vco_runtime_param.frequency, SI5351_CLK0);
        changeFrequency(vco_runtime_param.frequency);
    }

    if (vco_ctrl_args.enable->count)
    {
        vco_runtime_param.enable = vco_ctrl_args.enable->dval[0];
        synth.output_enable(SI5351_CLK0, vco_runtime_param.enable);
    }

    return 0;
}

void register_vco_console_command(void)
{
    vco_ctrl_args.frequency    = arg_dbl0("f", NULL, "<fr>", "Set Frequency value of VCO");
    vco_ctrl_args.enable       = arg_dbl0("e", NULL, "<en>", "Set Enable value of VCO");
    vco_ctrl_args.end          = arg_end(2);
    const esp_console_cmd_t vcxo_ctrl_cmd = 
    {
        .command = "vco",
        .help = "Set VCO parameters",
        .hint = NULL,
        .func = &do_vco_ctrl_cmd,
        .argtable = &vco_ctrl_args
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&vcxo_ctrl_cmd));
}

static i2c_config_t i2c_conf;

void vco_init(void)
{

    int i2c_master_port = I2C_MASTER_NUM;

    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = I2C_MASTER_SDA_IO;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_io_num = I2C_MASTER_SCL_IO;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = 100000;
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &i2c_conf));

    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, i2c_conf.mode, 0, 0, 0));


    synth.init(I2C_MASTER_NUM, SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    // Set CLK0 to output 14 MHz
    synth.set_freq(1400000000ULL, SI5351_CLK0);

    // Enable clock fanout for the XO
    synth.set_clock_fanout(SI5351_FANOUT_XO, 1);

    // Enable clock fanout for MS
    synth.set_clock_fanout(SI5351_FANOUT_MS, 1);

    // Set CLK1 to output the XO signal
    //synth.set_clock_source(SI5351_CLK1, SI5351_CLK_SRC_XTAL);
    synth.output_enable(SI5351_CLK1, 1);

    // Set CLK2 to mirror the MS0 (CLK0) output
    synth.set_clock_source(SI5351_CLK2, SI5351_CLK_SRC_MS0);
    synth.output_enable(SI5351_CLK2, 1);

    // Change CLK0 output to 10 MHz, observe how CLK2 also changes
    synth.set_freq(1234567ULL, SI5351_CLK1);
    // Change CLK0 output to 10 MHz, observe how CLK2 also changes
    synth.set_freq(423456700, SI5351_CLK0);

    synth.update_status();
}

#include "AiEsp32RotaryEncoder.h"
#include "main.h"
#define ROTARY_ENCODER_A_PIN 15
#define ROTARY_ENCODER_B_PIN 4
#define ROTARY_ENCODER_BUTTON_PIN 0
#define ROTARY_ENCODER_VCC_PIN 2

#define ROTARY_ENCODER_STEPS 4

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
const int table[51] = {0, 1, 4, 9, 14, 20, 25, 30, 36, 41, 46, 52, 57, 62, 67, 73, 78, 84, 89, 94, 99, 104, 110, 115, 121, 126, 128, 133, 139, 144, 149, 155, 160, 165, 170, 176, 181, 187, 192, 197, 202, 208, 213, 218, 224, 230, 235, 241, 246, 251, 255};

float getAmpair()
{
    return (float)rotaryEncoder.readEncoder() / 100.0;
}

void rotary_onButtonClick()
{
    static unsigned long lastTimePressed = 0;
    if (millis() - lastTimePressed < 200)
        return;
    lastTimePressed = millis();

    printf("Radio station set to %f MHz\n", getAmpair());
}

void IRAM_ATTR readEncoderISR()
{
	rotaryEncoder.readEncoder_ISR();
}


void rotary_loop()
{
    if (rotaryEncoder.encoderChanged())
    {
        int val = rotaryEncoder.readEncoder();
        //val = map(val, 0, 1000, 0, 255);
        printf("%d.%d A, %d\n", val/10, val%10, table[val]);
        tftOutput(val);
        dacWrite(25, table[val]);
     //changeFrequency(val);
    }
    if (rotaryEncoder.isEncoderButtonClicked())
    {
        rotary_onButtonClick();
    }
}

void rotary_init(void)
{
    //we must initialize rotary encoder
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    //rotaryEncoder.setBoundaries(3 * 100, 10 * 100, true); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
    rotaryEncoder.setBoundaries(0, 50, true); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
    rotaryEncoder.setAcceleration(30);
    rotaryEncoder.setEncoderValue(0); //set default to 92.1 MHz
	//rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
	//rotaryEncoder.setAcceleration(250); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

}
