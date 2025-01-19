# Introduction
This project aims to create a custom device with esp8266 based microcontroller to track floor heating system's inlet- and outlet temperatures and allow to control the heating system's temperature dropping functionality.
The device can be integrated to any homeautomation system with MQTT capabilities.

- Reads two DS18B20 sensors
- Reports sensor values to selected MQTT topics
- Controls external components via relay, controllable with MQTT
- Display to show errors and status values
- LED to indicate statuses with guick glance
- Web UI for configuring the device, WiFi AP / WLAN
- Intended to integrate into Home Assistant

# Project structure
Project is a normal Arduino/ESP8266 created in VSCode PlatformIO extension - so the project structure differs a bit from traditional program created with Arduino IDE.

1) Install VSCode
2) Install PlatformIO extension
3) Clone the repository
4) Access the project through PlatformIO extension
5) Upload the program to device with PlatformIO
6) Upload the static files in Data directory to device SPIFFS with PlatformIO

The Data -directory contains all static files to be saved in the SPIFFS.

# Hardware
This is just an example of the bill of materials for the project. With small changes you can adapt your own hardware. List of hardware used in this project:
- Wemos D1 Mini
- 0.96" OLED I2C generic display
- 2x DS18B20 temperature sensors
- Relay board with 3.3V coil voltage
- WS1812B LED
- 3D printed case
- Cable glands
- Dupont wires
- Preferably Home Assistant or similar homeautomation system

### Case
The case has been designed with SolidWorks and needed STL & 3MF files are published in MakerWorld community
[Wemos D1 Mini project enclosure](https://makerworld.com/en/models/946838#profileId-913814)
