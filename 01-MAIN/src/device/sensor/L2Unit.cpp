#include "L2Unit.h"

L2Unit::L2Unit(HardwareSerial *serial)
{
    _serial = serial;
    _serial->begin(115200);
    init();
}

void L2Unit::init()
{
    for (int i = 0; i < 2; i++)
    {
        tof_values[i] = 0;
        loadcell_values[i] = 0;
    }
    Flush();
}

void L2Unit::Flush()
{
    while (_serial->available())
    {
        _serial->read();
    }
}

int L2Unit::read()
{
    if (_serial->available() < receiveSize + 1)
    {
        return 1;
    }

    if (_serial->read() == 0)
    {
        for (int i = 0; i < 2; i++)
        {
            // 2バイトに分割して受信
            tof_values[i] = _serial->read() << 8;
            tof_values[i] |= _serial->read();
        }
        for (int i = 0; i < 2; i++)
        {
            loadcell_values[i] = _serial->read();
        }
    }

    Flush();

    return 0;
}

void L2Unit::print(HardwareSerial *serial)
{
    serial->print("L2Unit: ");

    serial->print("TOF: ");
    for (int i = 0; i < 2; i++)
    {
        serial->print(tof_values[i]);
        serial->print(" ");
    }
    serial->print("Loadcell: ");
    for (int i = 0; i < 2; i++)
    {
        serial->print(loadcell_values[i]);
        serial->print(" ");
    }
    serial->println();
}