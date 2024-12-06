#include <stdio.h>
#include <sys/param.h>
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
#include "esp32-dht11.h"
#include "my_data.h"

#define CONFIG_DHT11_PIN GPIO_NUM_4
#define CONFIG_CONNECTION_TIMEOUT 5

static char ip_address[16] = {0};  // Buffer to hold the IP address as a string

static const char *TAG = "Webserver";

// Global variables to store temperature and humidity
static float temperature = 0.0;
static float humidity = 0.0;

// HTML content to serve
const char *index_html = 
"<!DOCTYPE html>"
"<html>"
"<head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>ESP32 Sensor Data</title>"
    "<style>"
        "body {"
            "font-family: Arial, sans-serif;"
            "background-color: #282c34;"
            "color: #ffffff;"
            "margin: 0;"
            "padding: 20px;"
            "display: flex;"
            "justify-content: center;"
            "align-items: center;"
            "height: 100vh;"
        "}"
        ".container {"
            "background-color: #3b3f46;"
            "border-radius: 8px;"
            "padding: 20px;"
            "box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);"
            "width: 100%;"
            "max-width: 600px;"
            "text-align: center;"
        "}"
        "h1 {"
            "color: #61dafb;"
        "}"
        ".data {"
            "font-size: 24px;"
            "margin: 20px 0;"
        "}"
        ".data span {"
            "display: block;"
            "font-size: 18px;"
            "color: #61dafb;"
        "}"
        ".footer {"
            "margin-top: 20px;"
            "font-size: 14px;"
            "color: #888888;"
        "}"
        "@media (max-width: 600px) {"
            "body {"
                "padding: 10px;"
            "}"
            ".container {"
                "padding: 10px;"
            "}"
        "}"
    "</style>"
    "<script>"
        "async function fetchData() {"
            "try {"
                "let response = await fetch('/data');"
                "let data = await response.json();"
                "document.getElementById('temperature').innerText = `Temperature: ${data.temperature} °C`;"
                "document.getElementById('humidity').innerText = `Humidity: ${data.humidity} %`;"
            "} catch (error) {"
                "console.error('Error fetching data:', error);"
            "}"
        "}"
        "setInterval(fetchData, 2000); // Refresh every 2 seconds"
        "window.onload = fetchData; // Initial fetch"
    "</script>"
"</head>"
"<body>"
    "<div class=\"container\">"
        "<h1>ESP32 Sensor Data</h1>"
        "<div id=\"temperature\" class=\"data\">Temperature: -- °C</div>"
        "<div id=\"humidity\" class=\"data\">Humidity: -- %</div>"
        "<div class=\"footer\">Real-time sensor data from ESP32</div>"
    "</div>"
"</body>"
"</html>";

// Wi-Fi event handler
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

// Wi-Fi initialization
void wifi_connection() {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS
        }
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_start();
    esp_wifi_connect();
}

// Asynchronous response data structures and handlers
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

static void generate_async_resp_post(void *arg) {
    char http_string[250];
    char *data_string = "POST response from ESP32 websocket server...";
    snprintf(http_string, sizeof(http_string), "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", strlen(data_string));

    struct async_resp_arg *resp_arg = (struct async_resp_arg *)arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;

    ESP_LOGI(TAG, "Executing queued work POST fd : %d", fd);
    httpd_socket_send(hd, fd, http_string, strlen(http_string), 0);
    httpd_socket_send(hd, fd, data_string, strlen(data_string), 0);

    free(arg);
}

static esp_err_t async_get_handler(httpd_req_t *req) {
    if (strcmp(req->uri, "/") == 0) {
        // Serve HTML page
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, index_html, strlen(index_html));
    } else if (strcmp(req->uri, "/data") == 0) {
        // Serve JSON data
        char data_string[100];
        snprintf(data_string, sizeof(data_string), "{\"temperature\": %.2f, \"humidity\": %.2f}", temperature, humidity);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, data_string, strlen(data_string));
    } else {
        httpd_resp_send_404(req);
    }
    return ESP_OK;
}

static esp_err_t async_post_handler(httpd_req_t *req) {
    char content[100];
    size_t recv_size = MIN(req->content_len, sizeof(content));
    httpd_req_recv(req, content, recv_size);
    printf("Data sent by the client: %.*s\n", recv_size, content);

    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    ESP_LOGI(TAG, "Queuing work POST fd : %d", resp_arg->fd);
    httpd_queue_work(req->handle, generate_async_resp_post, resp_arg);
    return ESP_OK;
}

// URI handler structures
static const httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = async_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t uri_data_get = {
    .uri = "/data",
    .method = HTTP_GET,
    .handler = async_get_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t uri_post = {
    .uri = "/ws",
    .method = HTTP_POST,
    .handler = async_post_handler,
    .user_ctx = NULL,
};

static void websocket_app_start(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_data_get);
        httpd_register_uri_handler(server, &uri_post);
    } else {
        ESP_LOGE(TAG, "Failed to start server!");
    }
}

// DHT11 sensor task
void dht11_task(void *pvParameter) {
    dht11_t dht11_sensor;
    dht11_sensor.dht11_pin = CONFIG_DHT11_PIN;

    while (1) {
        if (!dht11_read(&dht11_sensor, CONFIG_CONNECTION_TIMEOUT)) {
            temperature = dht11_sensor.temperature;
            humidity = dht11_sensor.humidity;
            ESP_LOGI(TAG, "Temperature: %.2f, Humidity: %.2f", temperature, humidity);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    wifi_connection();
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    websocket_app_start();
    xTaskCreate(dht11_task, "dht11_task", 2048, NULL, 5, NULL);
}