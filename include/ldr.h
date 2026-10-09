#ifndef LDR_H

#define LDR_H

#include "hal/gpio_types.h"

bool border_found(gpio_num_t pin);
void ldr_setup(gpio_num_t pin);

#endif