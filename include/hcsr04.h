#ifndef HCSR04_H
#define HCSR04_H

#include "driver/gpio.h"

#define TRIG_GPIO 23
#define ECHO_GPIO 22

void setup_hc_sr04(int trig_pin, int echo_pin);
int ler_HC_SR04(int trig_pin, int echo_pin);

#endif
