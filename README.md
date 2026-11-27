# Robô de Sumô Autônomo (ESP32 / ESP-IDF)

Firmware de controle para robô de sumô autônomo desenvolvido em C utilizando o framework **ESP-IDF** e **FreeRTOS**. O sistema opera por meio de uma **Máquina de Estados Finitos (FSM)** adaptável para diferentes cenários de competição, gerenciando leitura de sensores, controle de tração diferencial e proteção de borda.

---

## Hardware e Periféricos Conectados

O funcionamento do robô baseia-se na integração de três subsistemas principais:

- **Sensor Ultrassônico (`HC-SR04`):** Responsável pela detecção de distância frontal (`TRIG` e `ECHO`, timeout de `25 ms`). Utilizado para varredura, estimativa/medição de largura angular e confirmação de aproximação do alvo. No modo `HUNTER`, o alcance de detecção é até `55 cm` (perdido em `60 cm`); no modo `GLADIATOR`, detecta até `65 cm` (perdido em `75 cm`).
- **Sensor de Refletância (`LDR`):** Instalado na parte inferior voltado para o piso da arena (dojô). Detecta a transição entre o piso preto e a faixa branca delimitadora da borda (configurado com `LDR_WHITE = 0` — ativo-baixo: `0` = branco — e filtro de `3` leituras consecutivas).
- **Conjunto de Motores DC (`motor_set`):** Tração diferencial (motor esquerdo e direito) controlada por direção (`L_IN1`/`L_IN2` e `R_IN1`/`R_IN2`) e modulação de largura de pulso (`LEDC PWM` via `L_PWM` e `R_PWM`), permitindo movimentação linear e giros sobre o próprio eixo.

---

## Modos de Operação

O comportamento estratégico pode ser alternado pela variável `current_mode`:

| Característica | `MODE_HUNTER` (Caçador) | `MODE_GLADIATOR` (Gladiador) |
| :--- | :--- | :--- |
| **Procura (`SEARCHING - SCANNING`)** | Alvos estacionários (ex.: garrafas/obstáculos), até `55 cm`. | Combate 1v1 contra oponente móvel, até `65 cm`. |
| **Mira e Varredura (`SEARCHING - MEASURING`)** | Entra em `SEARCH_MEASURING` e mede a largura angular do objeto para centralizar; ignora paredes largas (`> 1,5 s`). | Pula o subestado de medição estimando o ângulo até o centro de um alvo de `10 cm` ($\arctan(5 / d)$, com distância mínima de `8 cm`) e gira no sentido horário para centralizar a mira. |
| **Confirmação (`STALKING`)** | Aguarda `800 ms` validando estabilidade (`±3 cm`). | Resposta rápida (`100 ms`) com maior tolerância (`±5 cm`). |
| **Aproximação (`CREEPING`)** | Avança devagar (`25%` PWM, `300 ms`) exigindo redução real da distância (ou distância menor ou igual a `10 cm`). | Avança a `50%` PWM (`100 ms`) aceitando alvo estável ou em aproximação (até `+5 cm`). |
| **Ataque (`CHARGING`)** | Investida com timeout curto (`600 ms`) caso não atinja a borda antes. | Investida com timeout maior (`1200 ms`) para sustentar o empurrão contra o oponente. |

---

## Arquitetura e Lógica de Funcionamento

Ao ser energizado (`app_main` -> `setup`), o robô carrega a configuração do modo selecionado, inicializa `HC-SR04`, `LDR` e `motor_set`, e aguarda **5 segundos (`STARTUP_DELAY = 5000 ms`)** em conformidade com as regras de início antes de entrar em `STATE_SEARCHING`. O `loop` roda a cada `64 ms` (`LOOP_DELAY`).

### 1. Proteção Global de Borda (Prioridade Máxima)
A cada ciclo de `64 ms`, o sensor **LDR** é lido antes da máquina de estados. Quando `LDR_WHITE` ocorre por `3` ciclos consecutivos (`LDR_TRIGGER_COUNT`), o bloco de interrupção global pode abortar o estado atual e acionar `STATE_RETREAT` (verifique no código se o `if` está ativo antes de ir para a arena).

