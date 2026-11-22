
// esp_timer_get_time
#include "esp_timer.h"

// esp_rom_delay_us
#include "esp_rom_sys.h"

// gpio_set_direction, gpio_set_level, gpio_get_level
#include "driver/gpio.h"

// bool
#include <stdbool.h>

// prototype definitions
#include "hcsr04.h"

hcsr04_struct hcsr04_create(const gpio_num_t trigger_pin,
                            const gpio_num_t echo_pin,
                            const int timeout_duration) {
  hcsr04_struct hcsr04 = {
      .trigger_pin = trigger_pin,
      .echo_pin = echo_pin,
      .timeout_duration = timeout_duration,
  };

  return hcsr04;
}

static void hcsr04_trigger(const gpio_num_t trig_pin) {
  gpio_set_level(trig_pin, 0);
  esp_rom_delay_us(2);

  gpio_set_level(trig_pin, 1);
  esp_rom_delay_us(10);

  gpio_set_level(trig_pin, 0);
}

static bool hcsr04_wait_echo(const int echo_pin, const int value,
                             const int64_t deadline) {
  while (gpio_get_level(echo_pin) != value) {
    if (esp_timer_get_time() >= deadline) {
      return false;
    }
  }

  return true;
}

static int hcsr04_to_cm(const int64_t elapsed_us) {
  // Sound travels at approximately 343 m/s.
  // The measured pulse represents the round trip,
  // so divide by 2 to obtain the actual distance.

  return (int)(elapsed_us * 0.0343 / 2);
}

void hcsr04_setup(hcsr04_struct *hcsr04) {
  gpio_set_direction(hcsr04->trigger_pin, GPIO_MODE_OUTPUT);
  gpio_set_direction(hcsr04->echo_pin, GPIO_MODE_INPUT);

  gpio_set_level(hcsr04->trigger_pin, 0);
}

int hcsr04_read_cm(hcsr04_struct *hcsr04) {
  const int trig_pin = hcsr04->trigger_pin;
  const int echo_pin = hcsr04->echo_pin;
  const int timeout_duration = hcsr04->timeout_duration;

  // Ensure ECHO is LOW before starting a new measurement.
  int64_t deadline = esp_timer_get_time() + timeout_duration;

  if (!hcsr04_wait_echo(echo_pin, 0, deadline)) {
    return -1;
  }

  // Send the 10 us trigger pulse.
  hcsr04_trigger(trig_pin);

  // Wait for ECHO to become HIGH.
  deadline = esp_timer_get_time() + timeout_duration;

  if (!hcsr04_wait_echo(echo_pin, 1, deadline)) {
    return -1;
  }

  // Start measuring the ECHO HIGH pulse.
  int64_t start = esp_timer_get_time();

  // Wait for ECHO to become LOW.
  deadline = start + timeout_duration;

  if (!hcsr04_wait_echo(echo_pin, 0, deadline)) {
    return -1;
  }

  int64_t end = esp_timer_get_time();

  // Convert pulse duration into centimeters.
  return hcsr04_to_cm(end - start);
}
