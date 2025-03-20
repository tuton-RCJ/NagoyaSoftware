#include <Arduino.h>
#include "../device/device.h"
#define PI 3.14159265358979323846

//---------------------------------------
// extern modules
extern HardwareSerial uart1; // USB

extern Buzzer buzzer;
extern STS3032 sts3032;
extern ToF tof;
extern BNO055 bno;

extern LineUnit line;
extern L2Unit l2unit;
extern Front front;

extern bool isRescue;

//-----------------------------------------
// extern rescue modules
extern void GoRandomPosition();
extern void Pcontrol(int x);
extern int XtoTurnRate(int x);
extern bool GetFrontObject(int threshold);
extern void Kabeyoke(bool isWallleft);
extern void BallDrop();
extern void SetEntranceWallRight();
//-----------------------------------------

int status = 0; // 0=銀を探す、1=黒を探す、2=脱出
int rotation = 0;

bool InEntrance;     // 入口にいるかどうか
bool VictimDetected; // 被災者が検出されたかどうか
bool ZoneDetected;   // 避難ゾーンが検出されたかどうか
bool HaveVictim;     // 被災者を持っているかどうか

bool EntranceWallRight = false; // 入口の右側に壁があるかどうか

bool RescueVictim(int target);

bool DetectVictim(int target); // 被災者を探す。 target:0=銀、1=黒
bool PickUpVictim(int target); // 被災者を拾う。 target:0=銀、1=黒
bool DetectCorner(int target); // 避難所を探す。 target:0=緑、1=赤
bool PlaceVictim(int target);  // 被災者を避難所に置く。 target:0=緑、1=赤

void RescueSetup()
{
    VictimDetected = false;
    ZoneDetected = false;
    HaveVictim = false;
    InEntrance = true;
    line.setBrightness(0);

    buzzer.EnterEvacuationZone();
    sts3032.straight(50, 150);
    l2unit.setCameraTarget(0); // 始めは銀を探す
}

void RescueLoop()
{
    if (status < 2)
    {
        if (!RescueVictim(status)) // 2周回って見つからなかったら次のタスクに移動っていう処理。
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
        return DetectVictim(target);
    }
    // 生存者を発見しているが未回収
    else
    {
        return PickUpVictim(target);
    }
}

bool DetectVictim(int target)
{
    if (l2unit.read() && l2unit.OpenMVData > 20 && l2unit.OpenMVData < 140)
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
        return true;
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
        return false; // これをあとで2周回ったらfalseに変える。
    }
}

bool PickUpVictim(int target) // target 0: 銀, 1: 黒
{
    if (GetFrontObject(50))
    {
        sts3032.stop();
        l2unit.ArmDown();
        delay(600);
        l2unit.ArmUp();
        delay(600);
        HaveVictim = true;
        NearbyVictim = false;
        VictimDetected = false;
        l2unit.setCameraTarget(target * 2 + 1);
    }
    else
    {
        if (l2unit.read() && OpenMVData != 255)
        {
            Pcontrol(OpenMVData);
        }
        else
        {
            sts3032.drive(20, 0);
        }
    }
    return true;
}