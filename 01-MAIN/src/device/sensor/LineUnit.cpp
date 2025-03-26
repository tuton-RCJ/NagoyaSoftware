#include "LineUnit.h"

LineUnit::LineUnit(HardwareSerial *serial)
{
    _serial = serial;
    _serial->begin(115200);
    init();
}

void LineUnit::init()
{
    for (int i = 0; i < 4; i++)
    {
        colorLTime[i] = 0;
        colorRTime[i] = 0;
    }
    LastColorL = 4;
    LastColorR = 4;
    Flush();
}

void LineUnit::Flush()
{
    while (_serial->available())
    {
        _serial->read();
    }
}
bool LineUnit::read()
{
    if (_serial->available() < 23)
    {
        return false;
    }
    while (_serial->available() >= 23)
    {

        if (_serial->read() == 0)
        {
            for (int i = 0; i < 15; i++)
            {
                _photoReflector[i] = _serial->read();
                if (_photoReflector[i] > threshold)
                {
                    photoReflector[i] = 1;
                }
                else if (_photoReflector[i] < silver_threshould)
                {
                    photoReflector[i] = 2;
                }
                else
                {
                    photoReflector[i] = 0;
                }
            }
            _frontPhotoReflector = _serial->read();
            if (_frontPhotoReflector > front_threshould)
            {
                frontPhotoReflector = 1;
            }
            else if (_frontPhotoReflector < silver_threshould)
            {
                frontPhotoReflector = 2;
            }
            else
            {
                frontPhotoReflector = 0;
            }
            for (int i = 0; i < 3; i++)
            {
                colorL[i] = _serial->read();
            }
            for (int i = 0; i < 3; i++)
            {
                colorR[i] = _serial->read();
            }
        }
    }
    Flush();
    checkColor(colorL, colorLTime, &LastColorL);
    checkColor(colorR, colorRTime, &LastColorR);

    return true;
}

void LineUnit::setBrightness(int brightness)
{
    _serial->write(brightness);
}

void LineUnit::checkColor(int colorArr[], unsigned long colorTime[], int *LastColor)
{
    int color = 0;                              // Black
    if (colorArr[0] > 200 && colorArr[1] > 200) // White
    {
        color = 1;
    }
    else if (colorArr[0] < 150 && (float)colorArr[0] * 1.3f < colorArr[1] && _frontPhotoReflector > 225) // Green
    {

        color = 2;
    }
    else if (colorArr[0] > colorArr[1] + 50) // Red
    {
        color = 3;
    }
    else if (colorArr[0] < 120 && colorArr[1] < 120)
    {
        color = 0;
    }
    else
    {
        color = 1;
    }

    if (color != *LastColor % 5)
    {
        *LastColor = color;
    }
    else
    {
        if (*LastColor / 5 > -1)
        {
            colorTime[color] = millis();
        }
        else
        {
            *LastColor += 5;
        }
    }
}

void LineUnit::print(HardwareSerial *serial)
{
    serial->print("photo:");
    for (int i = 0; i < 15; i++)
    {
        serial->print(_photoReflector[i]);
        serial->print(" ");
    }
    serial->print("F:");
    serial->print(_frontPhotoReflector);
    serial->print(" ");
    serial->print("L:");
    for (int i = 0; i < 3; i++)
    {
        serial->print(colorL[i]);
        serial->print(" ");
    }
    serial->print(LastColorL);
    serial->print(" ");
    serial->print("R:");
    for (int i = 0; i < 3; i++)
    {
        serial->print(colorR[i]);
        serial->print(" ");
    }
    serial->print(LastColorR);
    serial->print(" ");
    serial->println();
}