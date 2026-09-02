#ifndef MOTOR_H
#define MOTOR_H

#include "driver/gpio.h"

#define LED_DIREITA GPIO_NUM_16
#define LED_ESQUERDA GPIO_NUM_32
#define LED_TRAS GPIO_NUM_27
#define LED_FRENTE GPIO_NUM_13

typedef enum {
  Default,
  Aggressive,
  Defensive,
  Random // Giro, Avanço, Recuo
} Strategy;

typedef enum {
  FRONT,
  SUPERIOR_LEFT,
  LEFT,
  INFERIOR_LEFT,
  BACK,
  INFERIOR_RIGHT,
  RIGHT,
  SUPERIOR_RIGHT,
} Direction;

void setup_motor();
void ir_duracao(Direction direcao, int duracao);
void exec_manobra();
void initial_start(Strategy strat);

#endif
