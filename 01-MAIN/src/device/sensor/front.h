#ifndef FRONT_H
#define FRONT_H

#include <Arduino.h>
#include <Wire.h>
#include <PacketSerial.h>

class Front
{
public:
    void init(HardwareSerial *serial);
    uint16_t values[5];
    void print(HardwareSerial *serial);
    void read();
    void Flush();

private:
    const size_t NUM_VALUES = 5;
    HardwareSerial *_serial;
    const int receiveSize = 5 * 2;
};

#endif