// Controle de pinos digitais (gpio_set_direction, gpio_set_level)
#include "driver/gpio.h"

// Controle de PWM por hardware (LEDC)
#include "driver/ledc.h"

// Temporizadores e tarefas do FreeRTOS (pdMS_TO_TICKS, vTaskDelay)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Funções matemáticas, como atan2f() para cálculo de ângulo no modo Gladiador
#include <math.h>

// Tipos booleanos, entrada/saída padrão e utilitários (printf, abs)
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Cronômetro de alta resolução do ESP32 (esp_timer_get_time)
#include "esp_timer.h"

// system components
#include "hcsr04.h"
#include "ldr.h"
#include "motor.h"

// pin definitions
#include "pins.h"

void setup();
void loop();
void app_main();



// ============================================================================
// SELEÇÃO DE MODO DE COMBATE
// ============================================================================

typedef enum {
  MODE_HUNTER,    // Modo Caçador (alvo parado)
  MODE_GLADIATOR  // Modo Gladiador (combate 1v1 contra robô móvel)
} RobotMode;

RobotMode current_mode = MODE_HUNTER;



// ============================================================================
// CONSTANTES DE CALIBRAÇÃO (Ajuste estes valores nos testes na arena)
// ============================================================================

// Please do not set loop delay below 64ms, to reduce interference from HC-SR04 previous echoes
#define LOOP_DELAY 64
#define STARTUP_DELAY 5000

// Sensor LDR
#define LDR_WHITE 0
#define LDR_TRIGGER_COUNT 3

// Limites de distância do ultrassônico (cm)
#define DIST_JUMP_THRESHOLD_CM 15

// Tempos dos estados (ms)
#define SEARCH_MAX_SWEEP_MS 1500

// Parâmetros do Retreat (fuga da borda)
#define RETREAT_PULSE_MS 150     // Cada pulso de ré
#define RETREAT_MAX_ATTEMPTS 4   // Máximo de tentativas antes de travar por segurança
#define RETREAT_MARGIN_MS 80     // Ré extra após pisar no preto
#define TURN_135_DEG_MS 450      // Calibrar na prática para girar ~120 graus

// Estrutura que concentra os parâmetros variáveis entre Hunter e Gladiator
typedef struct {
  int dist_detect_max_cm;
  int dist_lost_min_cm;
  int stalking_tolerance_cm;
  int64_t inertia_compensation_ms;
  int64_t stalking_duration_ms;
  int64_t creeping_duration_ms;
  int64_t charging_timeout_ms;
  motor_speed creeping_speed;
  motor_speed charging_speed;
  bool measure_target_width;
} ModeConfig;

ModeConfig cfg;



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

