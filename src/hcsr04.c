// esp_timer_get_time
#include "esp_timer.h"

// esp_rom_delay_us
#include "esp_rom_sys.h"

// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// prototype definitions
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

  int64_t inicio_tempo_echo_alto = esp_timer_get_time();
  while (gpio_get_level(echo_pin) != 0) {
    // wait for it to go down again
  }
  int64_t fim_tempo_echo_alto = esp_timer_get_time();

  int64_t duracao_us = fim_tempo_echo_alto - inicio_tempo_echo_alto;

  int distancia_cm = (int)(duracao_us * 0.0343 / 2);

  return distancia_cm;
}
