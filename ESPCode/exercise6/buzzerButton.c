// For gpio functions.
#include <driver/gpio.h>
// Standard input and output.
#include <stdio.h>
// For adc.
#include <driver/adc.h>
// For PWM.
#include "driver/ledc.h"
// For delays.
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Display library
#include "ssd1306.h"

// Pin modes
#define LED_0 GPIO_NUM_17 // Carkey indication. (1 = present)
#define LED_1 GPIO_NUM_18 // Parking light.
#define LED_2 GPIO_NUM_8 // Parking light.

#define LED_3 GPIO_NUM_12 // Right blinking light.
#define LED_4 GPIO_NUM_13 // Left blinking light.

#define BUZZ GPIO_NUM_5 // Buzzer pin

#define HONK_BUTTON GPIO_NUM_15 // Button to activate buzzer.
#define BREAK_PEDAL GPIO_NUM_16 // Button for breaking

// Dipswitch states.
#define SWITCH_1OFF GPIO_NUM_21
#define SWITCH_2PARK GPIO_NUM_47
#define SWITCH_3DRIVE GPIO_NUM_45
#define SWITCH_4BACK GPIO_NUM_0
#define SWITCH_5LEFT GPIO_NUM_35
#define SWITCH_6RIGHT GPIO_NUM_36
#define SWITCH_7NONE GPIO_NUM_37

// Make the motor spin CCW.
#define COUNTERWISE GPIO_NUM_9

// Magic numbers.
#define TWO 2
#define LDR_THRESHOLD 2000

// Define I2C configuration parameters
#define I2C_MASTER_SCL_IO 6        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 7        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

// For PWM
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (10) // de output GPIO beschrijven
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (400) // Set duty to 400. ((2 ** 13) - 1) * 50% = 4095

// Declare a handle for the SSD1306 device
static ssd1306_handle_t ssd1306_dev = NULL;
volatile bool button_pressed = false;
volatile bool button_buzz = false;
bool carKeyValue(int onOrOff);
int valueForSwitch();
int otherValueForSwitch();
int getAdcForPwm();

/// @brief Determine the gpio directions.
///
/// This function determines what kind of function the gpio's have
///
/// @param
///
/// @return none
void init_gpio() {
    gpio_set_direction(LED_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_4, GPIO_MODE_OUTPUT);

    gpio_set_direction(COUNTERWISE, GPIO_MODE_OUTPUT);

    gpio_set_direction(BUZZ, GPIO_MODE_OUTPUT);

    gpio_set_direction(HONK_BUTTON,GPIO_MODE_INPUT);
    gpio_set_pull_mode(HONK_BUTTON,GPIO_PULLUP_ONLY);

    gpio_set_direction(BREAK_PEDAL,GPIO_MODE_INPUT);
    gpio_set_pull_mode(BREAK_PEDAL,GPIO_PULLUP_ONLY);

    gpio_set_direction(SWITCH_1OFF,GPIO_MODE_INPUT);
    gpio_set_pull_mode(SWITCH_1OFF,GPIO_PULLUP_ONLY);

    gpio_set_direction(SWITCH_2PARK,GPIO_MODE_INPUT);
    gpio_set_pull_mode(SWITCH_2PARK,GPIO_PULLUP_ONLY);

    gpio_set_direction(SWITCH_3DRIVE,GPIO_MODE_INPUT);
    gpio_set_pull_mode(SWITCH_3DRIVE,GPIO_PULLUP_ONLY);

    gpio_set_direction(SWITCH_4BACK,GPIO_MODE_INPUT);
    gpio_set_pull_mode(SWITCH_4BACK,GPIO_PULLUP_ONLY);

    gpio_set_direction(SWITCH_5LEFT,GPIO_MODE_INPUT);
    gpio_set_pull_mode(SWITCH_5LEFT,GPIO_PULLUP_ONLY);

    gpio_set_direction(SWITCH_6RIGHT,GPIO_MODE_INPUT);
    gpio_set_pull_mode(SWITCH_6RIGHT,GPIO_PULLUP_ONLY);

    gpio_set_direction(SWITCH_7NONE,GPIO_MODE_INPUT);
    gpio_set_pull_mode(SWITCH_7NONE,GPIO_PULLUP_ONLY);
}