// Carrega os parâmetros do modo selecionado na variável current_mode
static void load_mode_config(RobotMode mode) {
  if (mode == MODE_HUNTER) {
    cfg.dist_detect_max_cm = 55;
    cfg.dist_lost_min_cm = 60;
    cfg.stalking_tolerance_cm = 3;
    cfg.inertia_compensation_ms = 64;
    cfg.stalking_duration_ms = 800;
    cfg.creeping_duration_ms = 300;
    cfg.charging_timeout_ms = 600;
    cfg.creeping_speed = MOTOR_SPEED_25;
    cfg.charging_speed = MOTOR_SPEED_75;
    cfg.measure_target_width = true;
  } else { // MODE_GLADIATOR
    cfg.dist_detect_max_cm = 65;
    cfg.dist_lost_min_cm = 75;
    cfg.stalking_tolerance_cm = 5;
    cfg.inertia_compensation_ms = 40;
    cfg.stalking_duration_ms = 100;
    cfg.creeping_duration_ms = 100;
    cfg.charging_timeout_ms = 1200;
    cfg.creeping_speed = MOTOR_SPEED_50;
    cfg.charging_speed = MOTOR_SPEED_75;
    cfg.measure_target_width = false;
  }
}

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
  load_mode_config(current_mode);

  // Timeout de 25ms (~4,3m máx); pode reduzir para ~10000us (10ms) para leituras mais rápidas
  hcsr04 = hcsr04_create(TRIG, ECHO, 25000);
  hcsr04_setup(&hcsr04);

  ldr = ldr_create(LDR);
  ldr_setup(&ldr);

  motors = motors_create(L_IN1, L_IN2, L_PWM, R_IN1, R_IN2, R_PWM);
  motors_setup(&motors);

  printf("Modo ativo: %s | Aguardando %d ms (regra de inicio)...\n",
         (current_mode == MODE_HUNTER) ? "HUNTER" : "GLADIATOR", STARTUP_DELAY);
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

  if (ldr_white_count >= LDR_TRIGGER_COUNT && current_state != STATE_RETREAT) {
    printf("[GLOBAL] Borda branca detectada %dx! Abortando para RETREAT.\n", ldr_white_count);
    motors_stop_all(&motors);
    ldr_white_count = 0;
    enter_state(STATE_RETREAT);
  }

  // Durante o Charging e o Retreat, ignoramos o ultrassônico (zona cega de 2-3 cm)
  int distance = -1;
  if (current_state != STATE_CHARGING && current_state != STATE_RETREAT) {
    distance = hcsr04_read_cm(&hcsr04);
    printf("Distância: %d\n", distance);
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
      if (!ignore_wide_obstacle && is_target_visible(distance, cfg.dist_detect_max_cm)) {
        if (cfg.measure_target_width) {
          // Modo Hunter: inicia cronômetro para medir largura da garrafa
          printf("[1A -> 1B] Alvo detectado a %d cm. Iniciando cronometro.\n", distance);
          sweep_start_ms = now_ms;
          last_valid_dist = distance;
          search_substate = SEARCH_MEASURING;
        } else {
          // Modo Gladiator: estima o ângulo até o centro de um alvo de 10 cm (raio = 5 cm)
          int effective_dist = (distance < 8) ? 8 : distance; // Evita giro exagerado se estiver colado
          float angle_deg = atan2f(5.0f, (float)effective_dist) * (180.0f / 3.14159265f);
          
          // Converte graus para ms usando a mesma referência de giro do robô (~3,33 ms/grau)
          int64_t extra_turn_ms = (int64_t)(angle_deg * (TURN_135_DEG_MS / 135.0f));

          printf("[1A -> STALKING] Oponente a %d cm. Girando +%lld ms (%.1f graus) para o centro.\n",
                 distance, extra_turn_ms, angle_deg);

          set_movement(&motors, MOVE_SPIN_CLOCKWISE);
          vTaskDelay(pdMS_TO_TICKS(extra_turn_ms));
          motors_stop_all(&motors);
          enter_state(STATE_STALKING);
        }
      } else if (!is_target_visible(distance, cfg.dist_lost_min_cm)) {
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
      } else if (distance < 0 || distance > cfg.dist_lost_min_cm ||
                 (distance - last_valid_dist) > DIST_JUMP_THRESHOLD_CM) {
        // Saiu da borda da garrafa! Calcula metade do tempo + compensação de inércia
        motors_stop_all(&motors);

        int64_t center_turn_ms = (elapsed_ms / 2) + cfg.inertia_compensation_ms;
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
  // ESTADO 2: STALKING (Confirmação de Alvo)
  // ==========================================================================
  case STATE_STALKING: {
    motors_stop_all(&motors);

    // Captura a distância inicial D0 na primeira leitura válida após centralizar
    if (stalking_d0 < 0) {
      if (is_target_visible(distance, cfg.dist_lost_min_cm)) {
        stalking_d0 = distance;
        printf("[STALKING] D0 registrado = %d cm.\n", stalking_d0);
      } else if ((now_ms - state_start_ms) > 200) {
        printf("[STALKING] Alvo nao confirmado apos giro. Voltando para SEARCHING.\n");
        enter_state(STATE_SEARCHING);
      }
      break;
    }

    // Valida se o alvo sumiu ou variou além da tolerância (+-3 cm)
    if (distance < 0 || distance > cfg.dist_lost_min_cm ||
        abs(distance - stalking_d0) > cfg.stalking_tolerance_cm) {
      printf("[STALKING] Alvo instavel ou perdido (dist=%d, D0=%d). Voltando para SEARCHING.\n",
             distance, stalking_d0);
      enter_state(STATE_SEARCHING);
    } else if ((now_ms - state_start_ms) >= cfg.stalking_duration_ms) {
      printf("[STALKING -> CREEPING] Alvo estavel confirmado!\n");
      creeping_start_dist = distance;
      drive_both(MOTOR_DIRECTION_FORWARD, cfg.creeping_speed);
      enter_state(STATE_CREEPING);
    }
    break;
  }

  // ==========================================================================
  // ESTADO 3: CREEPING (Aproximação Lenta e Trava de Alvo)
  // ==========================================================================
  case STATE_CREEPING: {
    if (distance < 0 || distance > cfg.dist_lost_min_cm) {
      printf("[CREEPING] Alvo perdido durante aproximacao. Voltando para SEARCHING.\n");
      motors_stop_all(&motors);
      enter_state(STATE_SEARCHING);
    } else if ((now_ms - state_start_ms) >= cfg.creeping_duration_ms) {
      // Ao fim dos 0,6s, valida se segue visível e mais perto (ou já encostando <= 10cm)
      // No Hunter exige aproximação real; no Gladiator aceita distância estável ou menor
      bool target_locked = (current_mode == MODE_GLADIATOR)
                             ? (distance <= creeping_start_dist + 5)
                             : (distance < creeping_start_dist || distance <= 10);

      if (is_target_visible(distance, cfg.dist_detect_max_cm) && target_locked) {
        printf("[CREEPING -> CHARGING] Alvo travado a %d cm! Ataque maximo!\n", distance);
        drive_both(MOTOR_DIRECTION_FORWARD, cfg.creeping_speed);
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
  // ESTADO 4: CHARGING (Investida ignorando o ultrassônico)
  // ==========================================================================
  case STATE_CHARGING: {
    // A saída normal ocorre pela interrupção global do LDR ao chegar na borda.
    // Aqui tratamos apenas o timeout de segurança caso a garrafa escape para o lado.
    if ((now_ms - state_start_ms) >= cfg.charging_timeout_ms) {
      printf("[CHARGING] Timeout de %lld ms sem achar borda. Voltando para SEARCHING.\n",
             cfg.charging_timeout_ms);
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
