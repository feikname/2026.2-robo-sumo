// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// pdMS_TO_TICKS
#include "freertos/FreeRTOS.h"

// vTaskDelay
#include "freertos/task.h"

// prototype definitions
#include "motor.h"

void motors_setup() {
  gpio_set_direction(LED_DIREITA, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_ESQUERDA, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_TRAS, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_FRENTE, GPIO_MODE_OUTPUT);
}

void do_movement(Direction direction, int duracao) {
  switch (direction) {
  case DIRECTION_FRONT:
    gpio_set_level(LED_FRENTE, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_FRENTE, 0);
    break;

  case DIRECTION_FRONT_LEFT:
    gpio_set_level(LED_FRENTE, 1);
    gpio_set_level(LED_ESQUERDA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_FRENTE, 0);
    gpio_set_level(LED_ESQUERDA, 0);
    break;

  case DIRECTION_LEFT:
    gpio_set_level(LED_ESQUERDA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_ESQUERDA, 0);
    break;

  case DIRECTION_REAR_LEFT:
    gpio_set_level(LED_TRAS, 1);
    gpio_set_level(LED_ESQUERDA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_TRAS, 0);
    gpio_set_level(LED_ESQUERDA, 0);
    break;

  case DIRECTION_REAR:
    gpio_set_level(LED_TRAS, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_TRAS, 0);
    break;

  case DIRECTION_REAR_RIGHT:
    gpio_set_level(LED_TRAS, 1);
    gpio_set_level(LED_DIREITA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_TRAS, 0);
    gpio_set_level(LED_DIREITA, 0);
    break;

  case DIRECTION_RIGHT:
    gpio_set_level(LED_DIREITA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_DIREITA, 0);
    break;

  case DIRECTION_FRONT_RIGHT:
    gpio_set_level(LED_FRENTE, 1);
    gpio_set_level(LED_DIREITA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_FRENTE, 0);
    gpio_set_level(LED_DIREITA, 0);
    break;
  }
}

void do_maneuver(Maneuver m) {
  if (m == MANEUEVER_EVASION) {
    do_evasion_maneuveur();
  }
}

void do_evasion_maneuveur() {
  do_movement(DIRECTION_REAR, 500);

  // tanto faz se direita ou esquerda
  do_movement(DIRECTION_LEFT, 1000);
}
