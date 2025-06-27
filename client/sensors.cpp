#include "sensors.h"
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int initTCS34725(){
    int file;
    const char *bus = "/dev/i2c-1";

    if ((file = open(bus, O_RDWR)) < 0) {
        std::cerr << "Error al abrir el bus I2C para TCS34725" << std::endl;
        std::exit(1);
    }

    if (ioctl(file, I2C_SLAVE, 0x29) < 0) {
        std::cerr << "Error al conectar con TCS34725" << std::endl;
        close(file);
        std::exit(1);
    }

    // Configuración inicial del sensor (se hace una sola vez)
    char config[2];
    config[0] = 0x80; // Registro ENABLE
    config[1] = 0x03; // Encender el sensor y habilitar RGBC
    if (write(file, config, 2) != 2) {
        std::cerr << "Error al escribir en el registro ENABLE de TCS34725" << std::endl;
        close(file);
        std::exit(1);
    }

    config[0] = 0x81; // Registro ALS (tiempo de integración)
    config[1] = 0x00; // 700 ms de integración
    if (write(file, config, 2) != 2) {
        std::cerr << "Error al escribir en el registro ALS de TCS34725" << std::endl;
        close(file);
        std::exit(1);
    }

    config[0] = 0x83; // Registro WTIME (tiempo de espera)
    config[1] = 0xFF; // 2.4 ms
    if (write(file, config, 2) != 2) {
        std::cerr << "Error al escribir en el registro WTIME de TCS34725" << std::endl;
        close(file);
        std::exit(1);
    }

    config[0] = 0x8F; // Registro de control (ganancia)
    config[1] = 0x00; // AGAIN = 1x
    if (write(file, config, 2) != 2) {
        std::cerr << "Error al escribir en el registro de control de TCS34725" << std::endl;
        close(file);
        std::exit(1);
    }
    usleep(100000);  // Espera breve para la estabilización

    return file;

}

TCS34725Data readTCS34725(int file) {
    TCS34725Data data;
    // Leer datos
    char reg = 0x94;
    if (write(file, &reg, 1) != 1) {
        std::cerr << "Error al escribir la dirección de registro para leer TCS34725" << std::endl;
        close(file);
        std::exit(1);
    }

    char rawData[8];
    if (read(file, rawData, 8) != 8) {
        std::cerr << "Error al leer datos de TCS34725" << std::endl;
        close(file);
        std::exit(1);
    }

    // Convertir los datos leídos:
    data.red = (rawData[3] << 8) | rawData[2];
    data.green = (rawData[5] << 8) | rawData[4];
    data.blue = (rawData[7] << 8) | rawData[6];
    data.ir = (rawData[1] << 8) | rawData[0];

    // Calcular la luminancia ambiental
    data.luminance = (-0.32466f * data.red) + (1.57837f * data.green) - (0.73191f * data.blue);
    if (data.luminance < 0) {
        data.luminance = 0;
    }

    return data;
}



int initMPU6050(){
    int file;
    const char *bus = "/dev/i2c-1";

    if ((file = open(bus, O_RDWR)) < 0) {
        std::cerr << "Error al abrir el bus I2C para MPU6050" << std::endl;
        std::exit(1);
    }

    if (ioctl(file, I2C_SLAVE, 0x68) < 0) {
        std::cerr << "Error al conectar con MPU6050" << std::endl;
        close(file);
        std::exit(1);
    }

    char writeBuffer[2] = {0x6B, 0x00};  // Despertar el MPU6050
    if (write(file, writeBuffer, 2) != 2) {
        std::cerr << "Error al despertar el MPU6050" << std::endl;
        close(file);
        std::exit(1);
    }

    usleep(100000);  // Espera breve para la estabilización
    return file;
}
MPU6050Data readMPU6050(int file) {
    MPU6050Data data;


    const float ACCEL_SENSITIVITY = 16384.0f;
    const float GYRO_SENSITIVITY = 131.0f;

    // Leer datos del acelerómetro
    char reg = 0x3B;
    if (write(file, &reg, 1) != 1) {
        std::cerr << "Error al escribir la dirección de registro para el acelerómetro" << std::endl;
        close(file);
        std::exit(1);
    }
    char rawData[6];
    if (read(file, rawData, 6) != 6) {
        std::cerr << "Error al leer datos del acelerómetro" << std::endl;
        close(file);
        std::exit(1);
    }
    int16_t ax = (rawData[0] << 8) | rawData[1];
    int16_t ay = (rawData[2] << 8) | rawData[3];
    int16_t az = (rawData[4] << 8) | rawData[5];

    data.ax = ax / ACCEL_SENSITIVITY;
    data.ay = ay / ACCEL_SENSITIVITY;
    data.az = az / ACCEL_SENSITIVITY;

    // Leer datos del giroscopio
    reg = 0x43;
    if (write(file, &reg, 1) != 1) {
        std::cerr << "Error al escribir la dirección de registro para el giroscopio" << std::endl;
        close(file);
        std::exit(1);
    }
    if (read(file, rawData, 6) != 6) {
        std::cerr << "Error al leer datos del giroscopio" << std::endl;
        close(file);
        std::exit(1);
    }
    int16_t gx = (rawData[0] << 8) | rawData[1];
    int16_t gy = (rawData[2] << 8) | rawData[3];
    int16_t gz = (rawData[4] << 8) | rawData[5];

    data.gx = gx / GYRO_SENSITIVITY;
    data.gy = gy / GYRO_SENSITIVITY;
    data.gz = gz / GYRO_SENSITIVITY;

    return data;
}

void closeSensor(int file){
    close(file);
}
