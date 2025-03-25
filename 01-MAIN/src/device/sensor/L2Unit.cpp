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
        loadcell_detected[i] = false;
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

bool L2Unit::read()
{
    if (_serial->available() < receiveSize + 1)
    {
        return false;
    }

    if (_serial->read() == 255)
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
            loadcell_detected[i] = loadcell_values[i] > loadcell_threshold[i];
        }
        OpenMVData = _serial->read();
        if (OpenMVData == 254)
        {
            OpenMVData = 255;
        }
        for (int i = 0; i < 2; i++)
        {

            touch[i] = !(bool)_serial->read();
        }
    }

    Flush();

    return true;
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
    serial->print("OpenMV: ");
    serial->print(OpenMVData);
    serial->print("Touch: ");
    for (int i = 0; i < 2; i++)
    {
        serial->print(touch[i]);
        serial->print(" ");
    }
    serial->println();
}

void L2Unit::setCameraTarget(int target)
{
    _serial->write(target);
}
void L2Unit::setCameraIdling()
{
    //_serial->write(4);
}
void L2Unit::setCameraCCW()
{
    //_serial->write(5);
}
void L2Unit::setCameraCW()
{
    // _serial->write(6);
}
void L2Unit::setCameraPcontrol()
{
    //_serial->write(7);
}
void L2Unit::ArmDown()
{
    _serial->write(9);
}
void L2Unit::ArmUp()
{
    _serial->write(10);
}
void L2Unit::HandOpen()
{
    _serial->write(11);
}
void L2Unit::HandClose()
{
    _serial->write(12);
}
void L2Unit::BasketOpen()
{
    _serial->write(13);
}
void L2Unit::BasketClose()
{
    _serial->write(14);
}
void L2Unit::AttachHand()
{
    _serial->write(15);
}
void L2Unit::DetachHand()
{
    _serial->write(16);
}
