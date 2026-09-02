#ifndef HCSR04_H
#define HCSR04_H

typedef struct {
  gpio_num_t trigger_pin;
  gpio_num_t echo_pin;
  int timeout_duration;
} hcsr04_struct;

hcsr04_struct hcsr04_create(const gpio_num_t trigger_pin,
                            const gpio_num_t echo_pin,
                            const int timeout_duration);
void hcsr04_setup(hcsr04_struct *hcsr04);
int hcsr04_read_cm(hcsr04_struct *hcsr04);

#endif
