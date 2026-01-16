# heat_can
CAN bus interface firmware for my Logamatic heating.
This is the firmware of an ESP8266 board with a MCP2515 CAN controller.
It's purpose is to access the CAN bus of a Logamatic heating controller remotely via MQTT.
It's just a wireless CAN interface, no magic here ;)
It contains the [Arduino CAN library](https://github.com/sandeepmistry/arduino-CAN) as a subtree, because it required some patches for my ESP8266 setup.

## Configuration
Edit the following constants in the main `src/main.cpp` file before uploading:
```cpp
const char* ssid = "WIFI_SSID";              // Your WiFi SSID
const char* password = "WIFI_PASSWD";         // Your WiFi password
const char* mqtt_server = "mqtt.example.org"; // MQTT broker address
const int mqtt_port = 1883;                   // MQTT broker port (default: 1883)
const char* mqtt_user = "";                   // MQTT username
const char* mqtt_password = "";               // MQTT password
const char* ota_password = "Donn1EfBu";       // ota password
```

## Pinout
| MCP2515 | ESP8266 |
| :--- | :---- |
| INT | D1 (GPIO5) |
| SCK | D5 (GPIO14) |
| SI | D7 (GPIO13) |
| SO | D6 (GPIO12) |
| CS | D8 (GPIO15) |
| GND | GND |
| VCC | 3V3 |

| Eco-Bus | Function |
| :--- | :---- |
| 1 | GND (cable shield) |
| 2 | CanL |
| 3 | CanH |
