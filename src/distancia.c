// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// prototype definitions
#include "distancia.h"

DistanceRange getDistanceRange(int distance) {
  if (distance <= 10) {
    return VERY_NEAR;
  }

  if (distance > 10 && distance <= 30) {
    return NEAR;
  }

  if (distance > 30 && distance <= 60) {
    return MEDIUM;
  }

  return FAR;
}

void setup_leds_distancia() {
  gpio_set_direction(LED_VERMELHO_10, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_AZUL_30, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_AMARELO_60, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_AZUL_90, GPIO_MODE_OUTPUT);
}

void reset_distance_leds() {
  gpio_set_level(LED_AZUL_90, 0);
  gpio_set_level(LED_AMARELO_60, 0);
  gpio_set_level(LED_AZUL_30, 0);
  gpio_set_level(LED_VERMELHO_10, 0);
}
