#include <stdio.h>
#include "ssd1306.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define I2C configuration parameters
#define I2C_MASTER_SCL_IO 37        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 38        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1    /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

// Declare a handle for the SSD1306 device
static ssd1306_handle_t ssd1306_dev = NULL;

// Main application function
void app_main(void) {
    int changeString = 0;

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

    while(1) {
        changeString++;

        // Refresh the display and clear the screen
        ssd1306_refresh_gram(ssd1306_dev);
        ssd1306_clear_screen(ssd1306_dev, 0x00);

        // Display a string on the screen
        char data_str[30] = {0};
        sprintf(data_str, "Lekker gek %d", changeString); 
        ssd1306_draw_string(ssd1306_dev, 0, 0, (const uint8_t *)data_str, 12, 1);
        sprintf(data_str, "gearbox %d", changeString);
        ssd1306_draw_string(ssd1306_dev, 0, 14, (const uint8_t *)data_str, 12, 1);

        // Refresh the display again
        ssd1306_refresh_gram(ssd1306_dev);
        vTaskDelay(500);
    }
}