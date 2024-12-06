# ESP-IDF Projects with PlatformIO

Welcome to the ESP-IDF Projects repository. This collection features various projects developed with the ESP32 microcontroller using PlatformIO and the ESP-IDF framework. Each project is designed to showcase different capabilities and use cases of the ESP32, providing a practical foundation for learning and development.

## Projects Overview

### 1. **Blink Code with PlatformIO**
A fundamental project that demonstrates the basic operation of the ESP32 by blinking an LED connected to a GPIO pin. This simple yet effective project serves as an introductory exercise for setting up an ESP32 project with PlatformIO and understanding the core structure of an ESP-IDF application.

**Key Features:**
- Basic GPIO control for LED blinking.
- Introduction to ESP-IDF and PlatformIO project setup.
- Essential for beginners to understand microcontroller programming.

### 2. **Temperature and Humidity Display with DHT11**
This project interfaces with a DHT11 sensor to measure temperature and humidity. The collected data is then displayed on an OLED screen, providing real-time environmental monitoring. This project demonstrates sensor integration and I2C communication with peripherals.

**Key Features:**
- Sensor data acquisition from the DHT11 sensor.
- Displaying data on an OLED screen via I2C.
- Real-time data updates and sensor interaction.

### 3. **ESP32 HTTP Server with WiFi Connection and Dynamic IP Display**
Set up an HTTP server on the ESP32 to serve a web page displaying the device’s dynamically assigned IP address. This project involves establishing a WiFi connection, setting up an HTTP server, and handling network-related tasks. The project also outputs the IP address to the serial monitor for easy access.

**Key Features:**
- Establishing a WiFi connection and handling IP assignments.
- Setting up an HTTP server to serve dynamic content.
- Serial monitoring of network details and web server operations.

### 4. **Control LED via Web Server on ESP32**
This project enables control of an LED through a web server hosted on the ESP32. By connecting the ESP32 to a WiFi network, users can interact with a web interface to toggle the LED state. This project highlights web server setup, HTTP request handling, and real-time hardware control via a browser.

**Key Features:**
- Web server implementation for remote hardware control.
- Handling HTTP GET and POST requests for LED operations.
- Interactive web interface for real-time control.

### 5. **ESP32 Web Server for Real-time Temperature and Humidity Data Display**
Build a web server on the ESP32 that continuously reads and displays temperature and humidity data from a DHT11 sensor. The web page, served by the ESP32, updates in real-time, providing users with live environmental data. This project showcases data streaming and web-based visualization.

**Key Features:**
- Real-time data acquisition and display using a web server.
- Integration with the DHT11 sensor for environmental monitoring.
- Dynamic content updates using JavaScript for live data visualization.

## Getting Started

### Prerequisites
- **PlatformIO IDE**: For managing and building the projects.
- **ESP-IDF Framework**: Required for ESP32 development.
- **Basic Knowledge**: Familiarity with C programming and microcontroller concepts is beneficial.

### Setting Up
1. Open the project in PlatformIO IDE.
2. Navigate to the project folder of interest.
3. Build and upload the code to your ESP32 board using PlatformIO.

### Running the Projects
Upon uploading the code, monitor the output using PlatformIO's serial monitor. For projects involving web servers, ensure that your ESP32 is connected to the same network as your computer, and access the provided IP address through a web browser.

## Contributing
Contributions are welcome! If you have suggestions for new projects or improvements, please fork the repository and submit a pull request.

## License
This repository is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.
