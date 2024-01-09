#include <driver/gpio.h>
#include <stdio.h>
#include <driver/adc.h>

#include "esp_err.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
 
// Jack = idiot

// Pin modes
#define LED_0 GPIO_NUM_17
#define LED_1 GPIO_NUM_18
#define LED_2 GPIO_NUM_8

#define LED_3 GPIO_NUM_12
#define LED_4 GPIO_NUM_13

#define SWITCH_1OFF GPIO_NUM_21
#define SWITCH_2PARK GPIO_NUM_47
#define SWITCH_3DRIVE GPIO_NUM_45
#define SWITCH_4BACK GPIO_NUM_0
#define SWITCH_5LEFT GPIO_NUM_36
#define SWITCH_6RIGHT GPIO_NUM_37
#define SWITCH_7NONE GPIO_NUM_38

#define COUNTERWISE GPIO_NUM_9

// Magic numbers
#define TWO 2
#define LDR_THRESHOLD 2000

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (10) // de output GPIO beschrijven
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (400) // Set duty to 400. ((2 ** 13) - 1) * 50% = 4095

bool carKeyValue(int onOrOff);
int valueForSwitch();
int getAdcForPwm();


void init_gpio() {
    gpio_set_direction(LED_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_4, GPIO_MODE_OUTPUT);

    gpio_set_direction(COUNTERWISE, GPIO_MODE_OUTPUT);
    

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



void app_main() {
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

    // Set duty to 50%
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

while(1) {
    analog = getAdcForPwm();
    int felHeid = (analog * 40.95);
    printf("Felheid = %d \n", felHeid);

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
        gpio_set_level(LED_0, 0); 

        break;

        case 1:
        printf("On state\n");
        break;

        case 2:
            printf("Park state\n");
                if (parkBlinkCount < 10) {
                    gpio_set_level(LED_1, 1);
                    gpio_set_level(LED_0, 1);
                    vTaskDelay(50);
                    gpio_set_level(LED_1, 0);
                    gpio_set_level(LED_0, 0);
                    vTaskDelay(50);
                    parkBlinkCount++;
                }
            gpio_set_level(LED_1, 0);
            gpio_set_level(LED_0, 0); 
        break;

        case 3:
        printf("Drive state\n");
        // Set duty to 50%
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, felHeid));
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

        case 5:
        gpio_set_level(LED_4, 1);
        vTaskDelay(20);
        gpio_set_level(LED_4, 0);
        vTaskDelay(20);
        break;

        case 6:
        gpio_set_level(LED_3, 1);
        vTaskDelay(20);
        gpio_set_level(LED_3, 0);
        vTaskDelay(20);
        break;

        case 7:
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
    int switchDrive = gpio_get_level(SWITCH_3DRIVE) << 2;
    int switchBack = gpio_get_level (SWITCH_4BACK) << 3;
    int switchLeft = gpio_get_level (SWITCH_5LEFT) << 4;
    int switchRight = gpio_get_level(SWITCH_6RIGHT) << 5;
    int switchNone= gpio_get_level (SWITCH_7NONE) << 6;

    int allTogether = switchOff + switchPark + switchDrive + switchBack;
    int allRestTogether = switchOff + switchLeft + switchRight + switchNone;

        if (allTogether % 2 == 0) 
        {
            // printf("Off state\n");
            return 0;
        }
        else if (allTogether == 1)
        {
            // printf("On state\n");
            return 1;
        }

        else if (allTogether == 3)
        {
            // printf("Park state\n");        
            return 2;
        }

        else if (allTogether == 5) {
            // printf("drive state\n");        
            return 3;
        }

        else if (allTogether == 9)
        {
            // printf("Left state\n");        
            return 4;
        }

        else if (allRestTogether == 17)
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

  printf("ADC value : %d\n", anaIn);
  return (anaIn * 100) / anaMax;

// Bereken input percentage 
}