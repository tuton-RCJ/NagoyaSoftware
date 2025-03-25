#include <Arduino.h>
#include "../device/device.h"

//---------------------------------------
// extern modules
extern HardwareSerial uart1; // USB

extern Buzzer buzzer;
extern STS3032 sts3032;

extern LineUnit line;
extern ToF tof;
extern BNO055 bno;

extern L2Unit l2unit;
extern Front front;

extern bool isRescue;

//-----------------------------------------

extern int EntrancePositon; // 入口が右側にあるかどうか 0: 右側, 1: 左側, 2: 未確定

extern int status; // 0=銀を探す、1=黒を探す、2=脱出
extern int rotation;

extern int NowAngle;
extern int midX;            // カメラの中央の x 座標
extern bool InEntrance;     // 入口にいるかどうか
extern bool VictimDetected; // 被災者が検出されたかどうか
extern bool ZoneDetected;   // 避難ゾーンが検出されたかどうか
extern bool NearbyVictim;   // 被災者の目の前にいるか
extern bool HaveVictim;     // 被災者を持っているかどうか
extern int HFOV;            // OpenMV の水平視野角

extern int PGain; // カメラから送られてきた重心を基準に前に進む時のP 制御のゲイン

extern bool EntranceWallRight; // 入口の右側に壁があるかどうか

/// @brief ランダムな位置に移動する
void GoRandomPosition()
{
    sts3032.turn(30, 90);
    sts3032.drive(30, 100);
    while (!(front.read() && front.values[2] > 300))
        ;
    sts3032.stop();
    sts3032.drive(30, 0);
    unsigned long start = millis();
    while (!(front.read() && front.values[2] < 300) && millis() - start < 2000)
    {
        // 黒または銀を検知したらよける
        line.read();
        for (int i = 0; i < 15; i++)
        {
            if (line.photoReflector[i] != 0)
            {
                sts3032.stop();
                sts3032.straight(30, -20);
                sts3032.turn(30, 90);
                GoRandomPosition();
                break;
            }
        }
    }
    sts3032.stop();
}
