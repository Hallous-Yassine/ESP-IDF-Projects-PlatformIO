#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#define BLINK_GPIO GPIO_NUM_2  // Pin where the LED is connected

void app_main(void) {
    // Initialize the GPIO
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(BLINK_GPIO, 1);  // Turn the LED on
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 1 second
        gpio_set_level(BLINK_GPIO, 0);  // Turn the LED off
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 1 second
    }
}