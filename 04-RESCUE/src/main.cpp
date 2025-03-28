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
  HandR.write(40);
  HandL.write(150);
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
  basket.write(110);
  delay(500);
  DetachBasket();
}
void BasketClose()
{
  AttachBasket();
  basket.write(40);
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
  // ArmL.write(102);
  // ArmR.write(80);
  // for(int i=0;i<40;i+=1){
  //   ArmL.write(102-i);
  //   ArmR.write(80+i);
  //   delay(10);
  // }
  ArmL.write(62);
  ArmR.write(120);
  delay(300);
  DetachArm();
}
void setup()
{
  // put your setup code here, to run once:
  uart1.begin(115200);
  uart2.begin(115200);
  uart3.begin(115200);
  uart4.begin(115200);
  init_i2c();
  loadcell.init();
  tof.init();

  AttachHand();
  HandClose();
  ArmUp();
  DetachHand();
  BasketClose();
  OpenMVData = 255;
  pinMode(PB12, INPUT);
  pinMode(PC15, INPUT);
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
  serial.write(digitalRead(PC15));
  serial.write(digitalRead(PB12));

  // 受け取る
  if (serial.available())
  {
    byte c = serial.read();
    if (c < 8)
    {
      uart3.write(c);
      OpenMVData = 255;
    }
    else if (c == 8)
    {
      // readStringしてそのままUI基板に垂れ流す。
      uart4.println(serial.readStringUntil('\n'));
    }
    else if (c < 17)
    {
      // サーボモータを動かす処理を書く
      if (c == 9)
      {
        ArmDown();
      }
      else if (c == 10)
      {
        ArmUp();
      }
      else if (c == 11)
      {
        HandOpen();
      }
      else if (c == 12)
      {
        HandClose();
      }
      else if (c == 13)
      {
        BasketOpen();
      }
      else if (c == 14)
      {
        BasketClose();
      }
      else if (c == 15)
      {
        AttachHand();
      }
      else if (c == 16)
      {
        DetachHand();
      }
    }
    else
    {
      uart4.write(uart3.read());
    }
  }
}
