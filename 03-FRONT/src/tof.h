#ifndef tof_H
#define tof_H

#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

class ToF
{
public:
    ToF();
    void init();
    uint16_t tof_values[5];
    void getTofValues();
    void print(HardwareSerial *serial);

private:
    int tof_pins[5] = {D0, D1, D2, D3, D8};
    VL53L0X tof_sensors[5];
    int init_tof_sensors();
};

#endif