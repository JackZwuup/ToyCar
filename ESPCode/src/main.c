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
#define FREQ_HZ = 5000
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
    // For gpio_set_level set high or low.
    gpio_set_direction(LED_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_4, GPIO_MODE_OUTPUT);

    gpio_set_direction(COUNTERWISE, GPIO_MODE_OUTPUT);

    gpio_set_direction(BUZZ, GPIO_MODE_OUTPUT);

    // gpio_get_level iternal resistor.
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

/// @brief Determine the pwm functions.
///
/// This function determines the ledc timer config and ledc channel_config.
///
/// @param
///
/// @return none
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

/// @brief Determine the adc functions.
///
/// This function determines the adc1 config width and adc1 config attenuation
///
/// @param
///
/// @return none
void adc_init(){
  // We start with the configuration of the resolution.
  adc1_config_width(ADC_WIDTH_BIT_DEFAULT);  

  // Then you set the range.

  // For LDR
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11);

  // For potmeter.
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);

  // For Hall effect sensor
  adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN_DB_11);
}

/// @brief Determine the lcd display functions.
///
/// This function determines the i2c configuration.
///
/// @param
///
/// @return none
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

/// @brief Everything that need to be displayed in one function
///
/// This function is called to display certain words dependent on the context.
///
/// @param dipCur1to7 
/// @param carState
/// @param otherCarState
/// @param speed
///
/// @return none

void whatNeedToBeDisplayed(int dipCur1, int dipCur2, int dipCur3, int dipCur4, int dipCur5, int dipCur6, int dipCur7, int carState, int otherCarState, int speed, bool carKey) {
        // Refresh the display and clear the screen
        ssd1306_refresh_gram(ssd1306_dev);
        ssd1306_clear_screen(ssd1306_dev, 0x00);

        // Display a string on the screen
        char data_str[30] = {0};
        sprintf(data_str, "gearbox %d %d %d %d %d %d %d", dipCur1, dipCur2, dipCur3, dipCur4, dipCur5, dipCur6, dipCur7);
        ssd1306_draw_string(ssd1306_dev, 0, 0, (const uint8_t *)data_str, 12, 1);

        // Choices between different states.
        if (carState == 0 || otherCarState == 0) {
            sprintf(data_str, "OffState");
            ssd1306_draw_string(ssd1306_dev, 0, 14, (const uint8_t *)data_str, 12, 1);
        }
        else if (carState == -1) {
            sprintf(data_str, "ERROR");
            ssd1306_draw_string(ssd1306_dev, 0, 14, (const uint8_t *)data_str, 12, 1);
        }
        else if (carState == 1) {
            sprintf(data_str, "OnState");
            ssd1306_draw_string(ssd1306_dev, 0, 14, (const uint8_t *)data_str, 12, 1);
        }
        else if (carState == TWO) {
            sprintf(data_str, "ParkState");
            ssd1306_draw_string(ssd1306_dev, 0, 14, (const uint8_t *)data_str, 12, 1);
        }
        else if (carState == 3) {
            sprintf(data_str, "DriveState");
            ssd1306_draw_string(ssd1306_dev, 0, 14, (const uint8_t *)data_str, 12, 1);
            sprintf(data_str, "SPEED (2000 max): %d", speed - 2095);
            ssd1306_draw_string(ssd1306_dev, 0, 28, (const uint8_t *)data_str, 12, 1);

        }
        else if (carState == 4) {
            sprintf(data_str, "BackState");  
            ssd1306_draw_string(ssd1306_dev, 0, 14, (const uint8_t *)data_str, 12, 1); 
        }  

        // Choices between other states.
        if (otherCarState == -1) {
            sprintf(data_str, "ERROR");
            ssd1306_draw_string(ssd1306_dev, 64, 14, (const uint8_t *)data_str, 12, 1);
        }      

        else if (otherCarState == 1) {
            sprintf(data_str, "RightState");
            ssd1306_draw_string(ssd1306_dev, 64, 14, (const uint8_t *)data_str, 12, 1);
        }   
        else if (otherCarState == 2) {
            sprintf(data_str, "LeftState");
            ssd1306_draw_string(ssd1306_dev, 64, 14, (const uint8_t *)data_str, 12, 1);
        }          
        else if (otherCarState == 3) {
            sprintf(data_str, "NoneState");
            ssd1306_draw_string(ssd1306_dev, 64, 14, (const uint8_t *)data_str, 12, 1);
        } 

        if (!carKey) {
            sprintf(data_str, "Key is absent");
            ssd1306_draw_string(ssd1306_dev, 0, 42, (const uint8_t *)data_str, 12, 1); 
           
        }
        else {
            sprintf(data_str, "Key is present");
            ssd1306_draw_string(ssd1306_dev, 0, 56, (const uint8_t *)data_str, 12, 1); 
  
        }
        


        // Refresh the display again
        ssd1306_refresh_gram(ssd1306_dev);
}

