#include "server.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <sys/select.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstring>
#include <unistd.h>
#include <iomanip>

struct TCS34725Data { int red, green, blue, ir; float luminance; };
struct MPU6050Data { float ax, ay, az, gx, gy, gz; };

double calculateMean(const std::vector<float>& d) {
    return std::accumulate(d.begin(), d.end(), 0.0) / d.size();
}
double calculateStdDev(const std::vector<float>& d, double mean) {
    double s = 0;
    for (auto v : d) s += (v - mean)*(v - mean);
    return std::sqrt(s / d.size());
}
bool isText(const char* b, int n) {
    for (int i = 0; i < n; ++i)
        if (!isprint(b[i]) && b[i] != '\n' && b[i] != '\r')
            return false;
    return true;
}
void printStatsTable(const std::vector<std::string>& names,
                     const std::vector<std::vector<float>>& vals) {
    std::cout << "----------------------------------------------------------------\n"
              << "| Sensor |   Mean    |   Max     |   Min     |  StdDev   |\n"
              << "----------------------------------------------------------------\n";
    for (size_t i = 0; i < names.size(); ++i) {
        const auto& v = vals[i];
        double m  = calculateMean(v);
        double M  = *std::max_element(v.begin(), v.end());
        double m0 = *std::min_element(v.begin(), v.end());
        double sd = calculateStdDev(v, m);
        std::cout << "| " << std::setw(6) << std::left << names[i]
                  << "| " << std::setw(9) << std::right << std::fixed << std::setprecision(3) << m
                  << "| " << std::setw(9) << M
                  << "| " << std::setw(9) << m0
                  << "| " << std::setw(9) << sd << " |\n";
    }
    std::cout << "----------------------------------------------------------------\n\n";
}

std::pair<std::vector<TCS34725Data>, std::vector<MPU6050Data>>
unpack(const std::vector<char>& pkt) {
    size_t block = sizeof(TCS34725Data) + sizeof(MPU6050Data);
    size_t n = pkt.size() / block;
    std::vector<TCS34725Data> tcs(n);
    std::vector<MPU6050Data> mpu(n);
    std::memcpy(tcs.data(), pkt.data(), n * sizeof(TCS34725Data));
    std::memcpy(mpu.data(), pkt.data() + n * sizeof(TCS34725Data), n * sizeof(MPU6050Data));
    return {tcs, mpu};
}

int main(int argc, char* argv[]) {
    int port    = 8080;
    int calcSec = 60;

    if (argc > 1) { int v = std::stoi(argv[1]); if (v > 0) port = v; else std::cerr<<"Puerto inválido. Usando "<<port<<"\n"; }
    if (argc > 2) { int v = std::stoi(argv[2]); if (v > 0) calcSec = v; else std::cerr<<"Intervalo inválido. Usando "<<calcSec<<"\n"; }

    if (argc <= 1) {
        std::cout << "Puerto UDP [Enter=" << port << "]: ";
        std::string s; std::getline(std::cin, s);
        if (!s.empty()) { int v = std::stoi(s); if (v > 0) port = v; }
    }
    if (argc <= 2) {
        std::cout << "Intervalo cálculo [s] [Enter=" << calcSec << "]: ";
        std::string s; std::getline(std::cin, s);
        if (!s.empty()) { int v = std::stoi(s); if (v > 0) calcSec = v; }
    }

    auto calcInterval = std::chrono::seconds(calcSec);
    Server server(port);
    int fd = server.getSocketFd();

    const std::vector<std::string> names = {
        "Red","Green","Blue","IR","Lumin","AX","AY","AZ","GX","GY","GZ"
    };
    std::vector<std::vector<float>> periodBuf(11);
    auto lastPeriod = std::chrono::steady_clock::now();
    auto nextPeriod = lastPeriod + calcInterval;

    std::cout << "[Server] Arrancado en puerto " << port
              << ". Cálculo cada " << calcSec << " s.\n";

    while (true) {
        auto now = std::chrono::steady_clock::now();

        if (now >= nextPeriod) {
            std::cout << "\n== Estadísticas período de " << calcSec << " s ==\n";
            if (std::all_of(periodBuf.begin(), periodBuf.end(),
                            [](auto& v){ return v.empty(); })) {
                std::cout << "-       NO DATA RECEIVED       -\n\n";
            } else {
                printStatsTable(names, periodBuf);
            }
            for (auto& buf : periodBuf) buf.clear();
            lastPeriod = std::chrono::steady_clock::now();
            nextPeriod = lastPeriod + calcInterval;
            continue;
        }

        auto timeout = std::chrono::duration_cast<std::chrono::microseconds>(nextPeriod - now);
        if (timeout.count() < 0) timeout = std::chrono::microseconds(0);
        struct timeval tv { 
            .tv_sec = static_cast<long>(timeout.count() / 1000000),
            .tv_usec = static_cast<long>(timeout.count() % 1000000)
        };

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        int ret = select(fd+1, &rfds, nullptr, nullptr, &tv);
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        if (ret == 0) continue;

        auto pkt = server.receiveBinaryMessage();
        size_t len = pkt.size();
        if (len > 0 && isText(pkt.data(), len)) {
            auto msg = server.receiveMessage();
            std::cout << "[Server] Texto: " << msg << "\n";
            server.sendReply("ACK: Text");
            continue;
        }

        auto [tcs, mpu] = unpack(pkt);
        std::vector<std::vector<float>> msgBuf(11);
        for (auto& t : tcs) {
            msgBuf[0].push_back(t.red);
            msgBuf[1].push_back(t.green);
            msgBuf[2].push_back(t.blue);
            msgBuf[3].push_back(t.ir);
            msgBuf[4].push_back(t.luminance);
        }
        for (auto& m : mpu) {
            msgBuf[5].push_back(m.ax);
            msgBuf[6].push_back(m.ay);
            msgBuf[7].push_back(m.az);
            msgBuf[8].push_back(m.gx);
            msgBuf[9].push_back(m.gy);
            msgBuf[10].push_back(m.gz);
        }

        std::cout << "[Server] Estadísticas paquete entrante:\n";
        printStatsTable(names, msgBuf);

        for (int i = 0; i < 11; ++i)
            periodBuf[i].insert(periodBuf[i].end(), msgBuf[i].begin(), msgBuf[i].end());

        server.sendReply("ACK: OK");
    }

    return 0;
}
