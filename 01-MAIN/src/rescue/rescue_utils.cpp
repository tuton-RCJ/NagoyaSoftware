#include <Arduino.h>
#include "../device/device.h"

//---------------------------------------
// extern modules
extern HardwareSerial uart1;

extern Buzzer buzzer;
extern STS3032 sts3032;

extern LineUnit line;
extern ToF tof;
extern BNO055 bno;

extern L2Unit l2unit;
extern Front front;

//-----------------------------------------

//-----------------------------------------

const int midX = 80; // カメラの中央の x 座標
const int HFOV = 60; // OpenMV の水平視野角
const int PGain = 1; // カメラから送られてきた重心を基準に前に進む時のP 制御のゲイン

int XtoTurnRate(int x);
int GetFrontObject();

void Pcontrol(int x);
void Kabeyoke(bool isWallleft);
void BallDrop();
void tremble(int times);

int ReadLine();
extern void DriveUntilWall();

extern void Flush();
/// @brief 進むときに壁への衝突を防ぐ
/// @param isWallLeft　壁が左側にあるかどうか
void Kabeyoke(bool isWallLeft)
{
    if (isWallLeft)
    {
        sts3032.turn(60, 40);
        sts3032.straight(50, 30);
        sts3032.turn(60, -40);
    }
    else
    {
        sts3032.turn(60, -40);
        sts3032.straight(50, 30);
        sts3032.turn(60, 40);
    }
}

void BallDrop()
{
    l2unit.BasketOpen();
    delay(500);
    // tremble(3);
    delay(500);
    l2unit.BasketClose();
    delay(1000);
}

/// @brief 与えられたカメラ上の x 座標に対して、その x 座標に対応する旋回量を計算する
/// @param x 0~160 の x 座標
/// @return
int XtoTurnRate(int x)
{
    return (HFOV * (midX - x) / (2 * midX));
}

/// @brief 与えられたカメラ上の x 座標に対して P 制御を行う
/// @param x 0~160 の x 座標
void Pcontrol(int x)
{
    sts3032.drive(40, (midX - x) * PGain);
}

/// @brief 正面の物体までの距離を取得する
/// @return ToFセンサの最小値
bool GetFrontObject(int distance)
{
    int _detectedCount = 0;
    for (int i = 0; i < 5; i++)
    {
        while (!front.read())
            ;
        int returnVal = front.values[0];
        for (int i = 1; i < 5; i++)
        {
            if (front.values[i] < returnVal)
            {
                returnVal = front.values[i];
            }
        }
        if (returnVal < distance)
        {
            _detectedCount++;
        }
    }
    if (_detectedCount >= 2)
    {
        return true;
    }
    return false;
}

void tremble(int times)
{
    int speed = 50;
    for (int i = 0; i < times; i++)
    {
        sts3032.drive(speed, 0);
        delay(200);
        sts3032.drive(-speed, 0);
        delay(200);
    }
}

void GoNextCorner()
{
    sts3032.drive(50, 0);
    Flush();
    while (true)
    {
        if (ReadLine() > 0)
        {
            sts3032.stop();
            sts3032.straight(30, -60);
            tof.read();
            if (tof.values[1] < 150)
            {
                sts3032.turn(40, 90);
                DriveUntilWall();
                sts3032.straight(40, -50);
                sts3032.turn(40, 180);
            }
            else
            {
                sts3032.turn(40, -90);
            }
            return;
        }

        if (l2unit.read() && l2unit.loadcell_detected[0] && l2unit.loadcell_detected[1])
        {
            sts3032.stop();
            sts3032.straight(30, -30);
            sts3032.turn(30, -90);
            return;
        }
    }
}

/// @brief ラインを読み取る。
/// @return 0=白、1=黒、2=銀
int ReadLine()
{
    while (!line.read())
        ;

    for (int i = 0; i < 15; i++)
    {
        if (line.photoReflector[i] == 1)
        {
            return 1;
        }
        else if (line.photoReflector[i] == 2)
        {
            return 2;
        }
    }

    return 0;
}
