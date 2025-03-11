#include "tof.h"

ToF::ToF(int front1, int front2)
{
    tof_pins[0] = front1;
    tof_pins[1] = front2;
}

void ToF::init()
{
    int e = init_tof_sensors();
}

void ToF::getTofValues()
{
    for (int i = 0; i < 2; i++)
    {
        tof_values[i] = tof_sensors[i].readRangeContinuousMillimeters();
    }
}

int ToF::init_tof_sensors()
{
    for (int i = 0; i < 2; i++)
    {
        pinMode(tof_pins[i], OUTPUT);
        digitalWrite(tof_pins[i], LOW);
    }
    delay(100);
    for (int i = 0; i < 2; i++)
    {

        digitalWrite(tof_pins[i], HIGH);
        delay(20);
        tof_sensors[i].setTimeout(500);
        if (!tof_sensors[i].init())
        {
            // debugSerial->println("Failed to detect and initialize sensor!");
            return 1;
        }
        tof_sensors[i].setAddress(0x30 + i);

        tof_sensors[i].startContinuous(0);
    }
    return 0;
}

void ToF::print(HardwareSerial *serial)
{
    serial->print("L: ");
    serial->print(tof_values[0]);
    serial->print(" R: ");
    serial->print(tof_values[1]);
    serial->println();
}