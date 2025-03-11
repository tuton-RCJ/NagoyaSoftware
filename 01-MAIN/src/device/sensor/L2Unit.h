#ifndef L2Unit_H
#define L2Unit_H

#include <Arduino.h>

class L2Unit
{
public:
    L2Unit(HardwareSerial *serial);
    int read();
    void Flush();
    void init();

    int tof_values[2];
    int loadcell_values[2];

    void print(HardwareSerial *serial);

private:
    HardwareSerial *_serial;
    const int receiveSize = 6; //ヘッダーをのぞいたバイト数
};

#endif
