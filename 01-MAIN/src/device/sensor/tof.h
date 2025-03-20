#ifndef tof_H
#define tof_H

#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

class ToF
{
public:
    ToF(int tofL, int tofR);
    void init();
    void read();
    void print(HardwareSerial *serial);

    int values[2];

private:
    int _pins[2];
    VL53L0X _sensors[2];
    int init_tof_sensors();
};

#endif