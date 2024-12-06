#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "esp32-dht11.h"

#define CONFIG_DHT11_PIN GPIO_NUM_4
#define CONFIG_CONNECTION_TIMEOUT 5

void app_main() {
    dht11_t dht11_sensor;
    dht11_sensor.dht11_pin = CONFIG_DHT11_PIN;

    // Read data
    while(1)
    {
      if(!dht11_read(&dht11_sensor, CONFIG_CONNECTION_TIMEOUT))
      {  
        printf("[Temperature]> %.2f \n",dht11_sensor.temperature);
        printf("[Humidity]> %.2f \n",dht11_sensor.humidity);
      }
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    } 
}
