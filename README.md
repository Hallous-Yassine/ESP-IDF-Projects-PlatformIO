# ESP-IDF Projects with PlatformIO

This repository contains various projects developed using the ESP-IDF framework and PlatformIO, demonstrating different functionalities of the ESP32 microcontroller. Each project showcases the capabilities of the ESP32, from basic GPIO operations to setting up web servers for sensor data and device control.

## Projects

### 1. **Blink Code with PlatformIO**
   **Objective:** Demonstrates basic GPIO functionality by blinking an LED. The ESP32 toggles the LED state in a loop with a specified delay.

### 2. **Temperature and Humidity Display with DHT11**
   **Objective:** Interfaces the ESP32 with a DHT11 sensor to read temperature and humidity data. Displays the readings on the serial monitor and through a web server hosted on the ESP32.

### 3. **ESP32 HTTP Server with WiFi Connection and Dynamic IP Display**
   **Objective:** Sets up a basic HTTP server on the ESP32, displaying a "Hello, World!" message along with the device’s dynamic IP address on a web page. Connects to WiFi and shows the IP both on the page and serial monitor.

### 4. **LED Control via Web Server on ESP32**
   **Objective:** Creates a web server to control an LED connected to the ESP32. The web interface allows users to turn the LED on or off by sending GET and POST requests to the server.

### 5. **ESP32 Web Server to Display Real-Time Temperature and Humidity Data**
   **Objective:** Establishes a web server that reads temperature and humidity data from a DHT11 sensor and displays it in real-time on a web page. The page refreshes periodically to show updated data.

