#ifndef LDR_H
#define LDR_H

// gpio_num_t
#include "hal/gpio_types.h"

typedef struct {
  gpio_num_t pin;
} ldr_struct;

ldr_struct ldr_create(const gpio_num_t pin);
void ldr_setup(ldr_struct *ldr);
int ldr_read(ldr_struct *ldr);

#endif
