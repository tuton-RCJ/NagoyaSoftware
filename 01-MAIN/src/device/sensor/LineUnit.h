#ifndef line_H
#define line_H

#include <Arduino.h>

class LineUnit
{
public:
    LineUnit(HardwareSerial *serial);
    int read();
    void Flush();
    void init();
    void setBrightness(int brightness);
    void checkColor(int *color, unsigned long *colorTime, int *LastColor);
    int _photoReflector[15];
    int _frontPhotoReflector;
    int colorL[3];
    int colorR[3];
    int LastColorL;
    int LastColorR;

    /// @brief Time of L color detection 0,1,2,3=Black,White,Green,Red
    unsigned long colorLTime[4];
    /// @brief Time of R color detection 0,1,2,3=Black,White,Green,Red
    unsigned long colorRTime[4];

    void print(HardwareSerial *serial);

private:
    HardwareSerial *_serial;

    void StringToIntValues(String str, int *values);
};

#endif
