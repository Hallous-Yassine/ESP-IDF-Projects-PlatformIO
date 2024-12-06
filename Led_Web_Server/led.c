#include <stdio.h>
#include <sys/param.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include <esp_http_server.h>
#include "my_data.h"

#define LED_PIN GPIO_NUM_2

static char ip_address[16] = {0};  // Buffer to hold the IP address as a string

static const char *TAG = "Websocket Server: ";

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
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
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

// Initialize the LED
void init_led()
{
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}

// Asynchronous response data structure
struct async_resp_arg
{
    httpd_handle_t hd; // Server instance
    int fd;            // Session socket file descriptor for cilent
};

// Serve the HTML page
esp_err_t index_handler(httpd_req_t *req)
{
    const char *html_page =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "    <meta charset=\"UTF-8\">"
        "    <title>ESP32 LED Control</title>"
        "    <style>"
        "        body {"
        "            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"
        "            background-color: #f0f0f0;"
        "            color: #333;"
        "            text-align: center;"
        "            margin: 0;"
        "            padding: 0;"
        "            display: flex;"
        "            flex-direction: column;"
        "            justify-content: center;"
        "            align-items: center;"
        "            height: 100vh;"
        "        }"
        "        h1 {"
        "            color: #007BFF;"
        "        }"
        "        .container {"
        "            background: #fff;"
        "            border-radius: 8px;"
        "            box-shadow: 0 4px 8px rgba(0,0,0,0.1);"
        "            padding: 20px;"
        "            width: 300px;"
        "            margin: 20px;"
        "        }"
        "        button {"
        "            background-color: #007BFF;"
        "            border: none;"
        "            color: white;"
        "            padding: 15px 30px;"
        "            font-size: 16px;"
        "            border-radius: 5px;"
        "            cursor: pointer;"
        "            margin: 10px;"
        "            transition: background-color 0.3s, transform 0.3s;"
        "        }"
        "        button:hover {"
        "            background-color: #0056b3;"
        "            transform: scale(1.05);"
        "        }"
        "        button:active {"
        "            background-color: #003d7a;"
        "        }"
        "    </style>"
        "    <script>"
        "        function toggleLED(state) {"
        "            fetch('/ws', {"
        "                method: 'POST',"
        "                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },"
        "                body: 'led=' + state"
        "            })"
        "            .then(response => response.text())"
        "            .then(data => console.log(data))"
        "            .catch(error => console.error('Error:', error));"
        "        }"
        "    </script>"
        "</head>"
        "<body>"
        "    <div class=\"container\">"
        "        <h1>ESP32 LED Control</h1>"
        "        <button onclick=\"toggleLED('on')\">Turn On</button>"
        "        <button onclick=\"toggleLED('off')\">Turn Off</button>"
        "    </div>"
        "</body>"
        "</html>";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, strlen(html_page));
    return ESP_OK;
}

// The asynchronous response POST
static void generate_async_resp_post(void *arg)
{
    // Data format to be sent from the server as a response to the client
    char http_string[250];
    char *data_string = "POST response from ESP32 websocket server ...";
    sprintf(http_string, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", strlen(data_string));

    // Initialize asynchronous response data structure
    struct async_resp_arg *resp_arg = (struct async_resp_arg *)arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;

    // Send data to the client
    ESP_LOGI(TAG, "Executing queued work POST fd : %d", fd);
    httpd_socket_send(hd, fd, http_string, strlen(http_string), 0);
    httpd_socket_send(hd, fd, data_string, strlen(data_string), 0);

    free(arg);
}

// Initialize a queue for asynchronous communication for POST
esp_err_t async_post_handler(httpd_req_t *req)
{
    // Read data sent by the client
    char content[100];
    size_t recv_size = MIN(req->content_len, sizeof(content));
    httpd_req_recv(req, content, recv_size);
    printf("Data sent by the client: %.*s\n", recv_size, content);

    // Control the LED based on the received data
    if (strstr(content, "led=on"))
    {
        gpio_set_level(LED_PIN, 1);
    }
    else if (strstr(content, "led=off"))
    {
        gpio_set_level(LED_PIN, 0);
    }

    // Generate server response queue
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    resp_arg->hd = req->handle;
    resp_arg->fd = httpd_req_to_sockfd(req);
    ESP_LOGI(TAG, "Queuing work POST fd : %d", resp_arg->fd);
    httpd_queue_work(req->handle, generate_async_resp_post, resp_arg);
    return ESP_OK;
}

// Create URI (Uniform Resource Identifier) handler structure for GET
// for the server which is added to default gateway
static const httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL,
};

// URI handler structure for POST
httpd_uri_t uri_post = {
    .uri = "/ws",
    .method = HTTP_POST,
    .handler = async_post_handler,
    .user_ctx = NULL
};

static void websocket_app_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Registering the uri_handler
        ESP_LOGI(TAG, "Registering URI handler");
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
}

void app_main(void)
{
    wifi_connection();
    init_led();
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    websocket_app_start();
}