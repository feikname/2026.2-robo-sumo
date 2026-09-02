#ifndef MOTOR_H
#define MOTOR_H

// TODO remove me
// hardcoded LEDs for wokwi
#define LED_DIREITA GPIO_NUM_16
#define LED_ESQUERDA GPIO_NUM_32
#define LED_TRAS GPIO_NUM_27
#define LED_FRENTE GPIO_NUM_13

typedef enum {
  MOTOR_INACTIVE,
  MOTOR_FORWARD_FAST,
  MOTOR_FORWARD_SLOW,
  MOTOR_BACKWARD_FAST,
  MOTOR_BACKWARD_SLOW,
} MotorState;

typedef struct {
  MotorState left_motor;
  MotorState right_motor;
} motor_set;

typedef enum {
  STRATEGY_DEFAULT,
  STRATEGY_AGGRESIVE,
  STRATEGY_DEFENSIVE,
  STRATEGY_RANDOM // Giro, Avanço, Recuo
} Strategy;

typedef enum {
  MANEUEVER_EVASION,
} Maneuver;

typedef enum {
  DIRECTION_FORWARD,
  DIRECTION_STEER_LEFT,
  DIRECTION_BACKWARD,
  DIRECTION_STEER_RIGHT,
  DIRECTION_STATIONARY,
  DIRECTION_SPIN_CLOCKWISE,
  DIRECTION_SPIN_COUNTERCLOCKWISE,
} Direction;

typedef enum {
  SPEED_FAST,
  SPEED_SLOW,
} Speed;

motor_set motors_create();
void motors_setup();
void set_motor_movement(Direction dir, Speed speed, motor_set *ms);
void do_movement(Direction dir, Speed speed, int duration, motor_set *ms);
void do_maneuver(Maneuver m, motor_set *ms);
void do_maneuver_and_restore_state(Maneuver m, motor_set *ms);

#endif
