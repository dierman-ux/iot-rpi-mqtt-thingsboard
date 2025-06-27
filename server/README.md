# Server - MQTT Subscriber (Optional)

This optional component subscribes to MQTT topics and processes the data locally. It can be used for local logging, visualization, or forwarding.

## 📦 Structure

- `main.cpp`: Entry point, sets up and runs the subscriber.
- `server.cpp/h`: Logic to connect to MQTT and process messages.
- `Makefile`: Build script.

## ⚙️ Build

```bash
make
```

## 📡 How It Works

1. Connects to the MQTT broker as a subscriber.
2. Listens to a specific topic.
3. Processes incoming JSON messages (e.g., prints or logs them).

## 🔄 Dependencies

- MQTT client libraries (e.g., Paho, Mosquitto)
