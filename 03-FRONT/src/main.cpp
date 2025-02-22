#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "tof.h"
#include <PacketSerial.h>

#define PIN D9
#define NUMPIXELS 5

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

ToF tof;

PacketSerial packetSerial;

void setup()
{
    Serial.begin(115200);
    Serial1.setTX(0);
    Serial1.setRX(1);
    Serial1.begin(115200);
    //packetSerial.begin(115200);
    packetSerial.setStream(&Serial1);

    Wire.setSCL(D5);
    Wire.setSDA(D4);
    Wire.begin();

    tof.init();
    // put your setup code here, to run once:
    pixels.begin();
    for (int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(0, 78, 0));
        pixels.show();
        delay(100);
    }
    // Serial.println("Hello World");
}

void loop()
{
    // put your main code here, to run repeatedly:
    // for (int i = 0; i < NUMPIXELS; i++)
    // {
    //     pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    //     pixels.show();
    //     delay(100);
    // }
    // delay(1000);
    // for (int i = 0; i < NUMPIXELS; i++)
    // {
    //     pixels.setPixelColor(i, pixels.Color(0, 78, 0));
    //     pixels.show();
    //     delay(100);
    // }
    // delay(1000);
    tof.getTofValues();
    tof.print(&Serial);

    packetSerial.send((uint8_t*)tof.tof_values, sizeof(tof.tof_values));
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