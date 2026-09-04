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
#include "ldr.h"

#define LDR_FRENTE GPIO_NUM_25
#define LDR_TRAS GPIO_NUM_4

Strategy strat = STRATEGY_DEFAULT;

void setup();
void loop();
void app_main();

hcsr04_struct hcsr04;
motor_set motors;

void setup() {
  // Distance detection subsystem
  hcsr04 = hcsr04_create(GPIO_NUM_23, GPIO_NUM_22, 25000);
  hcsr04_setup(&hcsr04);
  setup_distance_leds();

  // (Simulated) Motor/movement subsystem
  motors = motors_create();
  motors_setup();

  // LDR setup
  ldr_setup(LDR_TRAS);
  ldr_setup(LDR_FRENTE);

  // Activation subsystem
  setup_pushbutton();
  wait_system_start();

  // Default strategy: go forward
  set_motor_movement(DIRECTION_FORWARD, SPEED_FAST, &motors);
}

void loop() {
  int distance = hcsr04_read_cm(&hcsr04);
  printf("Distância: %d cm \n", distance);

  bool ldr_tras = border_found(LDR_TRAS);
  bool ldr_frente = border_found(LDR_FRENTE);
  printf("LDR frente -> %d\nLDR tras -> %d\n", ldr_frente, ldr_tras);

  ObstacleDistance od = to_dist_range(distance);
  distance_leds_update(od);

  if (od == OBSTACLE_IS_VERY_CLOSE) {
    do_maneuver(MANEUEVER_EVASION, &motors);
  } else if ( ldr_tras ) {
    do_maneuver(MANEUEVER_BACK_LDR, &motors);
  } else if ( ldr_frente ) {
    do_maneuver(MANEUEVER_FRONT_LDR, &motors);
  } else {
    set_motor_movement(DIRECTION_FORWARD, SPEED_FAST, &motors);
  }

  vTaskDelay(pdMS_TO_TICKS(250));
}

void app_main() {
  setup();
  while (true) {
    loop();
  }
}
