#include "front.h"

// uart1をextern
extern HardwareSerial uart1;

Front::Front(HardwareSerial *serial)
{
    _serial = serial;
    _serial->begin(115200);
    init();
}

void Front::init()
{
    for (int i = 0; i < NUM_VALUES; i++)
    {
        values[i] = 0;
    }
    Flush();
}

void Front::Flush()
{
    while (_serial->available())
    {
        _serial->read();
    }
}

bool Front::read()
{
    if (_serial->available() < receiveSize + 1)
    {
        return false;
    }
    while (_serial->available() >= receiveSize + 1)
    {
        if (_serial->read() == 255)
        {
            for (int i = 0; i < NUM_VALUES; i++)
            {
                values[i] = _serial->read() << 8;
                values[i] |= _serial->read();
            }
        }
    }
    Flush();
    print(&uart1);
    return true;
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

void Front::begin()
{
    for (int i = 0; i < 5; i++)
    {
        _serial->write(0x01);
        delay(100);
    }
    delay(500);
}

void Front::end()
{
    for (int i = 0; i < 5; i++)
    {
        _serial->write(0x00);
        delay(20);
    }
}