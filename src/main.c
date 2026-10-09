// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// pdMS_TO_TICKS
#include "freertos/FreeRTOS.h"

// vTaskDelay
#include "freertos/task.h"

// L298N
#define IN1 GPIO_NUM_32
#define IN2 GPIO_NUM_33

void setup() {
    gpio_reset_pin(IN1);
    gpio_reset_pin(IN2);
    gpio_set_direction(IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN2, GPIO_MODE_OUTPUT);
}

void loop() {
    // 1. Forward for 1 second
    gpio_set_level(IN1, 1);
    gpio_set_level(IN2, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 2. Reverse for 1 second
    gpio_set_level(IN1, 0);
    gpio_set_level(IN2, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 3. Stop for 1 second
    gpio_set_level(IN1, 0);
    gpio_set_level(IN2, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void app_main() {
    setup();
    while (true) {
        loop();
    }
}