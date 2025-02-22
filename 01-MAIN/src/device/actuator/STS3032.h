#ifndef STS3032_H
#define STS3032_H

#include <Arduino.h>
#include <SCServo.h>

class STS3032
{
public:
    STS3032(HardwareSerial *serial);
    void init();
    void LeftDrive(int SpeedPercent, int acceleration);
    void RightDrive(int SpeedPercent, int acceleration);
    void stop();
    void drive(int driveSpeedPercent, int turnRate);
    void turn(int speed, int degree);
    void straight(int speed, int distance);

    volatile bool isDisabled = false;
    const int baudrate = 1000000;

private:
    SMS_STS sms_sts;
    HardwareSerial *_serial;
    int _maxSpeed = 7500;
};

#endif