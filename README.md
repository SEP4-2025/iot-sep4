# What is this repository

This repository is the IoT repository for the SEP4 project at VIA. It includes the IoT client that interacts with an ATMega2560 and a multitude of sensors. As well as an MQTT client implementation using [MQTTPacket](https://os.mbed.com/teams/mqtt/code/MQTTPacket/) and the driver for the ESP8266 ESP-01 WiFi module to transmit the data.

# How to navigate

## Embedded C - sep4_drivers 

The main folder is a PlatformIO project based on the [VIA drivers repository](https://github.com/erlvia/sep4_drivers).
* The `src/main.c` is the main file that contains the main loop and the initialization of the drivers.
* The `lib/drivers` directory is a copy of the drivers in the repository provided by VIA - https://github.com/erlvia/sep4_drivers
* The `lib/mqtt` directory is a copy of all the files in the [MQTTPacket](https://os.mbed.com/teams/mqtt/code/MQTTPacket/) library, which is a C implementation of the MQTT protocol. It is used to serialize MQTT packets for transmission over a network.

## mosquitto broker

The `mosquitto.docker-compose.yaml` is a docker-compose file that creates a Mosquitto MQTT broker. It uses the [official Mosquitto image](https://hub.docker.com/_/eclipse-mosquitto) from Docker Hub and exposes the MQTT on port (1883). The `mosquitto` folder is mounted to the container for `log`, `data` and `conf`. The `conf/mosquitto.conf` is the main configuration. By default it is configured to accept any connection without any form of authentication.

## node MQTT client

The `mqtt-node-client` folder contains a Node.js MQTT client that subscribes to the topics published by the ATMega2560. It uses the [mqtt](https://www.npmjs.com/package/mqtt) library to connect to the Mosquitto broker and subscribe to the topics.

# How to run

* You need to have Docker installed on your machine. You can run the Mosquitto broker using the following command:

    ```bash
    docker-compose -f mosquitto.docker-compose.yaml up
    ```
* Then you can run the Node.js MQTT client using the following command:

    ```bash
    cd mqtt-node-client
    npm install
    node index.js
    ```

* This will install the required dependencies and run the client. You should see a log in the container that the mqttjs client is connected.

    Finally you should have the ATMega2560 connected to your computer and the PlatformIO project open in VSCode (you should open VSCode in `sep4_drivers` as the root). 

    Once you have it open you need to navigate to the `main.c` file and change:

    * The wifi credentials in the `wifi_command_join_AP()` to your wifi credentials - SSID and Password
    * The broker host and port in the `wifi_command_create_TCP_connection()` function.

    After that you upload the code to the ATMega2560 and open the serial monitor. You should see the connection to the broker and the data being published to the topics. The Node.js client should also receive the data and print it to the console.
    In place of the node client you can use any client that supports MQTT. You see a lot of clients for almost any programming language [here](https://mqtt.org/software/). Just make sure to subscribe to the topics that are being published by the ATMega2560.

