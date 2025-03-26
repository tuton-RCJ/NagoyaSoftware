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
    for (int i = 0; i < 5; i++)
    {
        pinMode(tof_pins[i], OUTPUT);
        digitalWrite(tof_pins[i], LOW);
    }
    delay(10);
    for (int i = 4; i >-1; i--)
    {
        init_tof_sensors(i);
    }
}

void ToF::getTofValues()
{
    for (int i = 0; i < 5; i++)
    {
        tof_values[i] = tof_sensors[i].readRangeContinuousMillimeters();
    }
}

int ToF::init_tof_sensors(int i)
{
    digitalWrite(tof_pins[i], HIGH);
    delay(10);
    //i2c_scanner();
    tof_sensors[i].setTimeout(500);
    if (!tof_sensors[i].init())
    {
        Serial.println("Failed to detect and initialize sensor!");
        Serial.println(i);
    }
    else
    {
        tof_sensors[i].setAddress(0x30 + i);
    }
    tof_sensors[i].setMeasurementTimingBudget(50000);

    tof_sensors[i].startContinuous();

    // delay(100);
    // digitalWrite(tof_pins[i], LOW);

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

bool ToF::i2c_scanner()
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
    {
        Serial.println("No I2C devices found\n");
        return false;
    }
    else
    {
        Serial.println("done\n");
        return true;
    }
}