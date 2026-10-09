#ifndef MOTOR_H
#define MOTOR_H

// gpio_num_t
#include "hal/gpio_types.h"

typedef enum {
  MOTOR_DIRECTION_FORWARD,
  MOTOR_DIRECTION_BACKWARD,
} motor_direction;

typedef enum {
  MOTOR_SPEED_SLOW,
  MOTOR_SPEED_FAST,
} motor_speed;

// TODO: speed control pin (EN)
typedef struct {
  gpio_num_t in1;
  gpio_num_t in2;
  bool stopped;
  motor_direction direction;
  motor_speed speed;
} motor_struct;

typedef struct {
  motor_struct left;
  motor_struct right;
} motor_set;

typedef enum {
  MOVE_FORWARD,
  MOVE_BACKWARD,
  MOVE_STEER_RIGHT,
  MOVE_STEER_LEFT,
  MOVE_STATIONARY,
  MOVE_SPIN_CLOCKWISE,
  MOVE_SPIN_COUNTERCLOCKWISE,
} movement;

// TODO: speed control pin (EN)
motor_set motors_create(gpio_num_t in1, gpio_num_t in2, gpio_num_t in3,
                        gpio_num_t in4);
void motors_setup(motor_set *ms);

void motor_set_movement(motor_struct *m, motor_direction dir,
                        motor_speed speed);
void motor_stop(motor_struct *m);

void motors_stop_all(motor_set *ms);

void set_movement(motor_set *ms, movement m);
void do_movement_for(motor_set *ms, movement m, int duration);

#endif
