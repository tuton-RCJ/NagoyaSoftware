#include "BNO055.h"

BNO055::BNO055(int32_t sensorID, TwoWire *wire) : _bno(sensorID, 0x28 ,wire) {}

bool BNO055::begin() {
    return _bno.begin();
}

bool BNO055::isCalibrated() {
    uint8_t system, gyro, accel, mag;
    _bno.getCalibration(&system, &gyro, &accel, &mag);
    return (system == 3 && gyro == 3 && accel == 3 && mag == 3);
}

bool BNO055::readEulerAngles() {
    imu::Vector<3> euler = _bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    heading = euler.x();
    pitch = euler.y();
    roll = euler.z();
    return true;
}