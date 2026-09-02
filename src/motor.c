#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "motor.h"

void setup_motor() {
  gpio_set_direction(LED_DIREITA, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_ESQUERDA, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_TRAS, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_FRENTE, GPIO_MODE_OUTPUT);
}

void ir_duracao(Direction direcao, int duracao) {
  switch (direcao) {
  case FRONT:
    gpio_set_level(LED_FRENTE, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_FRENTE, 0);
    break;

  case SUPERIOR_LEFT:
    gpio_set_level(LED_FRENTE, 1);
    gpio_set_level(LED_ESQUERDA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_FRENTE, 0);
    gpio_set_level(LED_ESQUERDA, 0);
    break;

  case LEFT:
    gpio_set_level(LED_ESQUERDA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_ESQUERDA, 0);
    break;

  case INFERIOR_LEFT:
    gpio_set_level(LED_TRAS, 1);
    gpio_set_level(LED_ESQUERDA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_TRAS, 0);
    gpio_set_level(LED_ESQUERDA, 0);
    break;

  case BACK:
    gpio_set_level(LED_TRAS, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_TRAS, 0);
    break;

  case INFERIOR_RIGHT:
    gpio_set_level(LED_TRAS, 1);
    gpio_set_level(LED_DIREITA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_TRAS, 0);
    gpio_set_level(LED_DIREITA, 0);
    break;

  case RIGHT:
    gpio_set_level(LED_DIREITA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_DIREITA, 0);
    break;

  case SUPERIOR_RIGHT:
    gpio_set_level(LED_FRENTE, 1);
    gpio_set_level(LED_DIREITA, 1);
    vTaskDelay(pdMS_TO_TICKS(duracao));
    gpio_set_level(LED_FRENTE, 0);
    gpio_set_level(LED_DIREITA, 0);
    break;
  }
}

void exec_manobra() {
  ir_duracao(BACK, 1000);
  vTaskDelay(pdMS_TO_TICKS(500));
  ir_duracao(LEFT, 1000);
}

void initial_start(Strategy strat) {
  switch (strat) {
  case Default:
    ir_duracao(BACK, 200);
    break;

  case Aggressive:
    ir_duracao(FRONT, 200);
    ir_duracao(RIGHT, 100);
    // acelera em direção ao centro
    ir_duracao(SUPERIOR_LEFT, 200);
    ir_duracao(FRONT, 200);
    break;

  case Defensive:
    break;

  case Random:
    break;
  }
}
