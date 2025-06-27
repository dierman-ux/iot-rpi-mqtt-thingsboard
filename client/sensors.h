#ifndef SENSORS_H
#define SENSORS_H

#include <string>

struct TCS34725Data {
	int  red;
	int  green;
	int  blue;
	int  ir;
    float luminance;
};

struct MPU6050Data {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
};

int initTCS34725();
TCS34725Data readTCS34725(int file);

int initMPU6050();
MPU6050Data readMPU6050(int file);

void closeSensor(int file);
#endif
