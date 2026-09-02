#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hcsr04.h"

void setup_hc_sr04(int trig_pin, int echo_pin) {
  gpio_set_direction(trig_pin, GPIO_MODE_OUTPUT);
  gpio_set_direction(echo_pin, GPIO_MODE_INPUT);
}

int ler_HC_SR04(int trig_pin, int echo_pin) {
  gpio_set_level(trig_pin, 1);
  esp_rom_delay_us(10);
  gpio_set_level(trig_pin, 0);

  uint64_t inicio_time_out = esp_timer_get_time();
  while (gpio_get_level(echo_pin) == 0) {
    uint64_t tempo_atual = esp_timer_get_time();
    // 25ms
    if ((tempo_atual - inicio_time_out) >= 25000) {
      return -1;
    }
  }

  double inicio_tempo_echo_alto = esp_timer_get_time();

  while (gpio_get_level(echo_pin) == 1) {
  }
  double fim_tempo_echo_alto = esp_timer_get_time();

  // Converter para segundos
  fim_tempo_echo_alto = (double)fim_tempo_echo_alto / 1000000.0;

  // Converter para segundos
  inicio_tempo_echo_alto = (double)inicio_tempo_echo_alto / 1000000.0;

  // A velocidade do som é aprox 343m/s
  int distanciaCm =
      (int)(((fim_tempo_echo_alto - inicio_tempo_echo_alto) * 34300) / 2);

  return distanciaCm;
}
