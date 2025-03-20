#include <Arduino.h>
#include "./device/device.h"
#define PI 3.14159265358979323846

extern HardwareSerial uart1; // USB

extern Buzzer buzzer;
extern STS3032 sts3032;

extern LineUnit line;
extern ToF tof;
extern BNO055 bno;

extern L2Unit l2unit;

extern bool isRescue;

//-----------------------------------------

// todo tof の値は適当なのでものけんで調節する
//      ブザー音を鳴らすようにする

int EntrancePositon = 2; // 入口が右側にあるかどうか 0: 右側, 1: 左側, 2: 未確定

int status = 0; // 0=銀を探す、1=黒を探す、2=脱出
int rotation = 0;

int NowAngle;
int TargetX;         // 目標の x 座標
int midX = 16;       // カメラの中央の x 座標
bool InEntrance;     // 入口にいるかどうか
bool VictimDetected; // 被災者が検出されたかどうか
bool ZoneDetected;   // 避難ゾーンが検出されたかどうか
bool NearbyVictim;   // 被災者の目の前にいるか
bool HaveVictim;     // 被災者を持っているかどうか
int HFOV = 60;       // OpenMV の水平視野角

int PGain = 5; // カメラから送られてきた重心を基準に前に進む時のP 制御のゲイン

bool EntranceWallRight = false; // 入口の右側に壁があるかどうか

int SaveVictimXY[2]; // OpenMV から送られてきた生存者情報
int SaveVictimZone[2];

void BallDrop();

bool GetVictimData(int flag);

void Pcontrol(int x);

void GoRandomPosition();

void GoNextDetection();

int XtoTurnRate(int x);

bool GetFrontObject();

void BasketLock();

void ReadTopTof();
int TopTof[2];

void StringToIntValues(String str, int values[]);

bool RescueVictim(int target);

void Kabeyoke(bool isWallleft);

void RescueSetup()
{
    VictimDetected = false;
    ZoneDetected = false;
    NearbyVictim = false;
    HaveVictim = false;
    InEntrance = true;
    NowAngle = 0;
    line.setBrightness(0);
    l2unit.setCameraTarget(0);
    buzzer.EnterEvacuationZone();
    sts3032.straight(50, 150);
}

void RescueLoop()
{
    if (status < 2)
    {
        if (!RescueVictim(status) && rotation > 360)
        {
            status++;
            buzzer.DetectedBlackBall();
            if (status == 2)
            {
                // ExitSetup();
            }
        }
    }
    else
    {
        // ExitLoop();
    }
}

void BallDrop()
{
    l2unit.BasketOpen();
    delay(1000);
    l2unit.BasketClose();
    delay(1000);
}

void BasketLock()
{
}

int XtoTurnRate(int x)
{
    return -(HFOV * (midX - x) / (2 * midX));
}

void Pcontrol(int x)
{
    sts3032.drive(20, (midX - x) * PGain);
}

// // ランダムな位置に移動する
// void GoRandomPosition()
// {
//     tof.getTofValues();
//     while (tof.tof_values[2] < 500)
//     {
//         int angle = random(90, 270);
//         sts3032.turn(50, angle);
//         tof.getTofValues();
//     }
//     while (tof.tof_values[2] > 500)
//     {
//         sts3032.drive(50, 0);
//         tof.getTofValues();
//     }
// }

// void GoNextDetection()
// {
//     if (InEntrance)
//     {
//         if (NowAngle < 90)
//         {
//             if (EntrancePositon == 0)
//             {
//                 sts3032.turn(50, -45);
//             }
//             else if (EntrancePositon == 1)
//             {
//                 sts3032.turn(50, 45);
//             }
//             else if (EntrancePositon == 2)
//             {
//                 tof.read();
//                 if (tof.tof_values[5] < 400)
//                 { // 壁方向に回転することを防ぐ　
//                     sts3032.turn(50, -135);
//                     EntrancePositon = 0;
//                 }
//                 else
//                 {
//                     sts3032.turn(50, -45);
//                     EntrancePositon = 1;
//                 }
//             }
//             NowAngle += 45;
//             return;
//         }
//         else
//         {
//             InEntrance = false;
//             GoRandomPosition();
//         }
//     }
//     else if (NowAngle < 360)
//     {
//         sts3032.turn(50, 45);
//         NowAngle += 45;
//     }
//     else
//     {
//         GoRandomPosition();
//         NowAngle = 0;
//     }
// }

bool GetFrontObject()
{
    Front::read();
    for (int i = 0; i < 3; i++)
    {
        if (Front::values[i] < 60)
        {
            return true;
        }
    }
    return false;
}

bool RescueVictim(int target) // target 0: 銀, 1: 黒
{
    // 生存者を回収済みでゾーン未検出
    if (HaveVictim && !ZoneDetected)
    {
        l2unit.setCameraTarget(target * 2 + 1);

        int turnRate = 30;
        sts3032.drive(20, -100);
        while (true)
        {
            l2unit.read();
            if (l2unit.OpenMVData > 40 && l2unit.OpenMVData < 120)
            {
                buzzer.DetectedGreenCorner();
                sts3032.stop();
                sts3032.turn(40, XtoTurnRate(l2unit.OpenMVData));
                ZoneDetected = true;
                break;
            }
        }
        return true;
    }
    // 生存者を回収済みでゾーンも見つけた
    else if (HaveVictim && ZoneDetected)
    {
        l2unit.read();
        if (l2unit.loadcell_values[0] > 155 && l2unit.loadcell_values[1] > 155)
        {
            sts3032.stop();
            sts3032.straight(30, -50);
            sts3032.turn(50, 180);
            sts3032.drive(-30, 0);
            delay(2000);
            sts3032.stop();
            BallDrop();
            HaveVictim = false;
            ZoneDetected = false;
            sts3032.straight(30, 100);
            return true;
        }
        sts3032.drive(40, 0);
    }
    // 生存者を発見していない
    else if (!VictimDetected)
    {
        l2unit.read();
        l2unit.print(&uart1);
        if (InEntrance)
        {
            tof.read();
            if (tof.tof_values[0] > tof.tof_values[1])
            {
                EntranceWallRight = true;
                buzzer.beep(440, 0.5);
            }
            else
            {
                EntranceWallRight = false;
                buzzer.beep(880, 0.5);
            }
            InEntrance = false;
            if (EntranceWallRight)
            {
                sts3032.drive(20, -100);
            }
            else
            {
                sts3032.drive(20, 100);
            }
        }
        if (l2unit.OpenMVData > 20 && l2unit.OpenMVData < 140)
        {
            sts3032.stop();
            if (target == 0)
            {

                buzzer.DetectedSilverBall();
            }
            else
            {
                buzzer.DetectedBlackBall();
            }
            VictimDetected = true;
            sts3032.turn(50, XtoTurnRate(l2unit.OpenMVData));
        }
        else
        {
            if (EntranceWallRight)
            {
                sts3032.drive(10, -100);
            }
            else
            {
                sts3032.drive(10, 100);
            }
            return false;
        }
    }
    // 生存者を発見しているが未回収
    else
    {
        if (!NearbyVictim)
        {
            if (GetFrontObject())
            {
                sts3032.stop();
                NearbyVictim = true;
            }
            else
            {

                sts3032.drive(20, 0);
            }
        }
        else
        {

            l2unit.ArmDown();
            delay(600);
            l2unit.ArmUp();
            delay(600);
            HaveVictim = true;
            NearbyVictim = false;
            VictimDetected = false;
        }
    }
    return true;
}

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