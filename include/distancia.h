#ifndef DISTANCIA_H
#define DISTANCIA_H

#include "driver/gpio.h"

#define LED_VERY_NEAR GPIO_NUM_19
#define LED_NEAR GPIO_NUM_18
#define LED_MEDIUM GPIO_NUM_5
#define LED_FAR GPIO_NUM_17

typedef enum { DISTANCE_VERY_NEAR, DISTANCE_NEAR, DISTANCE_MEDIUM, DISTANCE_FAR, DISTANCE_VERY_FAR } DistanceRange;

DistanceRange to_dist_range(const int distance);
void setup_distance_leds();
void reset_distance_leds();
void distance_leds_update(DistanceRange dr);

#endif
