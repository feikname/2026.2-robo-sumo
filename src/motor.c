// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// pdMS_TO_TICKS
#include "freertos/FreeRTOS.h"

// vTaskDelay
#include "freertos/task.h"

// LEDC PWM
#include "driver/ledc.h"

// prototype definitions
#include "motor.h"

// PWM configuration (10-bit)
#define PWM_FREQ 20000
#define PWM_DUTY_100 1023 // 75% duty cycle
#define PWM_DUTY_75 768   // 75% duty cycle
#define PWM_DUTY_50 512   // 75% duty cycle
#define PWM_DUTY_25 256   // 25% duty cycle
#define PWM_DUTY_10 102   // 25% duty cycle

// PWM channels
#define PWM_CHANNEL_L LEDC_CHANNEL_0
#define PWM_CHANNEL_R LEDC_CHANNEL_1

// Misc
#define DEFAULT_SPEED MOTOR_SPEED_10

static motor_struct _default_motor_struct(gpio_num_t in1, gpio_num_t in2,
                                          gpio_num_t pwm,
                                          ledc_channel_t pwm_channel) {
  motor_struct motor = {
      .in1 = in1,
      .in2 = in2,
      .pwm = pwm,
      .pwm_channel = pwm_channel,
      .direction = MOTOR_DIRECTION_FORWARD,
      .speed = MOTOR_SPEED_100,
  };

  return motor;
}

motor_set motors_create(gpio_num_t l_in1, gpio_num_t l_in2, gpio_num_t l_pwm,
                        gpio_num_t r_in1, gpio_num_t r_in2, gpio_num_t r_pwm) {
  motor_set motors = {
      .left = _default_motor_struct(l_in1, l_in2, l_pwm, PWM_CHANNEL_L),
      .right = _default_motor_struct(r_in1, r_in2, r_pwm, PWM_CHANNEL_R),
  };

  return motors;
}

void motors_setup(motor_set *ms) {
  // clockwise / counter-clockwise
  gpio_set_direction(ms->left.in1, GPIO_MODE_OUTPUT);
  gpio_set_direction(ms->left.in2, GPIO_MODE_OUTPUT);
  gpio_set_direction(ms->right.in1, GPIO_MODE_OUTPUT);
  gpio_set_direction(ms->right.in2, GPIO_MODE_OUTPUT);

  gpio_set_level(ms->left.in1, 0);
  gpio_set_level(ms->left.in2, 0);
  gpio_set_level(ms->right.in1, 0);
  gpio_set_level(ms->right.in2, 0);

  // speed contro
  ledc_timer_config_t pwm_timer = {.speed_mode = LEDC_LOW_SPEED_MODE,
                                   .timer_num = LEDC_TIMER_0,
                                   .duty_resolution = LEDC_TIMER_10_BIT,
                                   .freq_hz = PWM_FREQ,
                                   .clk_cfg = LEDC_AUTO_CLK};
  ledc_timer_config(&pwm_timer);

  ledc_channel_config_t pwm_left = {.gpio_num = ms->left.pwm,
                                    .speed_mode = LEDC_LOW_SPEED_MODE,
                                    .channel = PWM_CHANNEL_L,
                                    .timer_sel = LEDC_TIMER_0,
                                    .duty = 0,
                                    .hpoint = 0};
  ledc_channel_config(&pwm_left);

  ledc_channel_config_t pwm_right = {.gpio_num = ms->right.pwm,
                                     .speed_mode = LEDC_LOW_SPEED_MODE,
                                     .channel = PWM_CHANNEL_R,
                                     .timer_sel = LEDC_TIMER_0,
                                     .duty = 0,
                                     .hpoint = 0};
  ledc_channel_config(&pwm_right);

  ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL_L, 0);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL_R, 0);

  ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL_L);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL_R);
}

int _to_pwm(motor_speed speed) {
  switch (speed) {
  case MOTOR_SPEED_100:
    return PWM_DUTY_100;

  case MOTOR_SPEED_75:
    return PWM_DUTY_75;

  case MOTOR_SPEED_50:
    return PWM_DUTY_50;

  case MOTOR_SPEED_25:
    return PWM_DUTY_25;

  case MOTOR_SPEED_10:
    return PWM_DUTY_10;

  case MOTOR_SPEED_0:
    return 0;

  default:
    return PWM_DUTY_100;
  }
}

