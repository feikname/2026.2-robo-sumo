// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// pdMS_TO_TICKS
#include "freertos/FreeRTOS.h"

// vTaskDelay
#include "freertos/task.h"

// prototype definitions
#include "motor.h"

// 'private' methods
static Direction to_direction(motor_set *motor_set);
static void update_motor_leds(motor_set *motor_set);
static void do_evasion_maneuver(motor_set *ms);

motor_set motors_create() {
  motor_set motors = {
      .left_motor = MOTOR_INACTIVE,
      .right_motor = MOTOR_INACTIVE,
  };

  return motors;
}

void motors_setup() {
  gpio_set_direction(LED_DIREITA, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_ESQUERDA, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_TRAS, GPIO_MODE_OUTPUT);
  gpio_set_direction(LED_FRENTE, GPIO_MODE_OUTPUT);
}

static Direction to_direction(motor_set *motor_set) {
  MotorState l = motor_set->left_motor;
  MotorState r = motor_set->right_motor;

  if (l == MOTOR_FORWARD_SLOW && r == MOTOR_FORWARD_SLOW) {
    return DIRECTION_FORWARD;
  }
  if (l == MOTOR_FORWARD_FAST && r == MOTOR_FORWARD_FAST) {
    return DIRECTION_FORWARD;
  }
  if (l == MOTOR_BACKWARD_SLOW && r == MOTOR_BACKWARD_SLOW) {
    return DIRECTION_BACKWARD;
  }
  if (l == MOTOR_BACKWARD_FAST && r == MOTOR_BACKWARD_FAST) {
    return DIRECTION_BACKWARD;
  }
  if (l == MOTOR_FORWARD_FAST && r == MOTOR_FORWARD_SLOW) {
    return DIRECTION_STEER_RIGHT;
  }
  if (l == MOTOR_FORWARD_SLOW && r == MOTOR_FORWARD_FAST) {
    return DIRECTION_STEER_LEFT;
  }
  if (l == MOTOR_INACTIVE && r == MOTOR_INACTIVE) {
    return DIRECTION_STATIONARY;
  }
  if (l == MOTOR_INACTIVE && r == MOTOR_INACTIVE) {
    return DIRECTION_STATIONARY;
  }
  if (l == MOTOR_FORWARD_SLOW && r == MOTOR_INACTIVE) {
    return DIRECTION_SPIN_CLOCKWISE;
  }
  if (l == MOTOR_FORWARD_FAST && r == MOTOR_INACTIVE) {
    return DIRECTION_SPIN_CLOCKWISE;
  }
  if (l == MOTOR_FORWARD_SLOW && r == MOTOR_INACTIVE) {
    return DIRECTION_SPIN_COUNTERCLOCKWISE;
  }
  if (l == MOTOR_FORWARD_FAST && r == MOTOR_INACTIVE) {
    return DIRECTION_SPIN_COUNTERCLOCKWISE;
  }

  // Not really
  return DIRECTION_SPIN_CLOCKWISE;
}

static void update_motor_leds(motor_set *motor_set) {
  Direction dir = to_direction(motor_set);

  bool frente = false;
  bool esquerda = false;
  bool tras = false;
  bool direita = false;

  switch (dir) {
  case DIRECTION_FORWARD:
    frente = true;
    break;

  case DIRECTION_STEER_LEFT:
    esquerda = true;
    break;

  case DIRECTION_BACKWARD:
    tras = true;
    break;

  case DIRECTION_STEER_RIGHT:
    direita = true;
    break;

  case DIRECTION_SPIN_CLOCKWISE:
    frente = true;
    direita = true;
    break;

  case DIRECTION_SPIN_COUNTERCLOCKWISE:
    tras = true;
    esquerda = true;
    break;

  case DIRECTION_STATIONARY:
    break;
  }

  gpio_set_level(LED_FRENTE, frente);
  gpio_set_level(LED_ESQUERDA, esquerda);
  gpio_set_level(LED_TRAS, tras);
  gpio_set_level(LED_DIREITA, direita);
}

void set_motor_movement(Direction dir, Speed speed, motor_set *ms) {
  if (dir == DIRECTION_FORWARD) {
    if (speed == SPEED_SLOW) {
      ms->left_motor = MOTOR_FORWARD_SLOW;
      ms->right_motor = MOTOR_FORWARD_SLOW;
    }
    if (speed == SPEED_FAST) {
      ms->left_motor = MOTOR_FORWARD_FAST;
      ms->right_motor = MOTOR_FORWARD_FAST;
    }
  }
  if (dir == DIRECTION_STEER_LEFT) {
    ms->left_motor = MOTOR_FORWARD_SLOW;
    ms->right_motor = MOTOR_FORWARD_FAST;
  }
  if (dir == DIRECTION_BACKWARD) {
    if (speed == SPEED_SLOW) {
      ms->left_motor = MOTOR_BACKWARD_SLOW;
      ms->right_motor = MOTOR_BACKWARD_SLOW;
    }
    if (speed == SPEED_FAST) {
      ms->left_motor = MOTOR_BACKWARD_FAST;
      ms->right_motor = MOTOR_BACKWARD_FAST;
    }
  }
  if (dir == DIRECTION_STEER_RIGHT) {
    ms->left_motor = MOTOR_FORWARD_FAST;
    ms->right_motor = MOTOR_FORWARD_SLOW;
  }
  if (dir == DIRECTION_STATIONARY) {
    ms->left_motor = MOTOR_INACTIVE;
    ms->right_motor = MOTOR_INACTIVE;
  }
  if (dir == DIRECTION_SPIN_CLOCKWISE) {
    if (speed == SPEED_SLOW) {
      ms->left_motor = MOTOR_FORWARD_SLOW;
      ms->right_motor = MOTOR_INACTIVE;
    }
    if (speed == SPEED_FAST) {
      ms->left_motor = MOTOR_FORWARD_FAST;
      ms->right_motor = MOTOR_INACTIVE;
    }
  }
  if (dir == DIRECTION_SPIN_COUNTERCLOCKWISE) {
    if (speed == SPEED_SLOW) {
      ms->left_motor = MOTOR_FORWARD_SLOW;
      ms->right_motor = MOTOR_INACTIVE;
    }
    if (speed == SPEED_FAST) {
      ms->left_motor = MOTOR_FORWARD_FAST;
      ms->right_motor = MOTOR_INACTIVE;
    }
  }

  update_motor_leds(ms);
}

void do_movement(Direction dir, Speed speed, int duration, motor_set *ms) {
  set_motor_movement(dir, speed, ms);
  vTaskDelay(pdMS_TO_TICKS(duration));
}

void do_maneuver(Maneuver m, motor_set *ms) {
  if (m == MANEUEVER_EVASION) {
    do_evasion_maneuver(ms);
  }
}

void do_maneuver_and_restore_state(Maneuver m, motor_set *ms) {
  // this doesn't quite work. need to debug.
  motor_set original_ms = *ms;
  if (m == MANEUEVER_EVASION) {
    do_evasion_maneuver(ms);
  }
  *ms = original_ms;
}

static void do_evasion_maneuver(motor_set *ms) {
  do_movement(DIRECTION_BACKWARD, SPEED_FAST, 500, ms);

  // tanto faz se direita ou esquerda
  do_movement(DIRECTION_STEER_LEFT, SPEED_FAST, 150, ms);
}
