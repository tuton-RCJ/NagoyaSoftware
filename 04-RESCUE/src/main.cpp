#include <Arduino.h>
#include "loadcell.h"
#include "tof.h"
#include <Wire.h>
#include "Servo.h"

#define DEBUG 0
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

void init_i2c()
{
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
}
void HandOpen()
{
  HandR.write(0);
  HandL.write(180);
}
void HandClose()
{
  HandR.write(180);
  HandL.write(10);
}
void setup()
{
  // put your setup code here, to run once:
  uart1.begin(115200);
  uart2.begin(115200);
  init_i2c();
  loadcell.init();
  tof.init();
  HandL.attach(HandLPin, 500, 2600);
  HandR.attach(HandRPin, 500, 2600);
  ArmL.attach(ArmLPin);
  ArmR.attach(ArmRPin);
  basket.attach(BasketPin);
  HandClose();
}

void loop()
{
  ArmL.write(140);
  ArmR.write(50);
  delay(1500);

  ArmL.write(70);
  ArmR.write(110);
  delay(700);

  return;

  tof.getTofValues();
  loadcell.read();
  uart2.write(0); // ヘッダー
  uart2.write(tof.tof_values[0] >> 8);
  uart2.write(tof.tof_values[0]);
  uart2.write(tof.tof_values[1] >> 8);
  uart2.write(tof.tof_values[1]);
  uart2.write(max(loadcell.raw_values[0] / 4, 1));
  uart2.write(max(loadcell.raw_values[1] / 4, 1));

  if (DEBUG)
  {
    uart1.print("ToF: ");
    uart1.print(tof.tof_values[0]);
    uart1.print(" ");
    uart1.print(tof.tof_values[1]);
    uart1.print(" Loadcell: ");
    uart1.print(loadcell.values[0]);
    uart1.print(" ");
    uart1.print(loadcell.values[1]);
    uart1.print(" ");
    uart1.print(millis());
    uart1.println();
  }
}
