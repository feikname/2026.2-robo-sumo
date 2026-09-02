// esp_timer_get_time
#include "esp_timer.h"

// esp_rom_delay_us
#include "esp_rom_sys.h"

// gpio_set_direction, gpio_set_level
#include "driver/gpio.h"

// prototype definitions
#include "hcsr04.h"

static void hcsr04_trigger(int trig_pin) {
  gpio_set_level(trig_pin, 1);
  esp_rom_delay_us(10);
  gpio_set_level(trig_pin, 0);
  esp_rom_delay_us(2);
}

static void hcsr04_wait_echo(int echo_pin, int value, int64_t timeout_duration) {
  int64_t deadline = esp_timer_get_time() + timeout_duration;

  while (gpio_get_level(ECHO_GPIO) != value) {
    int64_t now = esp_timer_get_time();
    if (now >= deadline) {
      break;
    }
  }
}

static int hcsr04_to_cm(int64_t elapsed_us) {
  // convert from HC-SR04 echo pin microseconds to a distance in cm
  //
  // the time received is the time for the sound to go and come back.
  //
  // since we're only interested in the time it takes to go, divide
  // by 2.
  //
  // 343m/s is approximate sound of speed
  int distance_cm = (int)(elapsed_us * 0.0343 / 2);

  return distance_cm;
}

void hcsr04_setup(int trig_pin, int echo_pin) {
  gpio_set_direction(trig_pin, GPIO_MODE_OUTPUT);
  gpio_set_direction(echo_pin, GPIO_MODE_INPUT);
}

int hcsr04_read_cm(int trig_pin, int echo_pin, int timeout_duration) {
  hcsr04_trigger(trig_pin);
  
  // we started receiving back the signal
  hcsr04_wait_echo(echo_pin, 1, timeout_duration);

  // time how long for it to be received entirely
  int64_t start = esp_timer_get_time();
  hcsr04_wait_echo(echo_pin, 0, timeout_duration);
  int64_t end = esp_timer_get_time();

  int64_t elapsed = end - start;
  return hcsr04_to_cm(elapsed);
}