/// @brief Interrupt handlers for the honk button.
///
/// This function is called when the honk button is pressed,
/// and it sets the global variable `button_buzz` to true.
///
/// @param arg: Unused argument (can be NULL).
///
/// @return none
static void breakPedalInterrupt(void* arg) {
    button_pressed = true;
}

static void honkInterrupt(void* arg) {
    button_buzz = true;
}

/// @brief Setup function for configuring the honk or break button.
///
/// Configures the honk or break button as an input, sets interrupt type
/// to trigger on the falling edge (1 to 0 transition), and installs
/// the interrupt handler.
///
/// @return none
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

/// @brief Process function for the honk or break button actions.
/// 
/// Makes sure that the buzzer is beeping or the car is breaking,
/// when the button is pressed thans to the interrupt.
/// 
/// @return none
void processBuzzInterrupt() {
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
}


void processBreakInterrupt() {
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
}


void app_main() {

    printf("Begin or something");

    // This determines whether the car is on or off thanks to the readings of a key.
    int onOrOff = 0;
    // Read ldr's old current
    int ldr_cur_old = 0;

    // Dip Switch currency's (high or low).
    int dipCur1 = 0;
    int dipCur2 = 0;
    int dipCur3 = 0;
    int dipCur4 = 0;
    int dipCur5 = 0;
    int dipCur6 = 0;
    int dipCur7 = 0;

    int brightness = 0;
    int carState = 0;
    int otherCarState = 0;
    int parkBlinkCount = 0;
    int analog = 0; 
    int isKeyPresent= 0;

    int ldr_value = 0;
    int pot_value = 0;
    
    // Setup certain interrupts.
    setupBreakPedal();
    setupHonkBuzz();
    // Get adc info.
    adc_init();
    // Get gpio info.
    init_gpio();
    // Get pwm info.
    init_pwm();
    // Get lcd info.
    lcd_init();

    // Make sure the motor is off.
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

    gpio_set_level(COUNTERWISE, 1);

while(1) {
    // get ldr adc value.
    ldr_value = adc1_get_raw(ADC1_CHANNEL_1);
    pot_value = adc1_get_raw(ADC1_CHANNEL_3);
    
    // get dipCur.
    dipCur1 = gpio_get_level (SWITCH_1OFF);
    dipCur2 = gpio_get_level (SWITCH_2PARK);
    dipCur3 = gpio_get_level (SWITCH_3DRIVE);
    dipCur4 = gpio_get_level (SWITCH_4BACK);
    dipCur5 = gpio_get_level (SWITCH_5LEFT);
    dipCur6 = gpio_get_level (SWITCH_6RIGHT);
    dipCur7 = gpio_get_level (SWITCH_7NONE);

    printf("\nswitch state: %d %d %d %d %d %d %d\n", dipCur1, dipCur2, dipCur3, dipCur4, dipCur5, dipCur6, dipCur7);
    // Display hall effect sensor value.
    printf("Hall effect adc value: %d\n", adc1_get_raw(ADC1_CHANNEL_2)); 
    printf("Button state = %d\n", gpio_get_level(BREAK_PEDAL));
    printf("carState = %d\n", carState);
    printf("LDR value: %d\n", ldr_value);
    printf("pot value: %d\n", pot_value);

    // Make sure the buttons are detected and break or buzz.
    processBreakInterrupt();
    processBuzzInterrupt();

    // Display things in the display.
    whatNeedToBeDisplayed(dipCur1, dipCur2, dipCur3, dipCur4, dipCur5, dipCur6, dipCur7, carState, otherCarState, brightness, isKeyPresent);

    // Get calculated adc value for pwm.
    analog = getAdcForPwm();

    // Retrieve carkey value.
    isKeyPresent = carKeyValue(onOrOff);

    // Get one of the 6 states the car could be in.
    carState = valueForSwitch();

    // Car is only on when the key is present.
    if (!isKeyPresent || parkBlinkCount == 10) {
        carState = 0;
    }

    switch(carState) {
        // Turn everything off what could be turned of.
        case 0:
        printf("Off state\n");
        gpio_set_level(LED_1, 0);
        gpio_set_level(LED_2, 0);
        gpio_set_level(LED_4, 0);
        gpio_set_level(LED_3, 0);  

        gpio_set_level(COUNTERWISE, 1);

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
        break;

        // Just an indication that it's on.
        case 1:
        printf("On state\n");
        break;

        // Park the car, after ten blinks the car is parked.
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

            gpio_set_level(COUNTERWISE, 1);

            ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
            // Update duty to apply the new value
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
        break;

        // Drive the car, wheels CW. Adjusted speed thanks to the potmeter. If the car drives it can park.
        case 3:
        printf("Drive state\n");

        parkBlinkCount = 0;
        brightness = (analog * 40.95);
        // Set duty to 50%
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, brightness));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

        gpio_set_level(COUNTERWISE, 1);

        break;

        // Wheels turn CCW, if the car drives it can park.
        case 4:
        printf("Back state\n");

        parkBlinkCount = 0;

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

        gpio_set_level(COUNTERWISE, 0);

        break;
    }
    
    // Those are other states independent of the states above.
    otherCarState = otherValueForSwitch();
    if (!isKeyPresent) {
        otherCarState = 0;
    }
    switch(otherCarState) {

        // Turn everything off.
        case 0:
        printf("Off state\n");
        parkBlinkCount = 0;
        gpio_set_level(LED_1, 0);
        gpio_set_level(LED_2, 0); 
        gpio_set_level(COUNTERWISE, 1);

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 400));
        // Update duty to apply the new value
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
        gpio_set_level(LED_3, 0);
        gpio_set_level(LED_4, 0); 
        break;

        // Blink with a certain frequency the right led or left led.
        case 1:
        printf("Left state\n");     
        gpio_set_level(LED_4, 1);
        vTaskDelay(20);
        gpio_set_level(LED_4, 0);
        vTaskDelay(20);
        break;

        case 2:
        printf("Right state\n");     
        gpio_set_level(LED_3, 1);
        vTaskDelay(20);
        gpio_set_level(LED_3, 0);
        vTaskDelay(20);
        break;

        // Blink no leds
        case 3:
        printf("None state\n");     
        gpio_set_level(LED_3, 0);
        gpio_set_level(LED_4, 0);
        break;
    }

    // If it's -1 it's error.
    if (valueForSwitch() == -1 || otherValueForSwitch() == -1) {
        printf("Error");
    }

    // This determines whether the threshold has been passed and

    // determines whether the key is present or not.
    if (ldr_value >= LDR_THRESHOLD && ldr_cur_old == 0) {
        // To add one to to onorof the function: bool carKeyValue(int onOrOff) can determine whether the key is absent or not (the car is on or off).
        onOrOff++;
    }
    // if ldr value is higher than ldr threshold ldr cur old = 1 if not it's 0.
    // Update the old currency recording to that.
    ldr_cur_old = (ldr_value > LDR_THRESHOLD) ? 1 : 0;

    vTaskDelay(100);
}
}

