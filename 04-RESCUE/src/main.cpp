#include <Arduino.h>

HardwareSerial uart1(PA10, PA9);

void setup() {
  // put your setup code here, to run once:
  uart1.begin(115200);
}

void loop() {
  uart1.println("Hello World");
  // put your main code here, to run repeatedly:
}
