#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_14

void setup_pushbutton()  {
  gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLDOWN_ONLY);
}

int pushbutton_is_pressed() {
  return gpio_get_level(BUTTON_PIN);
}

void pushbutton_waitpress( ) {
  while (true) {
      if (pushbutton_is_pressed()) {
        break;
      }

       vTaskDelay(pdMS_TO_TICKS(50));
    }
  }

void wait_system_start(bool *systemActive) {
  pushbutton_waitpress();

  printf("Starting system in 5 seconds...\n");
  vTaskDelay(pdMS_TO_TICKS(5000));
  *systemActive = true;
  printf("on\n");
}