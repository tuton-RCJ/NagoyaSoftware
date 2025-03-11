#include "loadcell.h"

LoadCell::LoadCell(int pinL, int pinR)
{
    LoadcellPin[0] = pinL;
    LoadcellPin[1] = pinR;
    init();
}

void LoadCell::init()
{
    pinMode(LoadcellPin[0], INPUT);
    pinMode(LoadcellPin[1], INPUT);
    LoadcellOffset[0] = analogRead(LoadcellPin[0]);
    LoadcellOffset[1] = analogRead(LoadcellPin[1]);
}

void LoadCell::read()
{
    for (int i = 0; i < 2; i++)
    {
        raw_values[i] = analogRead(LoadcellPin[i]);
        values[i] = raw_values[i] - LoadcellOffset[i];
    }
}

void LoadCell::CaclurateMoment()
{
    moment = degrees(atan2(values[0], values[1]));
}

void LoadCell::print(HardwareSerial *serial)
{
    for (int i = 0; i < 2; i++)
    {
        serial->print(values[i]);
        serial->print(" ");
    }
    serial->println();
}