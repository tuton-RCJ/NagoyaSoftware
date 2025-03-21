#include <Arduino.h>
#include "loadcell.h"
#include "tof.h"
#include <Wire.h>
#include "Servo.h"

#define serial uart2
HardwareSerial uart1(PA_10, PA_9);
HardwareSerial uart2(PA_3, PA_2);   // MAIN
HardwareSerial uart3(PB_11, PB_10); // OpenMV
HardwareSerial uart4(PC_11, PC_10); // XIAO

LoadCell loadcell(PC1, PC0);
ToF tof(PA14, PA15);
#define I2C_SDA PB7
#define I2C_SCL PB6

#define HandRPin PA8
#define HandLPin PA11
#define ArmLPin PA1
#define ArmRPin PA0
#define BasketPin PA6

Servo HandL;
Servo HandR;
Servo ArmL;
Servo ArmR;
Servo basket;

int OpenMVData;

void init_i2c()
{
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
}

void HandOpen()
{
  HandR.write(60);
  HandL.write(130);
}
void HandClose()
{
  HandR.write(180);
  HandL.write(10);
}

void AttachHand()
{
  HandL.attach(HandLPin, 500, 2600);
  HandR.attach(HandRPin, 500, 2600);
}
void DetachHand()
{
  HandL.detach();
  HandR.detach();
}

void AttachBasket()
{
  basket.attach(BasketPin);
}
void DetachBasket()
{
  basket.detach();
}
void BasketOpen()
{
  AttachBasket();
  basket.write(80);
  delay(500);
  DetachBasket();
}
void BasketClose()
{
  AttachBasket();
  basket.write(90);
  delay(500);
  DetachBasket();
}

void AttachArm()
{
  ArmL.attach(ArmLPin, 500, 2400);
  ArmR.attach(ArmRPin, 500, 2400);
}
void DetachArm()
{
  ArmL.detach();
  ArmR.detach();
}
void ArmDown()
{
  AttachArm();
  ArmL.write(180);
  ArmR.write(12);
  delay(300);
  DetachArm();
}
void ArmUp()
{
  AttachArm();
  ArmL.write(80);
  ArmR.write(112);
  delay(300);
  DetachArm();
}
void setup()
{
  // put your setup code here, to run once:
  uart1.begin(115200);
  uart2.begin(115200);
  uart3.begin(115200);
  init_i2c();
  loadcell.init();
  tof.init();

  AttachHand();
  HandClose();
  ArmUp();
  DetachHand();
  BasketClose();
  OpenMVData = 255;
}

void loop()
{
  // データとる
  tof.getTofValues();
  loadcell.read();

  while (uart3.available())
  {
    OpenMVData = uart3.read();
  }

  // 送る
  serial.write(255); // ヘッダー255
  serial.write(tof.values[0] >> 8);
  serial.write(min(tof.values[0] & 0xff, 254));
  serial.write(tof.values[1] >> 8);
  serial.write(min(tof.values[1] & 0xff, 254));
  serial.write(min(loadcell.raw_values[0] / 4, 254));
  serial.write(min(loadcell.raw_values[1] / 4, 254));
  serial.write(min(OpenMVData, 254));

  // 受け取る
  if (serial.available())
  {
    byte c = serial.read();
    if (c < 4)
    {
      uart3.write(c);
      OpenMVData = 255;
    }
    else if (c == 4)
    {
      // readStringしてそのままUI基板に垂れ流す。
    }
    else
    {
      // サーボモータを動かす処理を書く
      if (c == 5)
      {
        ArmDown();
      }
      else if (c == 6)
      {
        ArmUp();
      }
      else if (c == 7)
      {
        HandOpen();
      }
      else if (c == 8)
      {
        HandClose();
      }
      else if (c == 9)
      {
        BasketOpen();
      }
      else if (c == 10)
      {
        BasketClose();
      }
      else if (c == 11)
      {
        AttachHand();
      }
      else if (c == 12)
      {
        DetachHand();
      }
    }
  }
}
