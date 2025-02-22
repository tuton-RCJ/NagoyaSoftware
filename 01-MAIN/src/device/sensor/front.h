#ifndef FRONT_H
#define FRONT_H

#include <Arduino.h>
#include <Wire.h>
#include <PacketSerial.h>

class Front
{
public:
    static void init(HardwareSerial *serial);
    static uint16_t values[5];
    static void getValues();
    static void print(HardwareSerial *serial);
    static void update();

private:
    static const size_t NUM_VALUES = 5;
    static HardwareSerial *serial;
    static PacketSerial packetSerial;
    static void OnPacketReceived(const uint8_t *buffer, size_t size);
};

#endif