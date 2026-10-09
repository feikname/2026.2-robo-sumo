// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// pdMS_TO_TICKS
#include "freertos/FreeRTOS.h"

// vTaskDelay
#include "freertos/task.h"

// prototype definitions
#include "motor.h"

static motor_struct _default_motor_struct(gpio_num_t in1, gpio_num_t in2) {
  motor_struct motor = {
      .in1 = in1,
      .in2 = in2,
      .stopped = true,
      .direction = MOTOR_DIRECTION_FORWARD,
      .speed = MOTOR_SPEED_FAST,
  };

  return motor;
}

motor_set motors_create(gpio_num_t in1, gpio_num_t in2, gpio_num_t in3,
                        gpio_num_t in4) {
  motor_set motors = {
      .left = _default_motor_struct(in1, in2),
      .right = _default_motor_struct(in3, in4),
  };

  return motors;
}

void motors_setup(motor_set *ms) {
  gpio_set_direction(ms->left.in1, GPIO_MODE_OUTPUT);
  gpio_set_direction(ms->left.in2, GPIO_MODE_OUTPUT);
  gpio_set_direction(ms->right.in1, GPIO_MODE_OUTPUT);
  gpio_set_direction(ms->right.in2, GPIO_MODE_OUTPUT);

  gpio_set_level(ms->left.in1, 0);
  gpio_set_level(ms->left.in2, 0);
  gpio_set_level(ms->right.in1, 0);
  gpio_set_level(ms->right.in2, 0);
}

void motor_set_movement(motor_struct *m, motor_direction dir,
                        motor_speed speed) {
  // TODO: use motor speed
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

void motor_stop(motor_struct *m) {
  gpio_set_level(m->in1, 0);
  gpio_set_level(m->in2, 0);
  m->stopped = true;
}

void motors_stop_all(motor_set *ms) {
  motor_stop(&ms->left);
  motor_stop(&ms->right);
}

void set_movement(motor_set *ms, movement m) {
  switch (m) {
  case MOVE_FORWARD:
    motor_set_movement(&ms->left, MOTOR_DIRECTION_FORWARD, MOTOR_SPEED_FAST);
    motor_set_movement(&ms->right, MOTOR_DIRECTION_FORWARD, MOTOR_SPEED_FAST);
    break;
  case MOVE_BACKWARD:
    motor_set_movement(&ms->left, MOTOR_DIRECTION_BACKWARD, MOTOR_SPEED_FAST);
    motor_set_movement(&ms->right, MOTOR_DIRECTION_BACKWARD, MOTOR_SPEED_FAST);
    break;
  case MOVE_STEER_RIGHT:
    motor_set_movement(&ms->left, MOTOR_DIRECTION_FORWARD, MOTOR_SPEED_FAST);
    motor_set_movement(&ms->right, MOTOR_DIRECTION_FORWARD, MOTOR_SPEED_SLOW);
    break;
  case MOVE_STEER_LEFT:
    motor_set_movement(&ms->left, MOTOR_DIRECTION_FORWARD, MOTOR_SPEED_SLOW);
    motor_set_movement(&ms->right, MOTOR_DIRECTION_FORWARD, MOTOR_SPEED_FAST);
    break;
  case MOVE_STATIONARY:
    motors_stop_all(ms);
    break;
  case MOVE_SPIN_CLOCKWISE:
    motor_set_movement(&ms->left, MOTOR_DIRECTION_FORWARD, MOTOR_SPEED_FAST);
    motor_stop(&ms->right);
    break;
  case MOVE_SPIN_COUNTERCLOCKWISE:
    motor_stop(&ms->left);
    motor_set_movement(&ms->right, MOTOR_DIRECTION_FORWARD, MOTOR_SPEED_FAST);
    break;
  }
}

void do_movement_for(motor_set *ms, movement m, int duration) {
  set_movement(ms, m);
  vTaskDelay(pdMS_TO_TICKS(duration));
}
