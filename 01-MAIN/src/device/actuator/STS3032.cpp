#include "STS3032.h"

// positive speed is forward, Positive turnRate is right

STS3032::STS3032(HardwareSerial *serial)
{
    _serial = serial;
    _serial->begin(baudrate);
    sms_sts.pSerial = _serial;
    init();
}

void STS3032::init()
{
    for (int i = 1; i < 5; i++)
    {
        sms_sts.unLockEprom(i);
        sms_sts.EnableTorque(i, 1);
        sms_sts.WheelMode(i);
        sms_sts.writeByte(i, SMS_STS_MODE, 1);
        sms_sts.LockEprom(i);
    }
}

void STS3032::LeftDrive(int SpeedPercent, int acceleration)
{

    int _SpeedPercent = constrain(SpeedPercent, -100, 100);
    int speed = _SpeedPercent * _maxSpeed / 100;
    if (isDisabled)
        speed = 0;
    for (int i = 0; i < 3; i++)
    {
        sms_sts.WriteSpe(1, speed, acceleration);
        sms_sts.WriteSpe(2, speed, acceleration);
    }
}

void STS3032::RightDrive(int SpeedPercent, int acceleration)
{
    int _SpeedPercent = constrain(SpeedPercent, -100, 100);
    int speed = _SpeedPercent * _maxSpeed / 100;

    speed = -speed;
    if (isDisabled)
        speed = 0;
    for (int i = 0; i < 3; i++)
    {
        sms_sts.WriteSpe(3, speed, acceleration);
        sms_sts.WriteSpe(4, speed, acceleration);
    }
}

void STS3032::stop()
{
    for (int i = 0; i < 5; i++)
    {
        LeftDrive(0, 0);
        RightDrive(0, 0);
    }
}

/// @brief
/// @param driveSpeedPercent -100~100
/// @param turnRate -100~100
void STS3032::drive(int driveSpeedPercent, int turnRate)
{
    if (turnRate >= 100)
    {
        LeftDrive(driveSpeedPercent, 0);
        RightDrive(-driveSpeedPercent, 0);
    }
    else if (turnRate <= -100)
    {
        LeftDrive(-driveSpeedPercent, 0);
        RightDrive(driveSpeedPercent, 0);
    }
    else if (turnRate >= 0)
    {
        LeftDrive(driveSpeedPercent, 0);
        RightDrive(driveSpeedPercent * (50 - turnRate) / 50, 0);
    }
    else if (turnRate <= 0)
    {
        LeftDrive(driveSpeedPercent * (50 + turnRate) / 50, 0);
        RightDrive(driveSpeedPercent, 0);
    }
}

void STS3032::turn(int speed, int degree)
{
    float speedPercent = constrain(speed / 100.0f, -1, 1);
    float Deg360PerSpeed1 = 2.1f;
    float time = degree / 360.0f / speedPercent * Deg360PerSpeed1 * (degree > 0 ? 1 : -1);
    drive(speed, degree > 0 ? 100 : -100);
    delay(time * 1000);
    stop();
}

/// @brief
/// @param speed 0~100
/// @param distance  mm
void STS3032::straight(int speed, int distance)
{
    float speedPercent = constrain(speed / 100.0f, -1, 1);
    float Distance1cmPerSpeed1 = 0.36f;
    float time = distance / 100.0f / speedPercent * Distance1cmPerSpeed1 * (distance > 0 ? 1 : -1);
    drive(speed * (distance > 0 ? 1 : -1), 0);
    delay(time * 1000);
    stop();
}