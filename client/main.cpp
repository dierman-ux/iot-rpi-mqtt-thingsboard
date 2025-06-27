#include "client.h"
#include "sensors.h"
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstdlib>       // for std::system

// ThingsBoard MQTT parameters
const std::string accessToken = "pHcVuKsljZkP31NPm9rc";
const int         mqttPort    = 1883;
const std::string mqttTopic   = "v1/devices/me/telemetry";

static bool running = true;
void handle_sigint(int) { running = false; }

// Convertir datos TCS34725 a binario
std::vector<char> convertTCSDataToBinary(const std::vector<TCS34725Data>& tcsArray) {
    std::vector<char> bin;
    for (auto& d : tcsArray) {
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.red),
                   reinterpret_cast<const char*>(&d.red)   + sizeof(d.red));
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.green),
                   reinterpret_cast<const char*>(&d.green) + sizeof(d.green));
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.blue),
                   reinterpret_cast<const char*>(&d.blue)  + sizeof(d.blue));
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.ir),
                   reinterpret_cast<const char*>(&d.ir)    + sizeof(d.ir));
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.luminance),
                   reinterpret_cast<const char*>(&d.luminance) + sizeof(d.luminance));
    }
    return bin;
}

// Convertir datos MPU6050 a binario
std::vector<char> convertMPUDataToBinary(const std::vector<MPU6050Data>& mpuArray) {
    std::vector<char> bin;
    for (auto& d : mpuArray) {
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.ax),
                   reinterpret_cast<const char*>(&d.ax) + sizeof(d.ax));
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.ay),
                   reinterpret_cast<const char*>(&d.ay) + sizeof(d.ay));
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.az),
                   reinterpret_cast<const char*>(&d.az) + sizeof(d.az));
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.gx),
                   reinterpret_cast<const char*>(&d.gx) + sizeof(d.gx));
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.gy),
                   reinterpret_cast<const char*>(&d.gy) + sizeof(d.gy));
        bin.insert(bin.end(),
                   reinterpret_cast<const char*>(&d.gz),
                   reinterpret_cast<const char*>(&d.gz) + sizeof(d.gz));
    }
    return bin;
}

// Crear el JSON a enviar
std::string createJsonPayload(const TCS34725Data& t, const MPU6050Data& m) {
    std::ostringstream o;
    o << "{\"red\":"        << t.red
      << ",\"green\":"      << t.green
      << ",\"blue\":"       << t.blue
      << ",\"ir\":"         << t.ir
      << ",\"luminance\":"  << t.luminance
      << ",\"ax\":"         << m.ax
      << ",\"ay\":"         << m.ay
      << ",\"az\":"         << m.az
      << ",\"gx\":"         << m.gx
      << ",\"gy\":"         << m.gy
      << ",\"gz\":"         << m.gz
      << "}";
    return o.str();
}

// Publicar JSON usando mosquitto_pub CLI
void sendDataToThingsBoard(const std::string& jsonData, std::string ip ) {
    std::string cmd = 
        "mosquitto_pub -q 1"
        " -h " + ip +
        " -p " + std::to_string(mqttPort) +
        " -t \"" + mqttTopic + "\"" +
        " -u \"" + accessToken + "\"" +
        " -m '" + jsonData + "'";
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "Error: mosquitto_pub failed with code " << ret << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, handle_sigint);

    // Parámetros UDP
    std::string ip       = "192.168.1.151";
    int serverPort       = 8080;
    int numReadings      = 10;
    int tReadings        = 1;
    if (argc > 1) ip = argv[1];
    if (argc > 2) { int v = std::stoi(argv[2]); if (v > 0) serverPort = v; }
    if (argc > 3) { int v = std::stoi(argv[3]); if (v > 0) numReadings = v; }
    if (argc > 4) { int v = std::stoi(argv[4]); if (v > 0) tReadings = v; }

    // Lectura interactiva si falta algún argumento
    if (argc <= 1) {
        std::cout << "Introduce la IP del servidor UDP (Enter=" << ip << "): ";
        std::string s; std::getline(std::cin, s);
        if (!s.empty()) ip = s;
    }
    if (argc <= 2) {
        std::cout << "Introduce el puerto UDP (Enter=" << serverPort << "): ";
        std::string s; std::getline(std::cin, s);
        if (!s.empty()) {
            int v = std::stoi(s);
            if (v > 0) serverPort = v;
        }
    }
    if (argc <= 3) {
        std::cout << "Introduce las iteraciones por envío (Enter=" << numReadings << "): ";
        std::string s; std::getline(std::cin, s);
        if (!s.empty()) {
            int v = std::stoi(s);
            if (v > 0) numReadings = v;
        }
    }
    if (argc <= 4) {
        std::cout << "Introduce el tiempo entre lecturas [s] (Enter=" << tReadings << "): ";
        std::string s; std::getline(std::cin, s);
        if (!s.empty()) {
            int v = std::stoi(s);
            if (v > 0) tReadings = v;
        }
    }

    // Inicialización
    Client client(ip, serverPort);
    int fdTCS = initTCS34725();
    int fdMPU = initMPU6050();

    std::vector<TCS34725Data> tcsBuf;
    std::vector<MPU6050Data>  mpuBuf;
    std::vector<TCS34725Data> instcsBuf;
    std::vector<MPU6050Data>  insmpuBuf;

    // Bucle principal
    while (running) {
        for (int i = 0; i < numReadings && running; ++i) {
            TCS34725Data tcsData = readTCS34725(fdTCS);
            MPU6050Data  mpuData = readMPU6050(fdMPU);

            tcsBuf.push_back(tcsData);
            mpuBuf.push_back(mpuData);
            instcsBuf.push_back(tcsData);
            insmpuBuf.push_back(mpuData);

            // Publicar JSON vía mosquitto_pub
            std::string json = createJsonPayload(instcsBuf.back(), insmpuBuf.back());
            sendDataToThingsBoard(json, ip);

            std::this_thread::sleep_for(std::chrono::seconds(tReadings));
        }

        // Enviar binario al servidor UDP
        auto binT = convertTCSDataToBinary(tcsBuf);
        auto binM = convertMPUDataToBinary(mpuBuf);
        binT.insert(binT.end(), binM.begin(), binM.end());
        client.sendBinaryMessage(binT);
        client.receiveMessage();

        tcsBuf.clear();
        mpuBuf.clear();
        instcsBuf.clear();
        insmpuBuf.clear();
    }

    closeSensor(fdTCS);
    closeSensor(fdMPU);
    return 0;
}
