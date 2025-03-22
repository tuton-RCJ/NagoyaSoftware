#include "tof.h"

ToF::ToF()
{
    for (int i = 0; i < 5; i++)
    {
        tof_values[i] = 0;
    }
}
void ToF::init()
{
    while (init_tof_sensors() == 1)
        ;
}

void ToF::getTofValues()
{
    for (int i = 0; i < 5; i++)
    {
        tof_values[i] = tof_sensors[i].readRangeContinuousMillimeters();
    }
}

int ToF::init_tof_sensors()
{
    for (int i = 1; i < 5; i++)
    {
        pinMode(tof_pins[i], OUTPUT);
        digitalWrite(tof_pins[i], LOW);
    }
    delay(100);
    for (int i = 0; i < 5; i++)
    {

        digitalWrite(tof_pins[i], HIGH);
        delay(20);
        tof_sensors[i].setTimeout(500);
        if (!tof_sensors[i].init())
        {
            Serial.println("Failed to detect and initialize sensor!");
            Serial.println(i);
            return 1;
        }
        tof_sensors[i].setAddress(0x30 + i);

        tof_sensors[i].startContinuous(0);

        delay(100);
        // digitalWrite(tof_pins[i], LOW);
    }
    return 0;
}

void ToF::print(HardwareSerial *serial)
{
    serial->print("ToF: ");
    for (int i = 0; i < 5; i++)
    {
        serial->print(tof_values[i]);
        serial->print(" ");
    }
    serial->println();
}

void ToF::i2c_scanner()
{
    // scan for i2c devices
    byte error, address;
    int nDevices;

    Serial.println("Scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");

    delay(500);
}