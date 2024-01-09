#include <driver/gpio.h>
// For adc.
#include <driver/adc.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define HAL_EFFECT GPIO_NUM_46

#define LED_0 GPIO_NUM_17
#define LED_1 GPIO_NUM_18

#define MOTORGPIO1 GPIO_NUM_10
#define MOTORGPIO2 GPIO_NUM_9

void adc_init(){
  // We start with the configuration of the resolution.
  adc1_config_width(ADC_WIDTH_BIT_DEFAULT);  

  // Then you set the range.
  // For LDR
  adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN_DB_11);

}


void app_main() {
    gpio_set_direction(HAL_EFFECT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HAL_EFFECT, GPIO_PULLUP_ONLY);

    gpio_set_direction(MOTORGPIO1, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTORGPIO2, GPIO_MODE_OUTPUT);

    gpio_set_direction(LED_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);

    adc_init();

    while(1) {
        // printf("Hall effect value: %d\n", gpio_get_level(HAL_EFFECT));
    
    printf("Motor spin\n");
      printf("Hall effect adc value: %d\n", adc1_get_raw(ADC1_CHANNEL_2)); 
      gpio_set_level(LED_0, 1);
      gpio_set_level(LED_1, 0);
      gpio_set_level(MOTORGPIO1, 1);
      gpio_set_level(MOTORGPIO2, 0);

      vTaskDelay(100);
      printf("Motor spin other side\n");
              printf("Hall effect adc value: %d\n", adc1_get_raw(ADC1_CHANNEL_2)); 
      gpio_set_level(LED_0, 0);      
      gpio_set_level(MOTORGPIO1, 0);
      gpio_set_level(MOTORGPIO2, 1);

      vTaskDelay(25);
    }

}