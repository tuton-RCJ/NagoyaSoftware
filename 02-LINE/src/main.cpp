#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "colorsensor.h"
#include <PacketSerial.h>

#define RANGE_L PB10
#define GATE_L PB8
#define CK_L PB9
#define DOUT_L PB7

#define RANGE_R PB15 // 8番ピンをRange端子に設定
#define GATE_R PB13  // 9番ピンをGate端子に設定
#define CK_R PB14    // 10番ピンをCK端子に設定
#define DOUT_R PB12  // 11番ピンをDout端子に設定

#define uartPort uart3

HardwareSerial uart1(PA10, PA9);
HardwareSerial uart3(PC11, PC10);

int sensor[] = {PB1, PB0, PC5, PC4, PA7, PA6, PA5, PA4, PA2, PA1, PA0, PC3, PC2, PC1, PC0, PA3}; // LLLLLLLCRRRRRRRF

#define NP_R PC14
#define NP_L PC15
Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, NP_R, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(3, NP_L, NEO_GRB + NEO_KHZ800);

colorsensor colorsensor_L(RANGE_L, GATE_L, CK_L, DOUT_L);
colorsensor colorsensor_R(RANGE_R, GATE_R, CK_R, DOUT_R);

int startTime;
int waitTime = 30;

PacketSerial packetSerial;
uint16_t data[16];

int shiftIn(); // 12ビット分のパルス送信と読み込み処理

void setup()
{

  // シリアル通信設定
  uart1.begin(115200);
  uart3.begin(115200);

  packetSerial.setStream(&uart1);

  strip.begin();
  strip.show();
  for (int i = 0; i < 3; i++)
  {
    strip.setPixelColor(i, 255, 255, 0);
  }
  strip.setBrightness(48);
  strip.show();

  strip2.begin();
  strip2.show();
  for (int i = 0; i < 3; i++)
  {
    strip2.setPixelColor(i, 255, 255, 0);
  }
  strip2.setBrightness(48);
  strip2.show();

  for (int i = 0; i < 16; i++)
  {
    pinMode(sensor[i], INPUT);
  }
}

void loop()
{

  colorsensor_L.start();
  colorsensor_R.start();
  startTime = millis();

  for (int i = 0; i < 16; i++)
  {
    uartPort.print(analogRead(sensor[i]));
    uartPort.print(" ");
  }

  while (millis() - startTime < waitTime)
  {
    // 測光時間が経過するまで待機
  }

  colorsensor_L.end();
  colorsensor_R.end();

  for (int i = 0; i < 3; i++)
  {
    uartPort.print(colorsensor_L.color[i]);
    uartPort.print(" ");
  }
  for (int i = 0; i < 2; i++)
  {
    uartPort.print(colorsensor_R.color[i]);
    uartPort.print(" ");
  }
  uartPort.println(colorsensor_R.color[2]);


  if (uartPort.available())
  {
    int brightness = uartPort.read();

    strip.setBrightness(brightness);
    strip2.setBrightness(brightness);

    strip.show();
    strip2.show();
  }
  
}
