// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// prototype definitions
#include "distancia.h"

ObstacleDistance get_dist_range1(const int distance) {
  if (distance <= 10) {
    return OBSTACLE_IS_VERY_CLOSE;
  } else if (distance <= 30) {
    return OBSTACLE_IS_CLOSE;
  } else if (distance <= 60) {
    return OBSTACLE_IN_MEDIUM_DISTANCE;
  } else if (distance <= 90) {
    return OBSTACLE_IS_FAR;
  } else {
    return OBSTACLE_NOT_PRESENT;
  }
}

ObstacleDistance get_dist_range2(const int distance) {
  // easier to test inside wokwi because of the UI slider from 1-400
  if (distance <= 100) {
    return OBSTACLE_IS_VERY_CLOSE;
  } else if (distance <= 200) {
    return OBSTACLE_IS_CLOSE;
  } else if (distance <= 300) {
    return OBSTACLE_IN_MEDIUM_DISTANCE;
  } else if (distance <= 380) {
    return OBSTACLE_IS_FAR;
  } else {
    return OBSTACLE_NOT_PRESENT;
  }
}

ObstacleDistance to_dist_range(const int distance) {
  return get_dist_range1(distance);
}

void reset_distance_leds() {
  gpio_set_level(LED_FAR, 0);
  gpio_set_level(LED_MEDIUM, 0);
  gpio_set_level(LED_NEAR, 0);
  gpio_set_level(LED_VERY_NEAR, 0);
}

void setup_distance_leds() {
  gpio_set_direction(LED_VERY_NEAR, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_NEAR, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_MEDIUM, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_FAR, GPIO_MODE_OUTPUT);
}

void distance_leds_update(ObstacleDistance dr) {
  gpio_set_level(LED_VERY_NEAR, dr == OBSTACLE_IS_VERY_CLOSE);
  gpio_set_level(LED_NEAR, dr == OBSTACLE_IS_CLOSE);
  gpio_set_level(LED_MEDIUM, dr == OBSTACLE_IN_MEDIUM_DISTANCE);
  gpio_set_level(LED_FAR, dr == OBSTACLE_IS_FAR);
}
