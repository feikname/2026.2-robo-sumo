// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// pdMS_TO_TICKS
#include "freertos/FreeRTOS.h"

// vTaskDelay
#include "freertos/task.h"

// printf
#include "stdio.h"

// system components
#include "button.h"
#include "hcsr04.h"
#include "distancia.h"
#include "motor.h"

Strategy strat = STRATEGY_DEFAULT;

void setup();
void loop();
void app_main();

hcsr04_struct hcsr04;

void setup() {
  // Distance detection subsystem
  hcsr04 = hcsr04_create(GPIO_NUM_23, GPIO_NUM_22, 25000);
  hcsr04_setup(&hcsr04);
  setup_distance_leds();

  // (Simulated) Motor/movement subsystem
  motors_setup();

  // TODO: Edge detection subsystem

  // Activation subsystem
  setup_pushbutton();
  wait_system_start();
}

void loop() {
  int distance = hcsr04_read_cm(&hcsr04);
  printf("Distância: %d cm \n", distance);

  DistanceRange dr = to_dist_range(distance);
  distance_leds_update(dr);

  if (dr == DISTANCE_VERY_NEAR) {
    gpio_set_level(LED_FRENTE, 0);
    vTaskDelay(pdMS_TO_TICKS(500));

    printf("manobra_desvio\n");
    do_evasion_maneuveur();
  } else {
    gpio_set_level(LED_FRENTE, 1);
  }

  vTaskDelay(pdMS_TO_TICKS(250));
}

void app_main() {
  setup();
  while (true) {
    loop();
  }
}
