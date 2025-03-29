#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <TFT_eSPI.h> // TFT_eSPIライブラリを使用
#include "logo.h"
#include "line_btn.h"
#include <XPT2046_Touchscreen.h>
#include "StatusImage.h"

TFT_eSPI tft = TFT_eSPI(); // TFT インスタンスを作成

XPT2046_Touchscreen ts(D2, D3);

#define PIN D5
#define NUMPIXELS 8

#define SW_Pin D4

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void drawRGB565Image(int x, int y, int w, int h, const uint16_t *data);

int page; // 0=HOME, 1=LINE, 2=Front, 3=MAIN 5=Layer2 6=SIRO

int photoReflector[16] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 50};
int color[6] = {0, 0, 0, 0, 0, 0};
int frontTof[5] = {200, 200, 200, 200, 200};
int loadcell[2] = {160, 160};
int mainTof[2] = {100, 100};
int gyro[3] = {345, 0, 5};
int layer2Tof[2] = {100, 100};
int rescueSw[2] = {0, 0};
void MovePage(int nextPage);

void StringToIntValues(String str, int values[]);
void setup()
{

  Serial.begin(9600);
  Serial1.begin(115200);
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(10);

  tft.init();
  tft.setRotation(3); // 画面の向き（0～3）

  ts.begin();
  ts.setRotation(3);

  drawRGB565Image(0, 0, 320, 240, logo);
  // tft.pushImage(0, 0, 320, 240, logo);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    pixels.show();
    delay(100);
  }
  delay(1000);

  MovePage(0);

  // // 文字の設定
  // tft.setTextColor(TFT_WHITE, TFT_RED); // 白文字、赤背景
  // tft.setTextSize(3);
  // tft.setCursor(50, 100);
  // tft.print("Hello, World!");
  pinMode(SW_Pin, INPUT);
}

void loop()
{
  int touchX, touchY;
  if (ts.touched())
  {
    TS_Point p = ts.getPoint();
    touchX = map(p.x, 0, 4095, 0, 320);
    touchY = map(p.y, 0, 4095, 0, 240);
    Serial.print(touchX);
    Serial.print(",");
    Serial.println(touchY);
    if (touchX > 200 && touchY > 100)
    {
      MovePage(1);
    }
    if (touchX < 200 && touchX > 100 && touchY > 100)
    {
      MovePage(2);
    }
    if (touchX < 100 && touchY > 100)
    {
      MovePage(3);
    }
    if (touchX > 200 && touchY < 100)
    {
      MovePage(0);
    }
    if (touchX < 200 && touchX > 100 && touchY < 100)
    {
      MovePage(5);
    }
    if (touchX < 100 && touchY < 100)
    {
      MovePage(6);
    }
  }

  if (Serial.available())
  {
    byte data = Serial.read();
    if (data == 20)
    {
      drawRGB565Image(0, 0, 320, 240, SilverDetecting);
    }
    if (data == 21)
    {
      drawRGB565Image(0, 0, 320, 240, GreenDetecting);
    }
    if (data == 22)
    {
      drawRGB565Image(0, 0, 320, 240, BlackDetecting);
    }
    if (data == 23)
    {
      drawRGB565Image(0, 0, 320, 240, RedDetecting);
    }
  }

  if (!digitalRead(SW_Pin))
  {
    MovePage(page);
  }

  if (Serial1.available())
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(150, 150, 0));
    }
    pixels.show();
    String t = Serial1.readStringUntil('\n');
    int data[38];
    StringToIntValues(t, data);
    for (int i = 0; i < 16; i++)
    {
      photoReflector[i] = data[i];
    }
    for (int i = 0; i < 6; i++)
    {
      color[i] = data[i + 16];
    }
    for (int i = 0; i < 5; i++)
    {
      frontTof[i] = data[i + 22];
    }
    for (int i = 0; i < 2; i++)
    {
      loadcell[i] = data[i + 27];
    }
    for (int i = 0; i < 2; i++)
    {
      mainTof[i] = data[i + 29];
    }
    for (int i = 0; i < 3; i++)
    {
      gyro[i] = data[i + 32];
    }
    for (int i = 0; i < 2; i++)
    {
      layer2Tof[i] = data[i + 34];
    }
    for (int i = 0; i < 2; i++)
    {
      rescueSw[i] = data[i + 36];
    }
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    }
    pixels.show();
  }

  // Serial.println("Hello World");
  // for (int i = 0; i < NUMPIXELS; i++)
  // {
  //   pixels.setPixelColor(i, pixels.Color(0, 150, 0));
  //   pixels.show();
  //   delay(100);
  // }
  // tft.fillScreen(TFT_GREEN);
  // tft.setTextColor(TFT_WHITE, TFT_GREEN);
  // tft.setTextSize(3);
  // tft.setCursor(50, 100);
  // tft.print("Hello, World!");
  // delay(1000);
  // for (int i = 0; i < NUMPIXELS; i++)
  // {
  //   pixels.setPixelColor(i, pixels.Color(150, 0, 0));
  //   pixels.show();
  //   delay(100);
  // }
  // tft.fillScreen(TFT_RED);
  // tft.setTextColor(TFT_WHITE, TFT_RED);
  // tft.setTextSize(3);
  // tft.setCursor(50, 100);
  // tft.print("Hello, World!");

  // delay(1000);
  // for (int i = 0; i < NUMPIXELS; i++)
  // {
  //   pixels.setPixelColor(i, pixels.Color(0, 0, 150));
  //   pixels.show();
  //   delay(100);
  // }
  // tft.fillScreen(TFT_BLUE);
  // tft.setTextColor(TFT_WHITE, TFT_BLUE);
  // tft.setTextSize(3);
  // tft.setCursor(50, 100);
  // tft.print("Hello, World!");

  // delay(1000);
}

