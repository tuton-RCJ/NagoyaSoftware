#include "front.h"
uint16_t Front::values[5];  // 静的配列の定義
HardwareSerial* Front::serial = nullptr;  // HardwareSerial ポインタの定義
PacketSerial Front::packetSerial;  // PacketSerial のインスタンスの定義
void Front::init(HardwareSerial *serial)
{
    Front::serial = serial;
    serial->begin(115200);
    packetSerial.setStream(serial);
    packetSerial.setPacketHandler(Front::OnPacketReceived);
    for(int i = 0; i < NUM_VALUES; i++)
    {
        values[i] = 0;
    }
}
void Front::update()
{
    packetSerial.update();
}

void Front::OnPacketReceived(const uint8_t *buffer, size_t size)
{
    if (size == NUM_VALUES * sizeof(uint16_t))
    {
        memcpy(values, buffer, size);
    }
}

void Front::print(HardwareSerial *printSerial)
{
    printSerial->print("Front: ");
    for (int i = 0; i < NUM_VALUES; i++)
    {
        printSerial->print(values[i]);
        printSerial->print(" ");
    }
    printSerial->println();
}