void init_pwm()
{
    ledc_timer_config_t timer_config;
    ledc_channel_config_t channel_config;
    timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_config.timer_num = LEDC_TIMER_0;
    timer_config.duty_resolution = LEDC_TIMER_12_BIT;
    timer_config.freq_hz = 5000;
    timer_config.clk_cfg = LEDC_AUTO_CLK;

    channel_config.gpio_num = LEDC_OUTPUT_IO;
    channel_config.speed_mode = LEDC_LOW_SPEED_MODE;
    channel_config.channel = LEDC_CHANNEL_0;

    channel_config.timer_sel = LEDC_TIMER_0;
    channel_config.intr_type = LEDC_INTR_DISABLE;
    channel_config.duty = 0;
    channel_config.hpoint = 0;

    ledc_timer_config(&timer_config);
    ledc_channel_config(&channel_config);  
}

void adc_init(){
  // We start with the configuration of the resolution.
  adc1_config_width(ADC_WIDTH_BIT_DEFAULT);  

  // Then you set the range.
  // For LDR
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11);

  // For potmeter.
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
}

void lcd_init() {
    // Configure I2C settings
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

    // Initialize and install the I2C driver
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    // Create an instance of the SSD1306 device
    ssd1306_dev = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);
}

static void breakPedalInterrupt(void* arg) {
    button_pressed = true;
}

static void honkInterrupt(void* arg) {
    button_buzz = true;
}


void setupBreakPedal() {
    // Configure the button as input.
    esp_rom_gpio_pad_select_gpio(BREAK_PEDAL);
    gpio_set_direction(BREAK_PEDAL, GPIO_MODE_INPUT);
    // From 1 to 0 in this button's case.
    gpio_set_intr_type(BREAK_PEDAL, GPIO_INTR_NEGEDGE);

    // Install the interrupt
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BREAK_PEDAL, breakPedalInterrupt, NULL);
}

void setupHonkBuzz() {
    // Configure the button as input.
    esp_rom_gpio_pad_select_gpio(HONK_BUTTON);
    gpio_set_direction(HONK_BUTTON, GPIO_MODE_INPUT);
    // From 1 to 0 in this button's case.
    gpio_set_intr_type(HONK_BUTTON, GPIO_INTR_NEGEDGE);

    // Install the interrupt
    gpio_install_isr_service(0);
    gpio_isr_handler_add(HONK_BUTTON, honkInterrupt, NULL);
}



