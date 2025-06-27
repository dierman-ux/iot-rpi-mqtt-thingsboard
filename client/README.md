# Client - Raspberry Pi Sensor Publisher

This component runs on a Raspberry Pi and is responsible for reading data from sensors and sending it to the MQTT broker (ThingsBoard).

## Structure

- `main.cpp`: Entry point, handles main loop and publishing.
- `client.cpp/h`: MQTT connection and publishing logic.
- `sensors.cpp/h`: Interfaces with physical sensors (e.g., TCS34725, MPU6050).
- `app/`: May contain additional resources or configurations.
- `Makefile`: Build instructions.

## Build

```bash
make
```

## How It Works

1. Initializes sensors and MQTT client.
2. Reads sensor data periodically.
3. Converts data to JSON format.
4. Publishes to a configured MQTT topic (e.g., `v1/devices/me/telemetry` for ThingsBoard).

## Dependencies

- MQTT client libraries (e.g., Paho, Mosquitto)
- WiringPi or I2C libraries for sensor communication

