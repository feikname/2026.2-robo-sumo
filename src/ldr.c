#include "ldr.h"

// gpio_set_direction, gpio_set_level, gpio_num_t
#include "driver/gpio.h"

ldr_struct ldr_create(const gpio_num_t pin) {
  ldr_struct ldr = {
      .pin = pin,
  };

  return ldr;
}

void ldr_setup(ldr_struct *ldr) {
  gpio_set_direction(ldr->pin, GPIO_MODE_INPUT);
  gpio_set_pull_mode(ldr->pin, GPIO_PULLUP_ONLY);
}

int ldr_read(ldr_struct *ldr) {
    return gpio_get_level(ldr->pin); 
}