void app_main() {

    setupBreakPedal();
    setupHonkBuzz();

    printf("Begin or something");

    // This determines whether the car is on or off thanks to the readings of a key.
    int onOrOff = 0;
    // Read ldr's old current
    int ldr_cur_old = 0;
    int knop_cur;
    int knop_cur2;
    int knop_cur3;
    int knop_cur4;
    int knop_cur5;
    int knop_cur6;
    int knop_cur7;
    int carState = 0;
    int parkBlinkCount = 0;
    int analog = 0; 
    
    // get adc info
    adc_init();
    // Get gpio info
    init_gpio();
    // Get pwm info
    init_pwm();
    // Get lcd info
    lcd_init();


    // Set duty to 50%
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

while(1) {

    if (button_buzz) {
        button_buzz = false;
    do {
        // Stopping the car.
        printf("Beep, beep (Honking sound)\n"); 
        gpio_set_level(BUZZ, 1);

        vTaskDelay(pdMS_TO_TICKS(100)); // Delay for smoother output
    } 
    while (gpio_get_level(HONK_BUTTON) == 0);
    }
    else {
        gpio_set_level(BUZZ, 0);
    }

    if (button_pressed) {
        button_pressed = false;
    do {
        // Stopping the car.
        printf("Skrrt, skrtt (Braking sounds)\n"); 
        gpio_set_level(COUNTERWISE, 1);

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));


        vTaskDelay(pdMS_TO_TICKS(100)); // Delay for smoother output
    } 
    while (gpio_get_level(BREAK_PEDAL) == 0);
    }

    printf("\nButton state = %d\n", gpio_get_level(BREAK_PEDAL));
    analog = getAdcForPwm();
    int brightness = (analog * 40.95);
    // printf("Felheid = %d \n", brightness);

    int isKeyPresent = carKeyValue(onOrOff);
    carState = valueForSwitch();
    if (!isKeyPresent || parkBlinkCount >= 10) {
        carState = 0;
    }
    printf("carState = %d\n", carState);

    switch(carState) {
        case 0:
        printf("Off state\n");
        parkBlinkCount = 0;
        gpio_set_level(LED_1, 0);
        gpio_set_level(LED_2, 0); 
        gpio_set_level(COUNTERWISE, 1);

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
        break;

        case 1:
        printf("On state\n");
        break;

        case 2:
            printf("Park state\n");
                if (parkBlinkCount < 10) {
                    gpio_set_level(LED_1, 1);
                    gpio_set_level(LED_2, 1);
                    vTaskDelay(50); 
                    gpio_set_level(LED_1, 0);
                    gpio_set_level(LED_2, 0);
                    vTaskDelay(50);
                    parkBlinkCount++;
                }
            gpio_set_level(LED_1, 0);
            gpio_set_level(LED_2, 0); 
        break;

        case 3:
        printf("Drive state\n");
        // Set duty to 50%
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, brightness));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

        gpio_set_level(COUNTERWISE, 1);


        break;

        case 4:
        printf("Back state\n");

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

        gpio_set_level(COUNTERWISE, 0);

        break;
    }
    
    int otherCarState = otherValueForSwitch();
    switch(otherCarState) {
        case 5:
        printf("Right state\n");     
        gpio_set_level(LED_4, 1);
        vTaskDelay(20);
        gpio_set_level(LED_4, 0);
        vTaskDelay(20);
        break;

        case 6:
        printf("Left state\n");     
        gpio_set_level(LED_3, 1);
        vTaskDelay(20);
        gpio_set_level(LED_3, 0);
        vTaskDelay(20);
        break;

        case 7:
        printf("None state\n");     
        gpio_set_level(LED_3, 0);
        gpio_set_level(LED_4, 0);
        break;
    }

    // get ldr adc value.
    int ldr_value = adc1_get_raw(ADC1_CHANNEL_1);
    int pot_value = adc1_get_raw(ADC1_CHANNEL_3);

    knop_cur = gpio_get_level (SWITCH_1OFF);
    knop_cur2 = gpio_get_level (SWITCH_2PARK);
    knop_cur3 = gpio_get_level (SWITCH_3DRIVE);
    knop_cur4 = gpio_get_level (SWITCH_4BACK);
    knop_cur5 = gpio_get_level (SWITCH_5LEFT);
    knop_cur6 = gpio_get_level (SWITCH_6RIGHT);
    knop_cur7 = gpio_get_level (SWITCH_7NONE);


    printf("\nswitch state: %d %d %d %d %d %d %d\n", knop_cur, knop_cur2, knop_cur3, knop_cur4, knop_cur5, knop_cur6, knop_cur7);

        // Refresh the display and clear the screen
        ssd1306_refresh_gram(ssd1306_dev);
        ssd1306_clear_screen(ssd1306_dev, 0x00);

        // Display a string on the screen
        char data_str[30] = {0};
        sprintf(data_str, "gearbox %d %d %d %d %d %d %d", knop_cur, knop_cur2, knop_cur3, knop_cur4, knop_cur5, knop_cur6, knop_cur7);
        ssd1306_draw_string(ssd1306_dev, 0, 0, (const uint8_t *)data_str, 12, 1);
        if (carState == 0) {
            sprintf(data_str, "Off state");
        }
        else if (carState == 1) {
            sprintf(data_str, "On state");
        }
        else if (carState == 2) {
            sprintf(data_str, "Park state");
        }
        else if (carState == 3) {
            sprintf(data_str, "Drive state");
        }
        else if (carState == 4) {
            sprintf(data_str, "Back state");
        }        
        else if (carState == 5) {
            sprintf(data_str, "Right state");
        }   
        else if (carState == 6) {
            sprintf(data_str, "Left state");
        }          
        else if (carState == 7) {
            sprintf(data_str, "None state");
        }            
        else {
            sprintf(data_str, "Error state");
        }                
        ssd1306_draw_string(ssd1306_dev, 0, 14, (const uint8_t *)data_str, 12, 1);

        // Refresh the display again
        ssd1306_refresh_gram(ssd1306_dev);

    printf("LDR value: %d\n", ldr_value);
    printf("pot value: %d\n", pot_value);

    // determine wheter the key is present or not.
    if (ldr_value >= LDR_THRESHOLD && ldr_cur_old == 0) {
        onOrOff++;
    }
    // if ldr value is higher than ldr threshold ldr cur old = 1 if not it's 0.
    ldr_cur_old = (ldr_value > LDR_THRESHOLD) ? 1 : 0;


    if (valueForSwitch() == -1) {

        printf("Error");
    }

    vTaskDelay(100);
}
}

