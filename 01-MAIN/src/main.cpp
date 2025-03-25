#include <Arduino.h>
#include "./device/device.h"
#include <Wire.h>

#define I2C_SDA PB7
#define I2C_SCL PB6

HardwareSerial uart1(PA10, PA9);              // USB
HardwareSerial uart2(PA3, PA2);               // LINE BOARD
HardwareSerial uart3(PC_11_ALT1, PC_10_ALT1); // STS3032
HardwareSerial uart4(PA1, PA0);               // FRONT
HardwareSerial uart5(PD2, PC12);              // Top
HardwareSerial uart6(PC7, PC6);               // not used

Buzzer buzzer(PB1);
STS3032 sts3032(&uart3);

// LoadCell loadcell(PC0, PC1);
LineUnit line(&uart2);
ToF tof(PA6, PA5);
BNO055 bno(55, &Wire);

L2Unit l2unit(&uart5);
Front front(&uart4);

volatile bool isRescue;

extern void LineSetup();
extern void LineLoop();
extern void RescueSetup();
extern void RescueLoop();
extern bool GetFrontObject(int distace);
void init_i2c();
void Flush();

void setup()
{
  uart1.begin(115200);

  // init I2C sensors
  init_i2c();
  tof.init();
  bno.begin();

  sts3032.isDisabled = false;
  buzzer.isDisabled = false;
  sts3032.stop();

  LineSetup();
  buzzer.boot();
}

void loop()
{
  if (!isRescue)
  {
    LineLoop();
    if (isRescue)
      RescueSetup();
  }
  else
  {
    RescueLoop();
    if (!isRescue)
      LineSetup();
  }
}

void init_i2c()
{
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
}

void Flush()
{
  line.Flush();
  l2unit.Flush();
  front.Flush();
}