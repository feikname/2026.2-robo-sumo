#ifndef LDR_H

#define LDR_H

#include "driver/gpio.h"

bool border_found(gpio_num_t pin);
void ldr_setup(gpio_num_t pin);

#endif