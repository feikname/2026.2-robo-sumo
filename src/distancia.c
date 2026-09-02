// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// prototype definitions
#include "distancia.h"

DistanceRange get_dist_range1(const int distance) {
  if (distance <= 10) {
    return DISTANCE_VERY_NEAR;
  } else if (distance <= 30) {
    return DISTANCE_NEAR;
  } else if (distance <= 60) {
    return DISTANCE_MEDIUM;
  } else if (distance <= 90) {
    return DISTANCE_FAR;
  } else {
    return DISTANCE_VERY_FAR;
  }
}

DistanceRange get_dist_range2(const int distance) {
  // easier to test inside wokwi because of the UI slider from 1-400
  if (distance <= 100) {
    return DISTANCE_VERY_NEAR;
  } else if (distance <= 200) {
    return DISTANCE_NEAR;
  } else if (distance <= 300) {
    return DISTANCE_MEDIUM;
  } else if (distance <= 380) {
    return DISTANCE_FAR;
  } else {
    return DISTANCE_VERY_FAR;
  }
}

DistanceRange to_dist_range(const int distance) {
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

void distance_leds_update(DistanceRange dr) {
  gpio_set_level(LED_VERY_NEAR, dr == DISTANCE_VERY_NEAR);
  gpio_set_level(LED_NEAR, dr == DISTANCE_NEAR);
  gpio_set_level(LED_MEDIUM, dr == DISTANCE_MEDIUM);
  gpio_set_level(LED_FAR, dr == DISTANCE_FAR);
}
