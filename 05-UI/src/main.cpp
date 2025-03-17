#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <TFT_eSPI.h> // TFT_eSPIライブラリを使用
#include "logo.h"
#include "line_btn.h"
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI(); // TFT インスタンスを作成

XPT2046_Touchscreen ts(D2, D3);

#define PIN D5
#define NUMPIXELS 8

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void drawRGB565Image(int x, int y, int w, int h, const uint16_t *data);

int page; // 0=HOME, 1=LINE

int photoReflector[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
void MovePage(int nextPage);

void setup()
{

  Serial.begin(9600);
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(50);

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
      MovePage(4);
    }
    if (touchX < 200 && touchX > 100 && touchY < 100)
    {
      MovePage(5);
    }
    if (touchX < 100 && touchY < 100)
    {
      MovePage(6);
    }
    delay(1000);
    MovePage(0);
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
    drawRGB565Image(5, 136, 100, 100, line_btn);
    drawRGB565Image(110, 34, 100, 100, line_btn);
    drawRGB565Image(110, 136, 100, 100, line_btn);
    drawRGB565Image(215, 34, 100, 100, line_btn);
    drawRGB565Image(215, 136, 100, 100, line_btn);
  }
  else{
    tft.fillScreen(TFT_WHITE);
    tft.setCursor(10, 10);
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK);
    tft.print("Tuton");
    tft.drawFastHLine(0, 30, 320, TFT_BLACK);
    tft.setCursor(10, 80);
    tft.setTextSize(5);
    tft.setTextColor(TFT_BLACK);
    // tft.print("Page");
    tft.print(page);
  }
}