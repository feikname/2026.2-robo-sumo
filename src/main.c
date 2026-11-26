// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// LEDC PWM
#include "driver/ledc.h"

// pdMS_TO_TICKS
#include "freertos/FreeRTOS.h"

// vTaskDelay
#include "freertos/task.h"

// printf
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_timer.h"

// system components
#include "distancia.h"
#include "hcsr04.h"
#include "ldr.h"
#include "motor.h"

// pin definitions
#include "pins.h"

void setup();
void loop();
void app_main();



// ============================================================================
// CONSTANTES DE CALIBRAÇÃO (Ajuste estes valores nos testes na arena)
// ============================================================================

// Please do not set loop delay below 64ms, to reduce interference from HC-SR04 previous echoes
#define LOOP_DELAY 64
#define STARTUP_DELAY 5000

// Sensor LDR (1 = faixa branca / nível alto; troque para 0 se for ativo-baixo)
#define LDR_WHITE 1
#define LDR_TRIGGER_COUNT 3

// Limites de distância do ultrassônico (cm)
#define DIST_DETECT_MAX_CM 55
#define DIST_LOST_MIN_CM 60
#define DIST_JUMP_THRESHOLD_CM 15
#define STALKING_TOLERANCE_CM 3

// Tempos dos estados (ms)
#define SEARCH_MAX_SWEEP_MS 1500
#define INERTIA_COMPENSATION_MS 40 // Ajuste fino (30 a 50 ms) para mirar no centro
#define STALKING_DURATION_MS 800   // Tempo parado confirmando o alvo (0,8 a 1,0 s)
#define CREEPING_DURATION_MS 600   // Tempo avançando devagar (0,6 s)
#define CHARGING_TIMEOUT_MS 1000   // Limite de segurança empurrando (2,5 s)

// Parâmetros do Retreat (fuga da borda)
#define RETREAT_PULSE_MS 150     // Cada pulso de ré
#define RETREAT_MAX_ATTEMPTS 4   // Máximo de tentativas antes de travar por segurança
#define RETREAT_MARGIN_MS 80     // Ré extra após pisar no preto
#define TURN_135_DEG_MS 450      // Calibrar na prática para girar ~135 graus

// Velocidades específicas
#define CREEPING_SPEED MOTOR_SPEED_25



// ============================================================================
// DEFINIÇÃO DOS ESTADOS (Máquina de Estados - Hunting Prey v2)
// ============================================================================

typedef enum {
  STATE_SEARCHING,
  STATE_STALKING,
  STATE_CREEPING,
  STATE_CHARGING,
  STATE_RETREAT
} RobotState;

typedef enum {
  SEARCH_SCANNING,  // 1A: Girando procurando a primeira borda da garrafa
  SEARCH_MEASURING  // 1B: Medindo o tempo até a outra borda da garrafa
} SearchSubState;

// Variáveis globais de hardware
hcsr04_struct hcsr04;
motor_set motors;
ldr_struct ldr;

// Variáveis globais de controle da máquina de estados
RobotState current_state = STATE_SEARCHING;
SearchSubState search_substate = SEARCH_SCANNING;

int ldr_white_count = 0;
int64_t state_start_ms = 0;

// Variáveis auxiliares do Searching
int64_t sweep_start_ms = 0;
int last_valid_dist = -1;
bool ignore_wide_obstacle = false;

// Variáveis auxiliares do Stalking e Creeping
int stalking_d0 = -1;
int creeping_start_dist = -1;



// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

// Retorna o tempo atual desde que o ESP32 ligou, em milissegundos
static int64_t millis() {
  return esp_timer_get_time() / 1000;
}

// Garante que a distância é válida (> 0, ignorando erro -1) e dentro do limite
static bool is_target_visible(int dist, int max_cm) {
  return (dist > 0 && dist < max_cm);
}

// Aciona ambos os motores com a mesma direção e velocidade
static void drive_both(motor_direction dir, motor_speed speed) {
  motor_set_direction_speed(&motors.left, dir, speed);
  motor_set_direction_speed(&motors.right, dir, speed);
}

// Realiza a troca de estado reiniciando o cronômetro do estado atual
static void enter_state(RobotState next_state) {
  current_state = next_state;
  state_start_ms = millis();

  if (next_state == STATE_SEARCHING) {
    search_substate = SEARCH_SCANNING;
    ignore_wide_obstacle = false;
  } else if (next_state == STATE_STALKING) {
    stalking_d0 = -1;
  }
}



// ============================================================================
// SETUP E LOOP PRINCIPAL
// ============================================================================

