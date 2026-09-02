#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"

#define BUTTON_PIN GPIO_NUM_14

void setup_pushbutton();
int pushbutton_is_pressed();
void pushbutton_waitpress();
void wait_system_start();

#endif
