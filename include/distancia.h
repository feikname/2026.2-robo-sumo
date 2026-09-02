#ifndef DISTANCIA_H
#define DISTANCIA_H

#include "driver/gpio.h"

#define LED_VERMELHO_10 GPIO_NUM_19
#define LED_AZUL_30 GPIO_NUM_18
#define LED_AMARELO_60 GPIO_NUM_5
#define LED_AZUL_90 GPIO_NUM_17

typedef enum { VERY_NEAR, NEAR, MEDIUM, FAR } DistanceRange;

DistanceRange getDistanceRange(int distance);
void setup_leds_distancia();
void reset_distance_leds();

#endif
