#ifndef LDR_H

#define LDR_H

// gpio_num_t, GPIO_NUM_\d+
#include "hal/gpio_types.h"

#define LDR_FRENTE GPIO_NUM_25
#define LDR_TRAS GPIO_NUM_4

bool border_found(gpio_num_t pin);
void ldr_setup(gpio_num_t pin);

#endif
