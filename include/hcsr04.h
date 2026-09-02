#ifndef HCSR04_H
#define HCSR04_H

#include "driver/gpio.h"

#define TRIG_GPIO 23
#define ECHO_GPIO 22

void hcsr04_setup(int trig_pin, int echo_pin);
int hcsr04_read_cm(int trig_pin, int echo_pin, int timeout);

#endif