void motor_set_direction(motor_struct *m, motor_direction dir) {
  switch (dir) {
  case MOTOR_DIRECTION_FORWARD:
    gpio_set_level(m->in1, 1);
    gpio_set_level(m->in2, 0);
    break;

  case MOTOR_DIRECTION_BACKWARD:
    gpio_set_level(m->in1, 0);
    gpio_set_level(m->in2, 1);
    break;
  }
}

void motor_set_speed(motor_struct *m, motor_speed speed) {
  int pwm = _to_pwm(speed);

  ledc_set_duty(LEDC_LOW_SPEED_MODE, m->pwm_channel, pwm);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, m->pwm_channel);
}

void motor_set_direction_speed(motor_struct *m, motor_direction dir,
                               motor_speed speed) {

  // without this, when changing direction, we modify IN1/IN2 while
  // the previous PWM duty cycle is still active.
  //
  // this can produce a brief unwanted motor-driving condition
  // during direction transitions.
  motor_set_speed(m, MOTOR_SPEED_0);

  motor_set_direction(m, dir);
  motor_set_speed(m, speed);
}

void motor_stop(motor_struct *m) {
  gpio_set_level(m->in1, 0);
  gpio_set_level(m->in2, 0);

  ledc_set_duty(LEDC_LOW_SPEED_MODE, m->pwm_channel, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, m->pwm_channel);
}

void motors_stop_all(motor_set *ms) {
  motor_stop(&ms->left);
  motor_stop(&ms->right);
}

void set_movement(motor_set *ms, movement m) {
  switch (m) {
  case MOVE_FORWARD:
    motor_set_direction_speed(&ms->left, MOTOR_DIRECTION_FORWARD,
                              DEFAULT_SPEED);
    motor_set_direction_speed(&ms->right, MOTOR_DIRECTION_FORWARD,
                              DEFAULT_SPEED);
    break;
  case MOVE_BACKWARD:
    motor_set_direction_speed(&ms->left, MOTOR_DIRECTION_BACKWARD,
                              DEFAULT_SPEED);
    motor_set_direction_speed(&ms->right, MOTOR_DIRECTION_BACKWARD,
                              DEFAULT_SPEED);
    break;
  case MOVE_STEER_RIGHT:
    motor_set_direction_speed(&ms->left, MOTOR_DIRECTION_FORWARD,
                              MOTOR_SPEED_50);
    motor_set_direction_speed(&ms->right, MOTOR_DIRECTION_FORWARD,
                              MOTOR_SPEED_25);
    break;
  case MOVE_STEER_LEFT:
    motor_set_direction_speed(&ms->left, MOTOR_DIRECTION_FORWARD,
                              MOTOR_SPEED_25);
    motor_set_direction_speed(&ms->right, MOTOR_DIRECTION_FORWARD,
                              MOTOR_SPEED_50);
    break;
  case MOVE_STATIONARY:
    motors_stop_all(ms);
    break;
  case MOVE_SPIN_CLOCKWISE:
    motor_set_direction_speed(&ms->left, MOTOR_DIRECTION_BACKWARD,
                              MOTOR_SPEED_25);
    motor_set_direction_speed(&ms->right, MOTOR_DIRECTION_FORWARD,
                              MOTOR_SPEED_25);
    break;

  case MOVE_SPIN_COUNTERCLOCKWISE:
    motor_set_direction_speed(&ms->left, MOTOR_DIRECTION_FORWARD,
                              MOTOR_SPEED_25);
    motor_set_direction_speed(&ms->right, MOTOR_DIRECTION_BACKWARD,
                              MOTOR_SPEED_25);
    break;

    break;
  }
}

void do_movement_for(motor_set *ms, movement m, int duration) {
  set_movement(ms, m);
  vTaskDelay(pdMS_TO_TICKS(duration));
}
