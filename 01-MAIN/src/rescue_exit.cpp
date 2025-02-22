#include <Arduino.h>
#include "./device/device.h"

// communication
extern HardwareSerial uart1;
extern HardwareSerial uart4;
extern HardwareSerial uart6;

// Sensor
extern LoadCell loadcell;
extern LineUnit line;
extern ToF tof;
extern BNO055 bno;

// Actuator
extern STS3032 sts3032;
extern Microservo servo;
extern Buzzer buzzer;

extern bool isRescue;
extern int threshold;
extern int silver_threshould;

bool IsNextTurn;
bool IsEscapeZone;
bool IsCheckingSide;
int LastConerDetectionTime;
int LastSideDetectionTime;
int TofPGain = 10;

bool CheckLine()
{
    line.read();
    for (int i = 0; i < 15; i++)
    {
        if (line._photoReflector[i] > threshold)
        {
            isRescue = false;
            return true;
        }
        if (line._photoReflector[i] < silver_threshould)
        {
            IsNextTurn = true;
            LastConerDetectionTime = millis();
            return false;
        }
    }
    return false;
}

void ExitSetup()
{
    IsNextTurn = true;
    IsEscapeZone = true;
    IsCheckingSide = false;
}

void ExitLoop()
{
    if (IsNextTurn)
    {
        sts3032.turn(20, 90);
        IsNextTurn = false;
        return;
    }
    if (IsEscapeZone)
    {
        sts3032.turn(20, 45);
        return;
    }
    if (IsCheckingSide)
    {
        if (millis() - LastSideDetectionTime > 1000)
        {
            IsCheckingSide = false;
        }
        else if (millis() - LastSideDetectionTime > 500)
        {
            sts3032.drive(-20, 0);
        }
        else
        {
            sts3032.drive(20, 0);
        }
        return;
    }
    if(CheckLine()){
        return;
    }
    int max_tof = 0;
    int min_tof = 10000;
    tof.getTofValues();
    for (int i = 2; i < 5; i++)
    {
        if (tof.tof_values[i] > max_tof)
        {
            max_tof = tof.tof_values[i];
        }
        if (tof.tof_values[i] < min_tof)
        {
            min_tof = tof.tof_values[i];
        }
    }

    if (max_tof - min_tof > 40 && min_tof < 40)
    { // 前に三角コーナーがある
        IsEscapeZone = true;
        LastConerDetectionTime = millis();
        return;
    }
    else if (min_tof < 40)
    { // 前に壁がある
        IsNextTurn = true;
        LastConerDetectionTime = millis();
        return;
    }

    else if (tof.tof_values[1] > 100)
    { // 横に穴がある
        IsCheckingSide = true;
        LastSideDetectionTime = millis();
        return;
    }

    sts3032.drive(20, (tof.tof_values[1] - 400) / TofPGain);
}