### 2. Máquina de Estados (FSM)

```text
[SEARCHING] ──(Alvo detectado/alinhado)──> [STALKING] ──(Alvo estável)──> [CREEPING]
     ^                                         │                              │
     └────────(Alvo perdido/instável)──────────┴─────(Não aproximou)──────────┘
                                                                              │
[RETREAT] <──(Borda branca via LDR)── [CHARGING] <──(Trava confirmada)────────┘

```

### Estados da Máquina de Estados (FSM)

* **`STATE_SEARCHING` (Varredura e Centralização sem Encoders):** Gira continuamente no sentido horário (`MOVE_SPIN_CLOCKWISE`).
  * No `MODE_HUNTER`, ao detectar o alvo (`SEARCH_SCANNING`), muda para `SEARCH_MEASURING` e cronometra o tempo até o feixe sair do objeto (ou ultrapassar `SEARCH_MAX_SWEEP_MS = 1500 ms`, marcando obstáculo largo). Ao sair da borda, para, gira no sentido anti-horário por `(elapsed_ms / 2) + cfg.inertia_compensation_ms` e vai para `STATE_STALKING`.
  * No `MODE_GLADIATOR`, ao detectar a primeira borda, calcula o ângulo até o centro de um alvo de `10 cm` (`5 cm` de raio), converte graus para milissegundos usando `TURN_135_DEG_MS / 135.0f`, gira no sentido horário por esse tempo extra, para e vai para `STATE_STALKING`.
* **`STATE_STALKING` (Confirmação de Alvo):** Mantém os motores parados, registra a distância inicial `D0` e verifica se o alvo permanece visível e dentro de `±cfg.stalking_tolerance_cm` durante `cfg.stalking_duration_ms`. Se confirmar, inicia o avanço lento (`cfg.creeping_speed`) e entra em `STATE_CREEPING`.
* **`STATE_CREEPING` (Aproximação Controlada):** Avança por `cfg.creeping_duration_ms` monitorando o `HC-SR04`. Ao término, valida se o alvo continua visível e se houve aproximação conforme o modo (`MODE_GLADIATOR` ou `MODE_HUNTER`). Se aprovado, entra em `STATE_CHARGING` (nota: no código atual, a chamada ao entrar em `STATE_CHARGING` passa `cfg.creeping_speed`; ajuste para `cfg.charging_speed` se quiser acelerar nessa transição).
* **`STATE_CHARGING` (Investida):** Avança em direção ao alvo **ignorando o `HC-SR04`** para evitar leituras falsas na zona cega (`2–3 cm`) durante o contato. A saída normal ocorre pela detecção da borda branca via `LDR` (`STATE_RETREAT`), ou pelo timeout de segurança (`cfg.charging_timeout_ms`: `600 ms` no `HUNTER` e `1200 ms` no `GLADIATOR`), que para os motores e retorna para `STATE_SEARCHING`.
* **`STATE_RETREAT` (Fuga da Borda e Giro para o Centro):** Executa até `4` pulsos de ré (`RETREAT_PULSE_MS = 150 ms`) com pausa de `20 ms` para estabilização até o `LDR` deixar de ler `LDR_WHITE`. Se confirmar piso preto, recua `RETREAT_MARGIN_MS` (`80 ms`), gira no sentido horário por `TURN_135_DEG_MS` (`450 ms`, ~`135°` / calibrável) para apontar de volta à arena, zera `ldr_white_count` e volta a `STATE_SEARCHING`. Caso continue no branco após `4` tentativas, trava os motores por segurança para evitar queda de costas.

---

## Otimizações de Sensores e Controle

* **Prevenção de Interferência Acústica:** O `LOOP_DELAY` de `64 ms` reduz ecos residuais do `HC-SR04` entre medições, e o sensor é pausado em `STATE_CHARGING` e `STATE_RETREAT`.
* **Compensação sem Encoders:** Parâmetros como `inertia_compensation_ms`, `TURN_135_DEG_MS` e `LDR_TRIGGER_COUNT` permitem calibrar deslizes mecânicos e ruídos de leitura diretamente nos testes na arena.