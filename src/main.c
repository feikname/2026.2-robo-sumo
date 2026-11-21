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
#include "hcsr04.h"
#include "ldr.h"
#include "motor.h"

void setup();
void loop();
void app_main();

hcsr04_struct hcsr04;
ldr_struct ldr;

// HC-SR04 (distance)
#define TRIG GPIO_NUM_18
#define ECHO GPIO_NUM_35

// H-Bridge (Motor 1 - Right)
#define R_IN1 GPIO_NUM_27
#define R_IN2 GPIO_NUM_14
#define R_PWM GPIO_NUM_26

// H-Bridge (Motor 2 - Left)
#define L_IN1 GPIO_NUM_22
#define L_IN2 GPIO_NUM_21
#define L_PWM GPIO_NUM_23

// LDR (arena edge)
#define LDR GPIO_NUM_34

// PWM configuration
#define PWM_FREQ 1000
#define PWM_DUTY_50 512  // 50% duty cycle
#define PWM_DUTY_25 256  // 25% duty cycle

void setup() {
  // Distance detection subsystem
  hcsr04 = hcsr04_create(TRIG, ECHO, 25000);
  hcsr04_setup(&hcsr04);

  // LDR setup
  ldr = ldr_create(LDR);
  ldr_setup(&ldr);

  // Motor GPIO configuration
  gpio_set_direction(L_IN1, GPIO_MODE_OUTPUT);
  gpio_set_direction(L_IN2, GPIO_MODE_OUTPUT);

  gpio_set_direction(R_IN1, GPIO_MODE_OUTPUT);
  gpio_set_direction(R_IN2, GPIO_MODE_OUTPUT);

  // PWM timer configuration
  ledc_timer_config_t pwm_timer = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .timer_num = LEDC_TIMER_0,
    .duty_resolution = LEDC_TIMER_10_BIT,
    .freq_hz = PWM_FREQ,
    .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&pwm_timer);

  // Left motor PWM
  ledc_channel_config_t pwm_left = {
    .gpio_num = L_PWM,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0
  };
  ledc_channel_config(&pwm_left);

  // Right motor PWM
  ledc_channel_config_t pwm_right = {
    .gpio_num = R_PWM,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = LEDC_CHANNEL_1,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0
  };
  ledc_channel_config(&pwm_right);

  // Initial motor state (stopped)
  gpio_set_level(L_IN1, 0);
  gpio_set_level(L_IN2, 0);

  gpio_set_level(R_IN1, 0);
  gpio_set_level(R_IN2, 0);
}

void loop() {
  int distance = hcsr04_read_cm(&hcsr04);
  bool border_found = ldr_read(&ldr);

  printf("Distância: %d\n", distance);
  printf("LDR: %d\n", border_found);
  printf("============\n");

  if (distance > 0 && distance < 30) {
    // Both motors forward at 50%
    gpio_set_level(R_IN1, 1);
    gpio_set_level(R_IN2, 0);

    gpio_set_level(L_IN1, 1);
    gpio_set_level(L_IN2, 0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, PWM_DUTY_50);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, PWM_DUTY_50);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

  } else {
    // Turn in place at 25%
    // Right motor forward
    gpio_set_level(R_IN1, 1);
    gpio_set_level(R_IN2, 0);

    // Left motor backward
    gpio_set_level(L_IN1, 0);
    gpio_set_level(L_IN2, 1);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, PWM_DUTY_25);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, PWM_DUTY_25);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
  }
}

void app_main() {
  setup();

  while (true) {
    loop();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
