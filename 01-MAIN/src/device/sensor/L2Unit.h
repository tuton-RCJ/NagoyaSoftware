#ifndef L2Unit_H
#define L2Unit_H

#include <Arduino.h>

class L2Unit
{
public:
    L2Unit(HardwareSerial *serial);
    void init();
    bool read();
    void Flush();
    void print(HardwareSerial *serial);

    // received data
    int tof_values[2];
    int loadcell_values[2];    // 0~254 1023を1/4に圧縮。
    bool loadcell_detected[2]; // ロードセルの閾値を超えたかどうか
    int OpenMVData;            // 0~160, 255=not found

    // send message
    void setCameraTarget(int target); // 0=銀、1=緑、2=黒、3=赤

    void ArmDown();
    void ArmUp();
    void HandOpen();
    void HandClose();
    void BasketOpen();
    void BasketClose();
    void AttachHand();
    void DetachHand();

private:
    HardwareSerial *_serial;
    const int receiveSize = 7;              // ヘッダーをのぞいたバイト数
    int loadcell_threshold[2] = {155, 155}; // ロードセルの閾値
};

#endif
