#ifndef MOTOR_H
#define MOTOR_H

// gpio_num_t
#include "hal/gpio_types.h"

typedef enum {
  MOTOR_DIRECTION_FORWARD,
  MOTOR_DIRECTION_BACKWARD,
} motor_direction;

//
// BEWARE!
// DC motors have a minimum starting voltage/current requirement
//
// A motor that can maintain rotation at 25% duty might not
// be able to start rotating at that duty
//
// This can produce behavior that looks like intermittent motor failures!
typedef enum {
  MOTOR_SPEED_100,
  MOTOR_SPEED_75,
  MOTOR_SPEED_50,
  MOTOR_SPEED_25,
  MOTOR_SPEED_10,
  MOTOR_SPEED_0,
} motor_speed;

typedef struct {
  gpio_num_t in1;
  gpio_num_t in2;
  gpio_num_t pwm;
  ledc_channel_t pwm_channel;
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

motor_set motors_create(gpio_num_t l_in1, gpio_num_t l_in2, gpio_num_t l_pwm,
                        gpio_num_t r_in1, gpio_num_t r_in2, gpio_num_t r_pwm);

void motors_setup(motor_set *ms);

void motor_set_direction(motor_struct *m, motor_direction dir);
void motor_set_speed(motor_struct *m, motor_speed speed);
void motor_set_direction_speed(motor_struct *m, motor_direction dir,
                               motor_speed speed);

void motor_stop(motor_struct *m);

void motors_stop_all(motor_set *ms);

void set_movement(motor_set *ms, movement m);
void do_movement_for(motor_set *ms, movement m, int duration);

#endif
