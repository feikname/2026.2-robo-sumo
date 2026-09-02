#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define TRIG_GPIO 23
#define ECHO_GPIO 22

void setup_hc_sr04(int trig_pin, int echo_pin);
int ler_HC_SR04(int trig_pin, int echo_pin);

void setup_hc_sr04(int trig_pin, int echo_pin) {
  // Define o pino TRIG como saída
  gpio_set_direction(trig_pin, GPIO_MODE_OUTPUT);
  // Define o pino ECHO como entrada
  gpio_set_direction(echo_pin, GPIO_MODE_INPUT);
}

int ler_HC_SR04(int trig_pin, int echo_pin) {
  gpio_set_level(echo_pin, 0); // Limpa o pino ECHO
  gpio_set_level(trig_pin, 1); // Gera um pulso curto no pino TRIG para acionar o sensor
  esp_rom_delay_us(10); // Aguarda 10 microsegundos
  gpio_set_level(trig_pin, 0); // Retorna o TRIG para zero
  // O primeiro while aguarda até que o pino ECHO vá para alto (sinalizando que o sensor detectou o eco).
  uint64_t inicio_time_out = esp_timer_get_time(); // O tempo está em microsegundos
  while (gpio_get_level(echo_pin) == 0) {
    uint64_t tempo_atual = esp_timer_get_time(); // O tempo está em microsegundos
    // significa que não houve sinal de retorno
    if ((tempo_atual - inicio_time_out) >= 25000)
    { // 25ms
      return -1;
    }
  }
  // Inicia a contagem do tempo quando o pino do echo recebe um dado.
  double inicio_tempo_echo_alto = esp_timer_get_time(); // O tempo está em microsegundos
  /*
    => Segundo while (Esperando o pino ECHO ficar baixo):
    O segundo while aguarda até que o pino ECHO vá para baixo.
    O tempo no início deste segundo while é usado como o término da medição.
  */
  while (gpio_get_level(echo_pin) == 1)
    ; // enquando ainda estiver recebendo o echo de resposta
  // Finaliza a contagem do tempo quando o pino do echo volta ser zero.
  double fim_tempo_echo_alto = esp_timer_get_time(); // O tempo está em microsegundos
  // Calcule a distância em centímetros
  // Distância (cm) = (Tempo (segundos) * Velocidade do Som no Ar em cm/s) / 2
  fim_tempo_echo_alto = (double)fim_tempo_echo_alto / 1000000.0; // Converter para segundos
  inicio_tempo_echo_alto = (double)inicio_tempo_echo_alto / 1000000.0; // Converter para segundos
  int distanciaCm = (int)(((fim_tempo_echo_alto - inicio_tempo_echo_alto) * 34300) / 2);
  return distanciaCm;
}
