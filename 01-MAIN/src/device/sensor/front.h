#ifndef FRONT_H
#define FRONT_H

#include <Arduino.h>
#include <Wire.h>

class Front
{
public:
    Front(HardwareSerial *serial);
    void init();
    bool read();
    void Flush();
    void print(HardwareSerial *serial);


    uint16_t values[5];

private:
    const size_t NUM_VALUES = 5;
    HardwareSerial *_serial;
    const int receiveSize = 5 * 2;
};

#endif