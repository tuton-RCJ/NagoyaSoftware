#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <TFT_eSPI.h>  // TFT_eSPIライブラリを使用

TFT_eSPI tft = TFT_eSPI();  // TFT インスタンスを作成

#define PIN D5
#define NUMPIXELS 8

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{

  Serial.begin(9600);
  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setBrightness(50);
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    pixels.show();
    delay(100);
  }

  tft.init();
  tft.setRotation(3);  // 画面の向き（0～3）

  // 背景を赤にする
  tft.fillScreen(TFT_RED);

  // 文字の設定
  tft.setTextColor(TFT_WHITE, TFT_RED);  // 白文字、赤背景
  tft.setTextSize(3);
  tft.setCursor(50, 100);
  tft.print("Hello, World!");
}

void loop()
{
  Serial.println("Hello World");
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    pixels.show();
    delay(100);
  }
  tft.fillScreen(TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_GREEN);
  tft.setTextSize(3);
  tft.setCursor(50, 100);
  tft.print("Hello, World!");
  delay(1000);
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(150, 0, 0));
    pixels.show();
    delay(100);
  }
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.setTextSize(3);
  tft.setCursor(50, 100);
  tft.print("Hello, World!");


  delay(1000);
  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 150));
    pixels.show();
    delay(100);
  }
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(3);
  tft.setCursor(50, 100);
  tft.print("Hello, World!");
  
  delay(1000);

}