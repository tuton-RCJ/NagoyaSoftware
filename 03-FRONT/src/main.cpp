#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "tof.h"

// include watchdog
#include <hardware/watchdog.h>

#define PIN D9
#define NUMPIXELS 5

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

ToF tof;

void setup()
{
    Serial.begin(115200);
    Serial1.setTX(0);
    Serial1.setRX(1);
    Serial1.begin(115200);
    pixels.begin();
    pixels.setBrightness(60);
    for (int i = 0; i < 1; i++)
    {
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        }
        pixels.show();
        delay(100);
        for (int i = 0; i < NUMPIXELS; i++)
        {
            pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        pixels.show();
        //delay(100);
    }
    pixels.setBrightness(20);
    pixels.show();

    Wire.setSCL(D5);
    Wire.setSDA(D4);
    // Wire.setClock(400000);
    Wire.begin();
    delay(10);
    tof.init();

    // put your setup code here, to run once:

    // for (int i = 0; i < NUMPIXELS; i++)
    // {
    //     pixels.setPixelColor(i, pixels.Color(0, 78, 0));
    //     pixels.show();
    //     delay(100);
    // }
    // Serial.println("Hello World");
    //watchdog_enable(600, 1);
}

void loop()
{
    //watchdog_update();
    tof.getTofValues();
    // tof.print(&Serial);
    Serial1.write(255);
    for (int i = 0; i < 5; i++)
    {
        Serial1.write(tof.tof_values[i] >> 8);
        Serial1.write(min(tof.tof_values[i] & 0xFF, 254));
    }
    // Serial.println("Hello World");

    for (int i = 0; i < NUMPIXELS; i++)
    {
        if (tof.tof_values[i] < 50)
        {
            pixels.setPixelColor(i, pixels.Color(78, 0, 0));
        }
        else
        {
            pixels.setPixelColor(i, pixels.Color(0, 78, 0));
        }
    }
    pixels.show();
}