#include "front.h"
void Front::init(HardwareSerial *serial)
{
    _serial = serial;
    _serial->begin(115200);
    for (int i = 0; i < NUM_VALUES; i++)
    {
        values[i] = 0;
    }
}
void Front::Flush()
{
    while (_serial->available())
    {
        _serial->read();
    }
}

void Front::read()
{
    if (_serial->available() < receiveSize + 1)
    {
        return 1;
    }

    if (_serial->read() == 255)
    {
        for (int i = 0; i < NUM_VALUES; i++)
        {
            values[i] = _serial->read() << 8;
            values[i] |= _serial->read();
        }
    }
    Flush();

    return 0;
}

void Front::print(HardwareSerial *printSerial)
{
    printSerial->print("Front: ");
    for (int i = 0; i < NUM_VALUES; i++)
    {
        printSerial->print(values[i]);
        printSerial->print(" ");
    }
    printSerial->println();
}