#include <Arduino.h>
#include "loadcell.h"
#include "tof.h"
#include <Wire.h>

#define DEBUG 0
HardwareSerial uart1(PA_10, PA_9);
HardwareSerial uart2(PA_3, PA_2);   // MAIN
HardwareSerial uart3(PB_11, PB_10); // OpenMV
HardwareSerial uart4(PC_11, PC_10); // XIAO

LoadCell loadcell(PC1, PC0);
ToF tof(PA14, PA15);
#define I2C_SDA PB7
#define I2C_SCL PB6

void init_i2c()
{
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
}

void setup()
{
  // put your setup code here, to run once:
  uart1.begin(115200);
  uart2.begin(115200);
  init_i2c();
  loadcell.init();
  tof.init();
}

void loop()
{

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
