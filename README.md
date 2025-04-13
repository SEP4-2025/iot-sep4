# What is this repository

This repository includes the IoT client that interacts with an ATMega2560 and a multitude of sensors. As well as an MQTT client implementation using [MQTTPacket](https://os.mbed.com/teams/mqtt/code/MQTTPacket/) and the driver for the ESP8266 ESP-01 WiFi module to transmit the data.

# How to navigate

The main folder is a PlatformIO project based on the [VIA drivers repository](https://github.com/erlvia/sep4_drivers).
* The `src/main.c` is the main file that contains the main loop and the initialization of the drivers.
* The `lib/drivers` directory is a copy of the drivers in the repository provided by VIA - https://github.com/erlvia/sep4_drivers
* The `lib/mqtt` directory is a copy of all the files in the [MQTTPacket](https://os.mbed.com/teams/mqtt/code/MQTTPacket/) library, which is a C implementation of the MQTT protocol. It is used to serialize MQTT packets for transmission over a network.
