#include <driver/gpio.h>
#include <stdio.h>
#include <driver/adc.h>

#include "esp_err.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Jack = idioot, ledbar andersom



// #define LEDBAR_LED_0 GPIO_NUM_2

#define LEDBAR_LED_0 GPIO_NUM_17 // Doet het goed
#define LEDBAR_LED_1 GPIO_NUM_18 // Doet het goed
#define LEDBAR_LED_2 GPIO_NUM_8 // Doet het goed
#define LEDBAR_LED_3 GPIO_NUM_3 // Doet het goed

#define LEDBAR_LED_4 GPIO_NUM_46 // Doet het goed
#define LEDBAR_LED_5 GPIO_NUM_9 // Doet het goed
#define LEDBAR_LED_6 GPIO_NUM_10 // Doet het goed
#define LEDBAR_LED_7 GPIO_NUM_11 // Doet het goed

#define LEDBAR_LED_8 GPIO_NUM_12 // Doet het goed
#define LEDBAR_LED_9 GPIO_NUM_13 // Doet het goed

#define LEDBAR_LED_10 GPIO_NUM_41 // Doet het niet
#define LEDBAR_LED_11 GPIO_NUM_40 // Doet het niet
#define LEDBAR_LED_12 GPIO_NUM_39 // Doet het een beetje
#define LEDBAR_LED_13 GPIO_NUM_38 // Doet het goed

#define LEDBAR_LED_14 GPIO_NUM_37 // Doet het goed
#define LEDBAR_LED_15 GPIO_NUM_36 // Doet het goed
#define LEDBAR_LED_16 GPIO_NUM_35 // Doet het goed
#define LEDBAR_LED_17 GPIO_NUM_0 // Doet het goed

#define LEDBAR_LED_18 GPIO_NUM_45 // Doet het goed
#define LEDBAR_LED_19 GPIO_NUM_48 // Doet het goed

#define LEDBAR_LED_20 GPIO_NUM_47 // Doet het redelijk goed
#define LEDBAR_LED_21 GPIO_NUM_21 // Doet het goed

#define LEDBAR_LED_22 GPIO_NUM_20 // Doet het een beetje goed
#define LEDBAR_LED_23 GPIO_NUM_19 // Doet het niet

#define LEDBAR_LED_24 GPIO_NUM_1 // Doet het niet
#define LEDBAR_LED_25 GPIO_NUM_2 // Doet het niet

#define LEDBAR_LED_26 GPIO_NUM_42 // Doet het niet
#define LEDBAR_LED_27 GPIO_NUM_19 // Doet het niet
#define LEDBAR_LED_29 GPIO_NUM_16 // Doet het goed

#define LEDBAR_LED_30 GPIO_NUM_15 // Doet het goed
#define LEDBAR_LED_31 GPIO_NUM_7 // Doet het goed

#define LEDBAR_LED_32 GPIO_NUM_6 // Doet het goed
#define LEDBAR_LED_33 GPIO_NUM_5 // Doet het goed

#define LEDBAR_LED_34 GPIO_NUM_4 // Doet het goed
#define LEDBAR_LED_35 GPIO_NUM_14 // Doet het goed





void app_main() {

    gpio_set_direction(LEDBAR_LED_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_4, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_5, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_6, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_7, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_8, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_9, GPIO_MODE_OUTPUT);

    gpio_set_direction(LEDBAR_LED_10, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_11, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_12, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_13, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_14, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_15, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_16, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_17, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_18, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_19, GPIO_MODE_OUTPUT); 
    gpio_set_direction(LEDBAR_LED_20, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_21, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_22, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_23, GPIO_MODE_OUTPUT);  
    gpio_set_direction(LEDBAR_LED_24, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_25, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_26, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_27, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_29, GPIO_MODE_OUTPUT); 
    gpio_set_direction(LEDBAR_LED_30, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_31, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_32, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_33, GPIO_MODE_OUTPUT);  
    gpio_set_direction(LEDBAR_LED_34, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDBAR_LED_35, GPIO_MODE_OUTPUT);

while(1) {
    printf("Hello Wim\n");
    gpio_set_level(LEDBAR_LED_0, 1);
    gpio_set_level(LEDBAR_LED_1, 1);
    gpio_set_level(LEDBAR_LED_2, 1);
    gpio_set_level(LEDBAR_LED_3, 1);
    gpio_set_level(LEDBAR_LED_4, 1);
    gpio_set_level(LEDBAR_LED_5, 1);
    gpio_set_level(LEDBAR_LED_6, 1);
    gpio_set_level(LEDBAR_LED_7, 1);
    gpio_set_level(LEDBAR_LED_8, 1);
    gpio_set_level(LEDBAR_LED_9, 1);
    gpio_set_level(LEDBAR_LED_10, 1);
    gpio_set_level(LEDBAR_LED_11, 1);
    gpio_set_level(LEDBAR_LED_12, 1);
    gpio_set_level(LEDBAR_LED_13, 1);
    gpio_set_level(LEDBAR_LED_14, 1);
    gpio_set_level(LEDBAR_LED_15, 1);
    gpio_set_level(LEDBAR_LED_16, 1);
    gpio_set_level(LEDBAR_LED_17, 1);
    gpio_set_level(LEDBAR_LED_18, 1);
    gpio_set_level(LEDBAR_LED_19, 1);
    gpio_set_level(LEDBAR_LED_20, 1);
    gpio_set_level(LEDBAR_LED_21, 1);
    gpio_set_level(LEDBAR_LED_22, 1);
    gpio_set_level(LEDBAR_LED_23, 1);
    gpio_set_level(LEDBAR_LED_24, 1);
    gpio_set_level(LEDBAR_LED_25, 1);
    gpio_set_level(LEDBAR_LED_26, 1);
    gpio_set_level(LEDBAR_LED_27, 1);
    gpio_set_level(LEDBAR_LED_29, 1);
    gpio_set_level(LEDBAR_LED_30, 1);
    gpio_set_level(LEDBAR_LED_31, 1);
    gpio_set_level(LEDBAR_LED_32, 1);
    gpio_set_level(LEDBAR_LED_33, 1);
    gpio_set_level(LEDBAR_LED_34, 1);    
    gpio_set_level(LEDBAR_LED_35, 1);   
    vTaskDelay(500);

    // gpio_set_level(LEDBAR_LED_0, 0);
    // gpio_set_level(LEDBAR_LED_1, 0);
    // gpio_set_level(LEDBAR_LED_2, 0);
    // gpio_set_level(LEDBAR_LED_3, 0);
    // gpio_set_level(LEDBAR_LED_4, 0);
    // gpio_set_level(LEDBAR_LED_5, 0);
    // gpio_set_level(LEDBAR_LED_6, 0);
    // gpio_set_level(LEDBAR_LED_7, 0);
    // gpio_set_level(LEDBAR_LED_8, 0);
    // gpio_set_level(LEDBAR_LED_9, 0);

    // vTaskDelay(500);
}
}