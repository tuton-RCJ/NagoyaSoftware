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

extern void Flush();
//-----------------------------------------
// extern rescue modules
extern void GoRandomPosition();
extern void Pcontrol(int x);
extern int XtoTurnRate(int x);
extern int GetFrontObject();
extern void Kabeyoke(bool isWallleft);
extern void BallDrop();
extern void SetEntranceWallRight();
extern void tremble(int times);

extern void ExitSetup();
extern bool ExitLoop();
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
        if (!RescueVictim(status)) // 2周回って見つからなかったら次のタスクに移動。
        {
            status++;
            buzzer.DetectedBlackBall();
            if (status == 2)
            {
                ExitSetup();
            }
        }
    }
    else
    {
        if (ExitLoop())
        {
            isRescue = false;
            return;
        }
    }
}

bool RescueVictim(int target) // target 0: 銀, 1: 黒
{
    if (HaveVictim)
    {
        if (ZoneDetected)
        {
            return PlaceVictim(target);
        }
        else
        {
            return DetectCorner(target);
        }
    }
    else if (VictimDetected)
    {
        return PickUpVictim(target);
    }
    else
    {
        return DetectVictim(target);
    }
}

/// @brief 被災者を探す
/// @param target 0: 銀, 1: 黒
/// @return true: 被災者を発見, false: 被災者を発見できなかった
bool DetectVictim(int target)
{
    if (l2unit.read() && l2unit.OpenMVData > 40 && l2unit.OpenMVData < 120)
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

/// @brief 被災者を拾う
/// @param target 0: 銀, 1: 黒
/// @return true
bool PickUpVictim(int target) // target 0: 銀, 1: 黒
{
    if (GetFrontObject() < 40)
    {
        sts3032.stop();
        sts3032.straight(30, -30); // 下がる

        // アームを下す
        l2unit.AttachHand();
        l2unit.HandOpen();
        delay(300);
        l2unit.ArmDown();
        delay(600);

        // 進む
        sts3032.straight(30, 30);
        l2unit.HandClose();
        delay(300);
        l2unit.ArmUp();
        delay(600);
        l2unit.DetachHand();

        tremble(3);

        HaveVictim = true;
        VictimDetected = false;
        l2unit.setCameraTarget(target * 2 + 1);
    }
    else
    {
        if (l2unit.read() && l2unit.OpenMVData != 255)
        {
            Pcontrol(l2unit.OpenMVData);
        }
        else
        {
            sts3032.drive(30, 0);
        }
    }
    return true;
}

/// @brief 避難所を探す
/// @param target 0: 緑, 1: 赤
/// @return true
bool DetectCorner(int target)
{
    sts3032.drive(20, -100);

    if (l2unit.read() && l2unit.OpenMVData > 40 && l2unit.OpenMVData < 120)
    {
        sts3032.stop();
        buzzer.DetectedGreenCorner();
        sts3032.turn(40, XtoTurnRate(l2unit.OpenMVData));
        ZoneDetected = true;
    }
    if (target == 0)
    {
        // 銀があるか確認
        l2unit.setCameraTarget(0);
        delay(1500);
        l2unit.Flush();
        for (int i = 0; i < 10; i++)
        {
            while (!l2unit.read())
                ;
            if (l2unit.OpenMVData != 255)
            {
                sts3032.turn(40, XtoTurnRate(l2unit.OpenMVData));

                // 銀の回収に戻る
                VictimDetected = true;
                HaveVictim = false;
                ZoneDetected = false;
                return true;
            }
        }

        // 黒があるか確認
        l2unit.setCameraTarget(2);
        delay(1500);
        l2unit.Flush();
        for (int i = 0; i < 10; i++)
        {
            while (!l2unit.read())
                ;
            if (l2unit.OpenMVData > 40 && l2unit.OpenMVData < 120)
            {
                // 黒の処理を入れる
                // ToFが反応するまでまっすぐ進む
                // どける処理
                // targetを緑に戻す
                return true;
            }
        }
        l2unit.setCameraTarget(1);
        delay(500);
        l2unit.Flush();
    }

    return true;
}

/// @brief 被災者を避難所に置く
/// @param target 0: 緑, 1: 赤
/// @return true
bool PlaceVictim(int target)
{
    while (!l2unit.read())
        ;
    if (l2unit.loadcell_detected[0] && l2unit.loadcell_detected[1]) // 両側が反応したとき
    {
        // 避難所に後ろをつける
        sts3032.stop();
        sts3032.straight(30, -50);
        sts3032.turn(50, 180);
        sts3032.drive(-30, 0);
        delay(2000);
        sts3032.stop();

        // ボールを落とす
        BallDrop();

        HaveVictim = false;
        ZoneDetected = false;
        sts3032.straight(30, 100);
        Flush();
        if (target == 0)
        {
            l2unit.setCameraTarget(2);
            return true;
        }
        else
        {
            return false;
        }
    }
    sts3032.drive(40, 0);
    return true;
}