void drawRGB565Image(int x, int y, int w, int h, const uint16_t *data)
{
  tft.startWrite();
  tft.setAddrWindow(x, y, w, h);
  for (int i = 0; i < w * h; i++)
  {
    tft.pushColor(pgm_read_word(&data[i])); // pgm_read_byte を pgm_read_word に戻す
  }
  tft.endWrite();
}

void MovePage(int nextPage)
{
  page = nextPage;
  if (page == 0)
  {
    tft.fillScreen(TFT_WHITE);
    tft.setCursor(10, 10);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    tft.print("Tuton");
    tft.drawFastHLine(0, 30, 320, TFT_BLACK);
    drawRGB565Image(5, 34, 100, 100, line_btn);
    // drawRGB565Image(5, 136, 100, 100, line_btn);
    drawRGB565Image(110, 34, 100, 100, line_btn);
    drawRGB565Image(110, 136, 100, 100, line_btn);
    drawRGB565Image(215, 34, 100, 100, line_btn);
    drawRGB565Image(215, 136, 100, 100, line_btn);
  }
  else
  {
    tft.fillScreen(TFT_WHITE);
    tft.setCursor(10, 10);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    tft.print("Tuton");
    tft.drawFastHLine(0, 30, 320, TFT_BLACK);
    if (page == 1)
    {
      tft.setCursor(10, 40);
      tft.setTextSize(2);
      tft.print("LINE");
      tft.setCursor(10, 70);
      tft.setTextSize(1);
      tft.print("F: ");
      tft.print(photoReflector[15]);
      tft.setCursor(10, 90);
      tft.print("L: ");
      for (int i = 0; i < 7; i++)
      {
        tft.print(photoReflector[i]);
        tft.print(" ");
      }
      tft.setCursor(10, 110);
      tft.print("C: ");
      tft.print(photoReflector[7]);
      tft.setCursor(10, 130);
      tft.print("R: ");
      for (int i = 8; i < 15; i++)
      {
        tft.print(photoReflector[i]);
        tft.print(" ");
      }
      tft.setCursor(10, 150);
      tft.print("ColorL: ");
      for (int i = 0; i < 3; i++)
      {
        tft.print(color[i]);
        tft.print(" ");
      }
      tft.setCursor(10, 170);
      tft.print("ColorR: ");
      for (int i = 3; i < 6; i++)
      {
        tft.print(color[i]);
        tft.print(" ");
      }
    }
    if (page == 2)
    {
      tft.setCursor(10, 40);
      tft.setTextSize(2);
      tft.print("Front");
      tft.setCursor(10, 70);
      tft.setTextSize(2);
      tft.print("ToF: ");
      for (int i = 0; i < 5; i++)
      {
        tft.print(frontTof[i]);
        tft.print(" ");
      }
      tft.setCursor(10, 90);
      tft.print("loadcell: ");
      for (int i = 0; i < 2; i++)
      {
        tft.print(loadcell[i]);
        tft.print(" ");
      }
    }
    if (page == 3)
    {
      tft.setCursor(10, 40);
      tft.setTextSize(2);
      tft.print("MAIN Board");
      tft.setCursor(10, 70);
      tft.setTextSize(2);
      tft.print("ToF: ");
      for (int i = 0; i < 2; i++)
      {
        tft.print(mainTof[i]);
        tft.print(" ");
      }
      tft.setCursor(10, 90);
      tft.print("Gyro: ");
      for (int i = 0; i < 3; i++)
      {
        tft.print(gyro[i]);
        tft.print(" ");
      }
    }
    if (page == 5)
    {
      tft.setCursor(10, 40);
      tft.setTextSize(2);
      tft.print("Layer2");
      tft.setCursor(10, 70);
      tft.setTextSize(2);
      tft.print("ToF: ");
      for (int i = 0; i < 2; i++)
      {
        tft.print(layer2Tof[i]);
        tft.print(" ");
      }
      tft.setCursor(10, 90);
      tft.print("SW: ");
      for (int i = 0; i < 2; i++)
      {
        tft.print(rescueSw[i]);
        tft.print(" ");
      }
    }
    if (page == 6)
    {
      tft.setCursor(10, 50);
      tft.setTextSize(3);
      tft.print("I am");
      tft.setCursor(10, 100);
      tft.setTextSize(16);
      tft.print("SIRO");
    }
    tft.setCursor(10, 210);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    tft.print("←Home");
  }
}

void StringToIntValues(String str, int values[])
{
  int i = 0;
  int j = 0;
  while (i < str.length())
  {
    if (j > 37)
    {
      break;
    }
    if (str[i] == ' ')
    {
      i++;
      continue;
    }
    String value = "";
    while (str[i] != ' ' && i < str.length())
    {
      value += str[i];
      i++;
    }
    values[j] = value.toInt();
    j++;
  }
}
