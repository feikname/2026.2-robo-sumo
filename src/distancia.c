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

ObstacleDistance to_dist_range(const int distance) {
  return get_dist_range1(distance);
}
