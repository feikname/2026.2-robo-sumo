// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// pdMS_TO_TICKS
#include "freertos/FreeRTOS.h"

// vTaskDelay
#include "freertos/task.h"

// printf
#include "stdio.h"

// random
#include "esp_random.h"

// system components
#include "button.h"
#include "distancia.h"
#include "hcsr04.h"
#include "ldr.h"
#include "motor.h"

void setup();
void loop();
void app_main();

hcsr04_struct hcsr04;
motor_set motors;
ldr_struct ldr;

// HC-SR04 (distance)
#define TRIG GPIO_NUM_23
#define ECHO GPIO_NUM_22

// H-Bridge (Motor 1) [missing speed control]
#define IN1 GPIO_NUM_33
#define IN2 GPIO_NUM_32

// H-Bridge (Motor 2) [missing speed control]
#define IN3 GPIO_NUM_26
#define IN4 GPIO_NUM_27

// LDR (arena edge)
#define LDR GPIO_NUM_21

void setup() {
  // Distance detection subsystem
  hcsr04 = hcsr04_create(TRIG, GPIO_NUM_22, 25000);
  hcsr04_setup(&hcsr04);

  // // LDR setup
  // ldr = ldr_create(LDR);
  // ldr_setup(&ldr);

  // Activation subsystem
  // setup_pushbutton();
  // wait_system_start();

  // (Simulated) Motor/movement subsystem
  motors = motors_create(IN1, IN2, IN3, IN4);
  motors_setup(&motors);

  // Default strategy: go forward
  // set_movement(&motors, MOVE_FORWARD);
}

void loop() {
  int distance = hcsr04_read_cm(&hcsr04);
  // bool border_found = ldr_read(&ldr);

  ObstacleDistance od = to_dist_range(distance);

  printf("Distância:%d\n", hcsr04_read_cm(&hcsr04));
  vTaskDelay(pdMS_TO_TICKS(1000));

  // do_movement_for(&motors, MOVE_FORWARD, 1000);
  // do_movement_for(&motors, MOVE_STATIONARY, 1000);
  // do_movement_for(&motors, MOVE_BACKWARD, 1000);
  // do_movement_for(&motors, MOVE_STATIONARY, 1000);
}

void app_main() {
  setup();
  while (true) {
    loop();
  }
}