bool carKeyValue(int onOrOff) {
    // If threshold has been passed ones after car key is present it's absent.
    if (onOrOff % TWO == 0) {
        printf("Car key is absent\n");
        gpio_set_level(LED_0, 0);
        return false;

    // If threshold has been passed ones after car key is absent it's present.
    } else {
        printf("Car key is present\n");
        gpio_set_level(LED_0, 1);
        return true;
    }
}

int valueForSwitch() {
    int switchOff = gpio_get_level (SWITCH_1OFF);
    int switchPark = gpio_get_level (SWITCH_2PARK) << 1;
    int switchDrive = gpio_get_level(SWITCH_3DRIVE) << TWO;
    int switchBack = gpio_get_level (SWITCH_4BACK) << 3;

    // These dependentstates are  
    int dependentTogether = switchOff + switchPark + switchDrive + switchBack;

        if (dependentTogether % TWO == 0) 
        {
        // printf("Off state\n");
            return 0;

        }
        else if (dependentTogether == 1)
        {
            // printf("On state\n");
            return 1;
        }

        else if (dependentTogether == 3)
        {
            // printf("Park state\n");        
            return 2;
        }

        else if (dependentTogether == 5) {
            // printf("drive state\n");        
            return 3;
        }

        else if (dependentTogether == 9)
        {
            // printf("Left state\n");        
            return 4;
        }
    return -1;
}

int otherValueForSwitch() {
    int switchOff = gpio_get_level (SWITCH_1OFF);
    int switchLeft = gpio_get_level (SWITCH_5LEFT) << 4;
    int switchRight = gpio_get_level(SWITCH_6RIGHT) << 5;
    int switchNone= gpio_get_level (SWITCH_7NONE) << 6;

    int allRestTogether = switchOff + switchLeft + switchRight + switchNone;


        if (allRestTogether % TWO == 0) 
        {
        // printf("Off state\n");
            return 0;
        }
        else if (allRestTogether == 1)
        {
            // printf("On state\n");
            return 1;
        }
        if (allRestTogether == 17)
        {
            // printf("Right state\n");        
            return 5;
        }

        else if (allRestTogether == 33) {
            // printf("None state\n");        
            return 6;
        }

        else if (allRestTogether == 65)
        {
            // printf("Back state\n");        
            return 7;
        }
    return -1;
}

int getAdcForPwm() {

// Analoog input value
  int anaIn = adc1_get_raw(ADC1_CHANNEL_3);
  int anaMax = 4095;

//   printf("ADC value : %d\n", anaIn);
  return (anaIn * 100) / anaMax;

// Bereken input percentage 
}