/// @brief This function determines whether the carkey is absent or not.
/// 
/// If it's an even number the car key is absent if it's a uneven number the car key is present.
/// @param onOrOff this number increases everytime the threshold is passed.
/// 
/// @return true or false. When key is present or not.

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

/// @brief determine a value for the dipswitch.
/// 
/// I have certain states with a certain bit assigned with bitshift. 
/// Thanks to simple math calculations I can assign the corresponding states.
/// 
/// @return number of certain state, so it can be implemented in the switch case.
int valueForSwitch() {
    int switchOff = gpio_get_level (SWITCH_1OFF);
    int switchPark = gpio_get_level (SWITCH_2PARK) << 1;
    int switchDrive = gpio_get_level(SWITCH_3DRIVE) << TWO;
    int switchBack = gpio_get_level (SWITCH_4BACK) << 3;

    // These dependent states are dependent on each other.
    int dependentTogether = switchOff + switchPark + switchDrive + switchBack;

        // If switch_1off = 0, all states will be even numbers, that's how you know offstate is applied.
        if (dependentTogether % TWO == 0) 
        {
        // printf("Off state\n");
            return 0;

        }
        // If switch_1off is on, the dependentTogether states will be one and nothing else is on.
        else if (dependentTogether == 1)
        {
            // printf("On state\n");
            return 1;
        }

        // If only switch_1off and switch_2park are on, parkstate is applied.
        else if (dependentTogether == 3)
        {
            // printf("Park state\n");        
            return 2;
        }

        // If only switch_1off and switch_3drive are on, drivestate is applied.
        else if (dependentTogether == 5) {
            // printf("drive state\n");        
            return 3;
        }

        // If only switch_1off and switch_4back are on, backstate is applied.
        else if (dependentTogether == 9)
        {
            // printf("Back state\n");        
            return 4;
        }

    // With the rest of the states more than one states are on, this is not possible and will result in a error.
    return -1;
}

