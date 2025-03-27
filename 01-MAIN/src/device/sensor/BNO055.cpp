#include "BNO055.h"

BNO055::BNO055(int32_t sensorID, TwoWire *wire) : _bno(sensorID, 0x28, wire) {}

bool BNO055::begin()
{
    direction_offset = 0;
    return _bno.begin();
}

bool BNO055::isCalibrated()
{
    uint8_t system, gyro, accel, mag;
    _bno.getCalibration(&system, &gyro, &accel, &mag);
    return (system == 3 && gyro == 3 && accel == 3 && mag == 3);
}

bool BNO055::read()
{
    imu::Vector<3> euler = _bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    heading = euler.x();
    pitch = euler.y();
    roll = euler.z();
    direction = heading - direction_offset;
    if (direction < 0)
    {
        direction += 360;
    }
    return true;
}

void BNO055::setZero()
{
    read();
    direction_offset = heading;
}

void BNO055::print(HardwareSerial *printSerial)
{
    printSerial->print("Heading: ");
    printSerial->print(heading);
    printSerial->print(" Pitch: ");
    printSerial->print(pitch);
    printSerial->print(" Roll: ");
    printSerial->print(roll);
    printSerial->print(" Direction: ");
    printSerial->println(direction);
}