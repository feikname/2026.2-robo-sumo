#include "ldr.h"

#include "driver/gpio.h"

void ldr_setup(gpio_num_t pin) {
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
}

bool border_found(gpio_num_t pin) {
    return gpio_get_level(pin);
}
