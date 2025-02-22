#include "Microservo.h"

Microservo::Microservo(HardwareSerial *serial)
{
    _serial = serial;
}

void Microservo::HandClose()
{
    _serial->println("HandClose");
}

void Microservo::HandOpen()
{
    _serial->println("HandOpen");
}

void Microservo::ArmUp()
{
    _serial->println("ArmUp");
}

void Microservo::ArmDown()
{
    _serial->println("ArmDown");
}

void Microservo::BasketClose()
{
    _serial->println("BasketClose");
}

void Microservo::BasketOpen()
{
    _serial->println("BasketOpen");
}

void Microservo::AttachArmServo()
{
    _serial->println("AttachArmServo");
}

void Microservo::AttachBasketServo()
{
    _serial->println("AttachBasketServo");
}

void Microservo::DetachArmServo()
{
    _serial->println("DetachArmServo");
}

void Microservo::DetachBasketServo()
{
    _serial->println("DetachBasketServo");
}

void Microservo::initPos()
{
    AttachArmServo();
    AttachBasketServo();
    delay(100);
    HandClose();
    ArmUp();
    BasketClose();
    delay(1000);
    DetachArmServo();
    DetachBasketServo();
}