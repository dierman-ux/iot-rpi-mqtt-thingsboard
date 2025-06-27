# IoT Raspberry Pi Project with MQTT and ThingsBoard

This project is a simple IoT system based on a Raspberry Pi, which uses MQTT to send sensor data to the [ThingsBoard](https://thingsboard.io) platform for remote monitoring and telemetry.

## 🧱 Project Structure

```
.
├── client/         # Code running on Raspberry Pi to read sensors and publish data
│   ├── app/
│   ├── client.cpp/.h/.o
│   ├── sensors.cpp/.h/.o
│   ├── main.cpp/.o
│   ├── Makefile
│   └── README.md
├── server/         # Optional component to receive/process MQTT messages
│   ├── main.cpp
│   ├── server.cpp/.h
│   ├── Makefile
│   └── README.md
└── README.md       # This file
```

## 🚀 Features

- MQTT communication between Raspberry Pi and ThingsBoard
- Sensor data acquisition (e.g., temperature, humidity, light)
- Modular C++ code for client and server
- Optional local server to receive and process MQTT messages

## 🛠️ Requirements

- Raspberry Pi with Raspbian/Linux
- MQTT broker (e.g., Mosquitto, or ThingsBoard integrated broker)
- g++ / make
- ThingsBoard account (Community or PE)
- Sensor modules (e.g., TCS34725, MPU6050)

## 🔧 Build Instructions

### Client

```bash
cd client
make
```

### Server (optional)

```bash
cd server
make
```

## 🧪 How It Works

1. The `client` program reads data from connected sensors.
2. It formats the data into JSON.
3. Sends the data to ThingsBoard via MQTT.
4. The `server` (if used) can subscribe to the MQTT topic to receive and log/process the data locally.

## 📄 Documentation

Each component has its own README:

- [`client/README.md`](client/README.md)
- [`server/README.md`](server/README.md)
