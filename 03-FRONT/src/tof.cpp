#include "tof.h"

ToF::ToF()
{
    for (int i = 0; i < 5; i++)
    {
        tof_values[i] = 0;
    }
}
void ToF::init()
{
    while (init_tof_sensors() == 1)
        ;
}

void ToF::getTofValues()
{
    for (int i = 0; i < 5; i++)
    {
        tof_values[i] = tof_sensors[i].readRangeContinuousMillimeters();
    }
}

int ToF::init_tof_sensors()
{
    for (int i = 0; i < 5; i++)
    {
        pinMode(tof_pins[i], OUTPUT);
        digitalWrite(tof_pins[i], LOW);
    }
    delay(100);
    for (int i = 0; i < 5; i++)
    {

        digitalWrite(tof_pins[i], HIGH);

        delay(20);
        tof_sensors[i].setTimeout(500);
        if (!tof_sensors[i].init())
        {
            // Serial.println("Failed to detect and initialize sensor!");
            // Serial.println(i);
            return 1;
        }
        tof_sensors[i].setAddress(0x30 + i);

        tof_sensors[i].startContinuous(0);

        delay(100);
        // digitalWrite(tof_pins[i], LOW);
    }
    return 0;
}

void ToF::print(HardwareSerial *serial)
{
    serial->print("ToF: ");
    for (int i = 0; i < 5; i++)
    {
        serial->print(tof_values[i]);
        serial->print(" ");
    }
    serial->println();
}