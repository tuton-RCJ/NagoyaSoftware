#ifndef MPU6050_H
#define MPU6050_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

class MPU6050
{
    public:
        MPU6050(TwoWire *wire);
        void init();
        void read();
        void getValues(int *values);
    private:
        Adafruit_MPU6050 mpu;
        int AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
        const int MPU_addr=0x68;  // I2C address of the MPU-6050


};

#endif
