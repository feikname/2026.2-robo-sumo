#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdio.h"

#include "button.h"
#include "hcsr04.h"

#include "distancia.h"
#include "motor.h"

Strategy strat = Default;

void setup();
void loop();
void app_main();

void distance_detection_loop(Strategy strat);
void exec_manobra();

bool systemActive = false;

void distance_detection_loop(Strategy strat) {
  int distancia = ler_HC_SR04(TRIG_GPIO, ECHO_GPIO);
  printf("Distância: %d cm \n", distancia);

  reset_distance_leds();

  switch (getDistanceRange(distancia)) {
  case VERY_NEAR:
    gpio_set_level(LED_VERMELHO_10, 1);

    gpio_set_level(LED_FRENTE, 0);
    vTaskDelay(pdMS_TO_TICKS(500));

    printf("exec_manobra\n");
    exec_manobra();
    break;

  case NEAR:
    gpio_set_level(LED_FRENTE, 1);
    gpio_set_level(LED_AZUL_30, 1);
    break;

  case MEDIUM:
    gpio_set_level(LED_FRENTE, 1);
    gpio_set_level(LED_AMARELO_60, 1);
    break;

  case FAR:
    gpio_set_level(LED_FRENTE, 1);
    gpio_set_level(LED_AZUL_90, 1);
    break;
  }
}

void setup() {
  setup_pushbutton();
  setup_hc_sr04(TRIG_GPIO, ECHO_GPIO);
  setup_leds_distancia();
  setup_motor();

  wait_system_start(&systemActive);

  initial_start(strat);
}

void loop() {
  distance_detection_loop(strat);

  vTaskDelay(pdMS_TO_TICKS(250));
}

void app_main() {
  setup();
  while (true) {
    loop();
  }
}
