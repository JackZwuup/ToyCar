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


// Magic numbers
#define TWO 2
#define LDR_THRESHOLD 2000

bool carKeyValue(int onOrOff);

void init_gpio() {
    gpio_set_direction(LED_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_2, GPIO_MODE_OUTPUT);

    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
}

void adc_init(){
// We start with the configuration of the resolution.
  adc1_config_width(ADC_WIDTH_BIT_DEFAULT);  

// Then you set the range.
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11);
}

void app_main() {
    // This determines whether the car is on or off thanks to the readings of a key.
    int onOrOff = 0;

    // Read ldr's old current
    int ldr_cur_old = 0;

    // get adc info
    adc_init();
    // Get gpio info
    init_gpio();


while(1) {

    // get ldr adc value.
    int ldr_value = adc1_get_raw(ADC1_CHANNEL_1);

    printf("adc value: %d\n", ldr_value);

    // determine wheter the key is present or not.
    if (ldr_value >= LDR_THRESHOLD && ldr_cur_old == 0) {
        onOrOff++;
    }
    // if ldr value is higher than ldr threshold ldr cur old = 1 if not it's 0.
    ldr_cur_old = (ldr_value > LDR_THRESHOLD) ? 1 : 0;

    // Determine the car key value.
    bool CarState = carKeyValue(onOrOff);

    if (CarState) {

    }

    else {
        printf("Car is off\n");
    }





    vTaskDelay(50);
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