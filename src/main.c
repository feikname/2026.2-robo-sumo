// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// LEDC PWM
#include "driver/ledc.h"

// pdMS_TO_TICKS
#include "freertos/FreeRTOS.h"

// vTaskDelay
#include "freertos/task.h"

// printf
#include "stdio.h"

// system components
#include "distancia.h"
#include "hcsr04.h"
#include "ldr.h"
#include "motor.h"

// pin definitions
#include "pins.h"

void setup();
void loop();
void app_main();

hcsr04_struct hcsr04;
motor_set motors;
ldr_struct ldr;

// General settings
// Please do not set loop delay below 64ms, to reduce interference from HC-SR04 previous echoes
#define LOOP_DELAY 64
#define STARTUP_DELAY 5000

void setup() {
  // pode valer a pena diminuir o timeout aqui de 25ms para algo menor
  // atualmente a distancia max é d = (0.025 * 343)/2 ~= 4.3m
  hcsr04 = hcsr04_create(TRIG, ECHO, 25000);
  hcsr04_setup(&hcsr04);

  ldr = ldr_create(LDR);
  ldr_setup(&ldr);

  motors = motors_create(L_IN1, L_IN2, L_PWM, R_IN1, R_IN2, R_PWM);
  motors_setup(&motors);

  // Wait 5s before start
  vTaskDelay(pdMS_TO_TICKS(STARTUP_DELAY));
}

void loop() {
  int distance = hcsr04_read_cm(&hcsr04);
  bool border_found = ldr_read(&ldr);

  printf("Distância: %d\n", distance);
  printf("LDR: %d\n", border_found);

  ObstacleDistance od = to_dist_range(distance);

  switch (od) {
  case OBSTACLE_IS_VERY_CLOSE:
  case OBSTACLE_IS_CLOSE:
    set_movement(&motors, MOVE_FORWARD);
    break;

  default:
    set_movement(&motors, MOVE_SPIN_CLOCKWISE);
    break;
  }

  printf("============\n");
}

void app_main() {
  setup();

  while (true) {
    loop();
    vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY));
  }
}
