#ifndef MOTOR_H
#define MOTOR_H

// TODO remove me
// hardcoded LEDs for wokwi
#define LED_DIREITA GPIO_NUM_16
#define LED_ESQUERDA GPIO_NUM_32
#define LED_TRAS GPIO_NUM_27
#define LED_FRENTE GPIO_NUM_13

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
  DIRECTION_FRONT,
  DIRECTION_FRONT_LEFT,
  DIRECTION_LEFT,
  DIRECTION_REAR_LEFT,
  DIRECTION_REAR,
  DIRECTION_REAR_RIGHT,
  DIRECTION_RIGHT,
  DIRECTION_FRONT_RIGHT,
} Direction;

void motors_setup();
void do_movement(Direction dir, int duration);
void do_maneuver(Maneuver m);
void do_evasion_maneuveur();

#endif
