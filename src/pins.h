// gpio_num_t
#include "hal/gpio_types.h"

// HC-SR04 (distance)
#define TRIG GPIO_NUM_18
#define ECHO GPIO_NUM_35

// H-Bridge (Motor 1 - Right)
#define R_IN1 GPIO_NUM_27
#define R_IN2 GPIO_NUM_14
#define R_PWM GPIO_NUM_26

// H-Bridge (Motor 2 - Left)
#define L_IN1 GPIO_NUM_22
#define L_IN2 GPIO_NUM_21
#define L_PWM GPIO_NUM_23

// LDR (arena edge)
#define LDR GPIO_NUM_34
