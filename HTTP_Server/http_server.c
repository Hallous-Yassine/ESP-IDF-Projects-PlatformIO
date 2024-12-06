#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include <esp_http_server.h>
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "my_data.h"

static char ip_address[16] = {0};  // Buffer to hold the IP address as a string

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        snprintf(ip_address, sizeof(ip_address), IPSTR, IP2STR(&event->ip_info.ip));
        printf("IP Address: %s\n", ip_address);  // Print IP address to the serial monitor
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    nvs_flash_init();
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation
    esp_event_loop_create_default();     // event loop
    esp_netif_create_default_wifi_sta(); // WiFi station
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

static esp_err_t get_handler(httpd_req_t *req)
{
    char html_response[512];
    snprintf(html_response, sizeof(html_response),
             "<!DOCTYPE html>"
             "<html>"
             "<head>"
             "<title>ESP32 Web Server</title>"
             "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
             "<style>"
             "body { font-family: Arial; text-align: center; margin-top: 50px; }"
             "</style>"
             "</head>"
             "<body>"
             "<h1>Hello, World!</h1>"
             "<p>Welcome to the ESP32 Web Server.</p>"
             "<p>IP Address: %s</p>"
             "</body>"
             "</html>", ip_address);
    
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Create URI (Uniform Resource Identifier)
// for the server which is added to default gateway
static const httpd_uri_t uri_handler = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL,
};

static void http_server_app_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_start(&server, &config);
    httpd_register_uri_handler(server, &uri_handler);
}

void app_main(void)
{
    wifi_connection();
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    http_server_app_start();
}