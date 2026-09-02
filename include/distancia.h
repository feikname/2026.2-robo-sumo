#ifndef DISTANCIA_H
#define DISTANCIA_H

#include "driver/gpio.h"

#define LED_VERY_NEAR GPIO_NUM_19
#define LED_NEAR GPIO_NUM_18
#define LED_MEDIUM GPIO_NUM_5
#define LED_FAR GPIO_NUM_17

typedef enum { OBSTACLE_IS_VERY_CLOSE, OBSTACLE_IS_CLOSE, OBSTACLE_IN_MEDIUM_DISTANCE, OBSTACLE_IS_FAR, OBSTACLE_NOT_PRESENT } ObstacleDistance;

ObstacleDistance to_dist_range(const int distance);
void setup_distance_leds();
void reset_distance_leds();
void distance_leds_update(ObstacleDistance od);

#endif
