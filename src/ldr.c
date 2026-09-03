#include "ldr.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void ldr_setup() {
    gpio_set_direction(FRONT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(FRONT, GPIO_PULLUP_ONLY);
    gpio_set_direction(BACK, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BACK, GPIO_PULLUP_ONLY);
}