/// @brief determine a value for the dipswitch.
/// 
/// I have certain states with a certain bit assigned with bitshift. 
/// Thanks to simple math calculations I can assign the corresponding states.
/// 
/// @return number of certain state, so it can be implemented in the switch case.
int otherValueForSwitch() {
    // I have assigned the bits in a certain understandable bit sequence.
    int switchOff = gpio_get_level (SWITCH_1OFF);
    int switchLeft = gpio_get_level (SWITCH_5LEFT) << 4;
    int switchRight = gpio_get_level(SWITCH_6RIGHT) << 5;
    int switchNone= gpio_get_level (SWITCH_7NONE) << 6;

    int allRestTogether = switchOff + switchLeft + switchRight + switchNone;

        // If switch_1off = 0, all states will be even numbers, that's how you know offstate is applied.
        if (allRestTogether % TWO == 0) 
        {       
        // printf("Off state\n");
            return 0;
        }

        // If switch_1off is on, the allresttogether states will be one and nothing else is on.
        // I actually don't need this and I find out this is quite redundant, but if it'snot adressed it's counted like an error.
        else if (allRestTogether == 1) 
        {
        // printf("On state\n");
            return 123456789;

        }

        // If only switch_1off and switch_ 5left state are on, leftstate is applied.
        else if (allRestTogether == 17)
        {
            // printf("Left state\n");        
            return 1;
        }
        // If only switch_1off and switch_6right state are on, rightstate is applied.
        else if (allRestTogether == 33) {
            // printf("Right state\n");        
            return 2;
        }
        // If only switch_1off and switch_7none state are on, nonestate is applied and no right or left leds are on.
        else if (allRestTogether == 65)
        {
            // printf("None state\n");        
            return 3;
        }
    return -1;
}

/// @brief determine a value for a modulation calculation for adjusting the of the motor speed
/// 
/// I have attemped to calculate a certain value for the speed of the motor,
/// Thanks to math calculations if can therefore modify the analog value.
/// 
/// @return (anaIn * 100) / anaMax;, because multiplying a number close to 0 doesn't work.
int getAdcForPwm() {

// Analog input value
  int anaIn = adc1_get_raw(ADC1_CHANNEL_3);
  int anaMax = 4095;

//   printf("ADC value : %d\n", anaIn);
  return (anaIn * 100) / anaMax;

// Calculate input percentage 
}