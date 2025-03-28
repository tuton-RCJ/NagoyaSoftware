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
bool isSensing = false;

void LEDoff()
{
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
}

void setup()
{
    Serial.begin(115200);
    Serial1.setTX(0);
    Serial1.setRX(1);
    Serial1.begin(115200);
    pixels.begin();
    // pixels.setBrightness(60);
    // for (int i = 0; i < 1; i++)
    // {
    //     for (int i = 0; i < NUMPIXELS; i++)
    //     {
    //         pixels.setPixelColor(i, pixels.Color(255, 255, 0));
    //     }
    //     pixels.show();
    //     delay(50);
    // }

    Wire.setSCL(D5);
    Wire.setSDA(D4);
    // Wire.setClock(400000);
    Wire.begin();
    pixels.setBrightness(20);
    LEDoff();
    isSensing = false;
    // put your setup code here, to run once:

    // for (int i = 0; i < NUMPIXELS; i++)
    // {
    //     pixels.setPixelColor(i, pixels.Color(0, 78, 0));
    //     pixels.show();
    //     delay(100);
    // }
    // Serial.println("Hello World");
    // watchdog_enable(600, 1);
}

void loop()
{

    if (Serial1.available())
    {
        int data = Serial1.read();
        if (data == 1 && !isSensing)
        {
            isSensing = true;
            tof.init();
        }
        else if (data == 0)
        {
            isSensing = false;
        }
    }
    if (Serial.available())
    {
        int data = Serial.read();
        if (data == 1)
        {
            isSensing = true;
            tof.init();
        }
        else if (data == 0)
        {
            isSensing = false;
        }
    }
    if (!isSensing)
    {
        LEDoff();
        tof.XshutLow();
        return;
    }
    // watchdog_update();
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