#ifndef BNO055_H
#define BNO055_H

#include <Adafruit_BNO055.h>
#include <SPI.h>
class BNO055
{
public:
    BNO055(int32_t sensorID = 55, TwoWire *wire = &Wire);

    bool begin();
    bool isCalibrated();
    void setZero();

    bool read();
    float direction;
    float heading, pitch, roll;

private:
    Adafruit_BNO055 _bno;
    float direction_offset ;
};

#endif