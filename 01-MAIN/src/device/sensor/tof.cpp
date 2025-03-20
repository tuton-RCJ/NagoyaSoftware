#include "tof.h"

ToF::ToF(int tofL, int tofR)
{
    _pins[0] = tofL;
    _pins[1] = tofR;
}

void ToF::init()
{
    int e = init_tof_sensors();
}

void ToF::read()
{
    for (int i = 0; i < 2; i++)
    {
        values[i] = _sensors[i].readRangeContinuousMillimeters();
    }
}

int ToF::init_tof_sensors()
{
    for (int i = 0; i < 2; i++)
    {
        pinMode(_pins[i], OUTPUT);
        digitalWrite(_pins[i], LOW);
    }
    delay(100);
    for (int i = 0; i < 2; i++)
    {

        digitalWrite(_pins[i], HIGH);
        delay(20);
        _sensors[i].setTimeout(500);
        if (!_sensors[i].init())
        {
            // debugSerial->println("Failed to detect and initialize sensor!");
            return 1;
        }
        _sensors[i].setAddress(0x30 + i);

        _sensors[i].startContinuous(0);
    }
    return 0;
}

void ToF::print(HardwareSerial *serial)
{
    serial->print("L: ");
    serial->print(values[0]);
    serial->print(" R: ");
    serial->print(values[1]);
    serial->println();
}