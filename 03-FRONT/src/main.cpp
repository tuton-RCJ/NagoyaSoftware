#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIN D9
#define NUMPIXELS 5

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
    // put your setup code here, to run once:
    pixels.begin();
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(0, 150, 0));
        pixels.show();
        delay(100);
    }
    delay(1000);
}

void loop()
{
    // put your main code here, to run repeatedly:
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        pixels.show();
        delay(100);
    }
    delay(1000);
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(0, 150, 0));
        pixels.show();
        delay(100);
    }
    delay(1000);
}