void setup() {
  // Timeout de 25ms (~4,3m máx); pode reduzir para ~10000us (10ms) para leituras mais rápidas
  hcsr04 = hcsr04_create(TRIG, ECHO, 25000);
  hcsr04_setup(&hcsr04);

  ldr = ldr_create(LDR);
  ldr_setup(&ldr);

  motors = motors_create(L_IN1, L_IN2, L_PWM, R_IN1, R_IN2, R_PWM);
  motors_setup(&motors);

  printf("Aguardando %d ms (regra de inicio)...\n", STARTUP_DELAY);
  vTaskDelay(pdMS_TO_TICKS(STARTUP_DELAY));

  enter_state(STATE_SEARCHING);
}

void loop() {
  int64_t now_ms = millis();
  int ldr_val = ldr_read(&ldr);

  // --------------------------------------------------------------------------
  // 1. INTERRUPÇÃO GLOBAL DE BORDA (Avaliada em todo ciclo acima dos estados)
  // --------------------------------------------------------------------------
  if (ldr_val == LDR_WHITE) {
    ldr_white_count++;
  } else {
    ldr_white_count = 0;
  }

  // if (ldr_white_count >= LDR_TRIGGER_COUNT && current_state != STATE_RETREAT) {
  //   printf("[GLOBAL] Borda branca detectada %dx! Abortando para RETREAT.\n", ldr_white_count);
  //   motors_stop_al;l(&motors);
  //   ldr_white_count = 0;
  //   enter_state(STATE_RETREAT);
  // }

  // Durante o Charging, ignoramos o ultrassônico (zona cega de 2-3 cm)
  int distance = -1;
  if (current_state != STATE_CHARGING && current_state != STATE_RETREAT) {
    distance = hcsr04_read_cm(&hcsr04);
  }

  // --------------------------------------------------------------------------
  // 2. MÁQUINA DE ESTADOS
  // --------------------------------------------------------------------------
  switch (current_state) {

  // ==========================================================================
  // ESTADO 1: SEARCHING (Varredura e Centralização sem Encoders)
  // ==========================================================================
  case STATE_SEARCHING: {
    set_movement(&motors, MOVE_SPIN_CLOCKWISE);

    if (search_substate == SEARCH_SCANNING) {
      // Sub-estado 1A: Procurando a borda inicial do alvo
      if (!ignore_wide_obstacle && is_target_visible(distance, DIST_DETECT_MAX_CM)) {
        printf("[1A -> 1B] Alvo detectado a %d cm. Iniciando cronometro.\n", distance);
        sweep_start_ms = now_ms;
        last_valid_dist = distance;
        search_substate = SEARCH_MEASURING;
      } else if (!is_target_visible(distance, DIST_LOST_MIN_CM)) {
        // Libera a trava assim que terminar de passar por um obstáculo largo
        ignore_wide_obstacle = false;
      }
    } else if (search_substate == SEARCH_MEASURING) {
      // Sub-estado 1B: Medindo largura angular até o fim do objeto
      int64_t elapsed_ms = now_ms - sweep_start_ms;

      if (elapsed_ms > SEARCH_MAX_SWEEP_MS) {
        // Passou de 1,5s sem sair do objeto -> provável parede ou obstáculo largo
        printf("[1B] Obstaculo muito largo (>%d ms). Ignorando.\n", SEARCH_MAX_SWEEP_MS);
        ignore_wide_obstacle = true;
        search_substate = SEARCH_SCANNING;
      } else if (distance < 0 || distance > DIST_LOST_MIN_CM ||
                 (distance - last_valid_dist) > DIST_JUMP_THRESHOLD_CM) {
        // Saiu da borda da garrafa! Calcula metade do tempo + compensação de inércia
        motors_stop_all(&motors);

        int64_t center_turn_ms = (elapsed_ms / 2) + INERTIA_COMPENSATION_MS;
        printf("[1C] Fim do alvo (largura=%lld ms). Voltando %lld ms para centralizar.\n",
               elapsed_ms, center_turn_ms);

        set_movement(&motors, MOVE_SPIN_COUNTERCLOCKWISE);
        vTaskDelay(pdMS_TO_TICKS(center_turn_ms));
        motors_stop_all(&motors);

        enter_state(STATE_STALKING);
      } else {
        last_valid_dist = distance;
      }
    }
    break;
  }

  // ==========================================================================
  // ESTADO 2: STALKING (Confirmação Estática)
  // ==========================================================================
  case STATE_STALKING: {
    motors_stop_all(&motors);

    // Captura a distância inicial D0 na primeira leitura válida após centralizar
    if (stalking_d0 < 0) {
      if (is_target_visible(distance, DIST_LOST_MIN_CM)) {
        stalking_d0 = distance;
        printf("[STALKING] D0 registrado = %d cm.\n", stalking_d0);
      } else if ((now_ms - state_start_ms) > 200) {
        printf("[STALKING] Alvo nao confirmado apos giro. Voltando para SEARCHING.\n");
        enter_state(STATE_SEARCHING);
      }
      break;
    }

    // Valida se o alvo sumiu ou variou além da tolerância (+-3 cm)
    if (distance < 0 || distance > DIST_LOST_MIN_CM ||
        abs(distance - stalking_d0) > STALKING_TOLERANCE_CM) {
      printf("[STALKING] Alvo instavel ou perdido (dist=%d, D0=%d). Voltando para SEARCHING.\n",
             distance, stalking_d0);
      enter_state(STATE_SEARCHING);
    } else if ((now_ms - state_start_ms) >= STALKING_DURATION_MS) {
      printf("[STALKING -> CREEPING] Alvo estavel confirmado!\n");
      creeping_start_dist = distance;
      drive_both(MOTOR_DIRECTION_FORWARD, CREEPING_SPEED);
      enter_state(STATE_CREEPING);
    }
    break;
  }

  // ==========================================================================
  // ESTADO 3: CREEPING (Aproximação Lenta e Trava de Alvo)
  // ==========================================================================
  case STATE_CREEPING: {
    if (distance < 0 || distance > DIST_LOST_MIN_CM) {
      printf("[CREEPING] Alvo perdido durante aproximacao. Voltando para SEARCHING.\n");
      motors_stop_all(&motors);
      enter_state(STATE_SEARCHING);
    } else if ((now_ms - state_start_ms) >= CREEPING_DURATION_MS) {
      // Ao fim dos 0,6s, valida se segue visível e mais perto (ou já encostando <= 10cm)
      if (is_target_visible(distance, DIST_DETECT_MAX_CM) &&
          (distance < creeping_start_dist || distance <= 10)) {
        printf("[CREEPING -> CHARGING] Alvo travado a %d cm! Ataque maximo!\n", distance);
        drive_both(MOTOR_DIRECTION_FORWARD, MOTOR_SPEED_75);
        enter_state(STATE_CHARGING);
      } else {
        printf("[CREEPING] Distancia nao diminuiu (ini=%d, fim=%d). Voltando para SEARCHING.\n",
               creeping_start_dist, distance);
        motors_stop_all(&motors);
        enter_state(STATE_SEARCHING);
      }
    }
    break;
  }

  // ==========================================================================
  // ESTADO 4: CHARGING (Investida em 100% ignorando o ultrassônico)
  // ==========================================================================
  case STATE_CHARGING: {
    // A saída normal ocorre pela interrupção global do LDR ao chegar na borda.
    // Aqui tratamos apenas o timeout de segurança caso a garrafa escape para o lado.
    if ((now_ms - state_start_ms) >= CHARGING_TIMEOUT_MS) {
      printf("[CHARGING] Timeout de %d ms sem achar borda. Voltando para SEARCHING.\n",
             CHARGING_TIMEOUT_MS);
      motors_stop_all(&motors);
      enter_state(STATE_SEARCHING);
    }
    break;
  }

  // ==========================================================================
  // ESTADO 5: RETREAT (Fuga da Borda e Giro para o Centro)
  // ==========================================================================
  case STATE_RETREAT: {
    bool confirmed_black = false;

    // Passos 1 e 2: Pulsos de ré de 150 ms até pisar no preto (máx. 4 vezes)
    for (int attempt = 1; attempt <= RETREAT_MAX_ATTEMPTS; attempt++) {
      printf("[RETREAT] Pulso de re %d/%d...\n", attempt, RETREAT_MAX_ATTEMPTS);
      set_movement(&motors, MOVE_BACKWARD);
      vTaskDelay(pdMS_TO_TICKS(RETREAT_PULSE_MS));
      motors_stop_all(&motors);
      vTaskDelay(pdMS_TO_TICKS(20)); // Pequena estabilização antes de ler o LDR

      if (ldr_read(&ldr) != LDR_WHITE) {
        confirmed_black = true;
        break;
      }
    }

    if (confirmed_black) {
      // Passo 3: Recuo extra de margem + giro de ~135° para apontar ao centro
      printf("[RETREAT] Piso preto confirmado. Recuando margem e girando 135 graus.\n");
      set_movement(&motors, MOVE_BACKWARD);
      vTaskDelay(pdMS_TO_TICKS(RETREAT_MARGIN_MS));

      set_movement(&motors, MOVE_SPIN_CLOCKWISE);
      vTaskDelay(pdMS_TO_TICKS(TURN_135_DEG_MS));

      motors_stop_all(&motors);
      ldr_white_count = 0;
      enter_state(STATE_SEARCHING);
    } else {
      // Trava de segurança: após 4 pulsos ainda vê branco (risco de cair de costas)
      printf("[RETREAT] PERIGO: Ainda na faixa branca apos %d tentativas. Motores travados!\n",
             RETREAT_MAX_ATTEMPTS);
      motors_stop_all(&motors);
    }
    break;
  }
  }
}

void app_main() {
  setup();

  while (true) {
    loop();
    vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY));
  }
}
