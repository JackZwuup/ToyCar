#include <driver/gpio.h>
#include <stdio.h>
#include <driver/adc.h>

#include "esp_err.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_0 GPIO_NUM_17
#define LED_1 GPIO_NUM_18

#define MOTORGPIO1 GPIO_NUM_10
#define MOTORGPIO2 GPIO_NUM_9

void init_gpio() {
    gpio_set_direction(MOTORGPIO1, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTORGPIO2, GPIO_MODE_OUTPUT);

    gpio_set_direction(LED_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);
}

void app_main() {

    init_gpio();

    while(1) {
      printf("Motor spin\n");
      gpio_set_level(LED_0, 1);
      gpio_set_level(LED_1, 0);
      gpio_set_level(MOTORGPIO1, 1);
      gpio_set_level(MOTORGPIO2, 0);

      vTaskDelay(1000);
      printf("Motor spin other side\n");
      gpio_set_level(LED_0, 0);      
      gpio_set_level(MOTORGPIO1, 0);
      gpio_set_level(MOTORGPIO2, 1);

      vTaskDelay(1000);
    }


}