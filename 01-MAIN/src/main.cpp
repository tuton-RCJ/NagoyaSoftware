#include <Arduino.h>
#include "buzzer.h"

Buzzer buzzer(PB1);

HardwareSerial uart1(PA10, PA9);
void setup()
{
  uart1.begin(115200);
}

void loop()
{
  // put your main code here, to run repeatedly:
  uart1.println("Hello, World!");
  buzzer.boot();
}
