#include <Arduino.h>
#include "../device/device.h"
#include "mydef.h"

#define SerchTurnDirection 1 // 0:左, 1:右

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
extern bool GetFrontObject(int distance);
extern void Kabeyoke(bool isWallleft);
extern void BallDrop();
extern void tremble(int times);

extern void ExitSetup();
extern bool ExitLoop();

extern void DriveUntilWall();
//-----------------------------------------

int status = 0; // 0=銀を探す、1=黒を探す、2=脱出
int rotation = 0;

bool InEntrance;     // 入口にいるかどうか
bool VictimDetected; // 被災者が検出されたかどうか
bool ZoneDetected;   // 避難ゾーンが検出されたかどうか
bool HaveVictim;     // 被災者を持っているかどうか

bool TurnedHalf;
bool TurnedOnce;
int victimCount;
bool isFrontCorner;

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
    l2unit.setCameraTarget(0); // 始めは銀を探す
    l2unit.setCameraCW();
    sts3032.straight(50, 150);
    sts3032.turn(50, -90);
    bno.setZero();
    TurnedHalf = false;
    TurnedOnce = true;
    victimCount = 0;
    isFrontCorner = true; // 入口も見ては行けないため、スルー。
    front.end();
}

void RescueLoop()
{
    if (status < 2)
    {
        if (!RescueVictim(status)) // 2周回って見つからなかったら次のタスクに移動。
        {
            sts3032.stop();
            status++;
            l2unit.setCameraTarget(2);
            l2unit.setCameraCW();
            buzzer.DetectedBlackBall();
            bno.setZero();
            if (status == 2)
            {
                if (!isFrontCorner)
                {
                    DetectCorner(1);
                    DriveUntilWall();
                    sts3032.straight(60, -40);
                    sts3032.turn(70, 180);
                }
                ExitSetup();
            }
        }
    }
    else
    {
        if (ExitLoop())
        {
            isRescue = false;
            // buzzer.kouka();
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

    bno.read();
    if (isFrontCorner && bno.direction > 180)
    {
        sts3032.drive(80, 100 * SerchTurnDirection);
    }
    else if (l2unit.read() && l2unit.OpenMVData > 20 && l2unit.OpenMVData < 140)
    {
        sts3032.stop();
        unsigned long start = millis();
        while (!(l2unit.read() && l2unit.OpenMVData != 255))
        {
            if (millis() - start >= 200)
            {
                return true;
            }
        }
        if (target == 0)
        {
            buzzer.DetectedSilverBall();
        }
        else
        {
            buzzer.DetectedBlackBall();
        }
        VictimDetected = true;
        TurnedHalf = false;
        TurnedOnce = false;
        sts3032.turn(50, XtoTurnRate(l2unit.OpenMVData));
        l2unit.setCameraPcontrol();
        Flush();
        isFrontCorner = false;
        delay(300);

        front.begin();
        return true;
    }
    else
    {
        sts3032.drive(30, 100 * SerchTurnDirection);
    }

    if (bno.direction > 160 && bno.direction < 200)
    {
        TurnedHalf = true;
    }
    if (TurnedHalf && bno.direction < 30)
    {
        TurnedHalf = false;
        if (TurnedOnce)
        {

            TurnedOnce = false;
            return false;
        }
        else
        {
            sts3032.stop();
            sts3032.turn(70, 90 * SerchTurnDirection);
            sts3032.straight(80, 400);
            isFrontCorner = false;
            TurnedOnce = true;
            return true;
        }
    }
    return true;
}

/// @brief 被災者を拾う
/// @param target 0: 銀, 1: 黒
/// @return true
bool PickUpVictim(int target) // target 0: 銀, 1: 黒
{

    if (GetFrontObject(35))
    {
        sts3032.stop();
        front.end();
        // sts3032.straight(30, 30);
        sts3032.straight(50, -130); // 下がる
        // アームを下す
        l2unit.AttachHand();
        l2unit.HandOpen();
        delay(300);
        l2unit.ArmDown();
        delay(600);
        Flush();
        front.begin();
        // 進む
        sts3032.drive(50, 0);
        while (true)
        {
            bno.read();
            if (bno.pitch > 8)
            {
                sts3032.straight(50, -40);
            }
            l2unit.read();
            if (l2unit.touch[0] && l2unit.touch[1])
            {
                sts3032.stop();
                sts3032.straight(50, -30);
                sts3032.straight(50, 30);
                delay(500);
                sts3032.straight(20, -5);
                sts3032.stop();
                break;
            }
            else if (l2unit.touch[0])
            {
                sts3032.drive(50, -75);
            }
            else if (l2unit.touch[1])
            {
                sts3032.drive(50, 75);
            }
            else
            {
                if (GetFrontObject(40))
                {
                    sts3032.stop();
                    sts3032.straight(30, -30);
                    break;
                }
                sts3032.drive(50, 0);
            }
        }
        front.end();
        sts3032.stop();
        delay(500);

        l2unit.HandClose();
        delay(1200);
        sts3032.straight(30, -30);
        l2unit.ArmUp();
        delay(600);
        l2unit.DetachHand();

        tremble(2);

        HaveVictim = true;
        VictimDetected = false;
        victimCount++;
        l2unit.setCameraTarget(target * 2 + 1);
        l2unit.setCameraCW();
        delay(500);
        Flush();
    }
    else
    {
        if (l2unit.read() && l2unit.OpenMVData != 255)
        {
            Pcontrol(l2unit.OpenMVData);
        }
        else
        {
            sts3032.drive(40, 0);
        }
    }
    return true;
}

/// @brief 避難所を探す
/// @param target 0: 緑, 1: 赤
/// @return true
bool DetectCorner(int target)
{
    sts3032.drive(50, 100 * SerchTurnDirection);

    if (l2unit.read() && l2unit.OpenMVData > 50 && l2unit.OpenMVData < 110)
    {
        if (l2unit.OpenMVData > 80)
        {
            sts3032.drive(20, -100);
        }
        else
        {
            sts3032.drive(20, 100);
        }
        sts3032.drive(20, 100 * SerchTurnDirection);
        while (!(l2unit.read() && l2unit.OpenMVData > 70 && l2unit.OpenMVData < 90))
            ;

        sts3032.stop();
        buzzer.DetectedGreenCorner();
        // sts3032.turn(40, XtoTurnRate(l2unit.OpenMVData));
        ZoneDetected = true;
        if (target == 0 && victimCount < 2)
        {
            // 銀があるか確認
            l2unit.setCameraTarget(0);
            l2unit.setCameraIdling();
            delay(1000);
            l2unit.Flush();
            for (int i = 0; i < 10; i++)
            {
                while (!l2unit.read())
                    ;
                if (l2unit.OpenMVData != 255)
                {
                    sts3032.turn(40, XtoTurnRate(l2unit.OpenMVData));
                    front.begin();

                    // 銀の回収に戻る
                    VictimDetected = true;
                    HaveVictim = false;
                    ZoneDetected = false;
                    return true;
                }
            }

            // 黒があるか確認
            l2unit.setCameraTarget(2);
            l2unit.setCameraIdling();
            delay(100);
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
            l2unit.setCameraPcontrol();
            l2unit.setCameraIdling();
            delay(100);
            l2unit.Flush();
        }
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
        sts3032.straight(60, -50);
        sts3032.turn(70, 180);
        sts3032.drive(-60, 0);
        delay(800);
        sts3032.stop();

        // ボールを落とす
        BallDrop();

        HaveVictim = false;
        ZoneDetected = false;

        isFrontCorner = true;
        if (target == 0)
        {
            sts3032.straight(60, 50);

            sts3032.turn(50, -90 * SerchTurnDirection);
            if (victimCount >= 2)
            {
                return false;
            }
            l2unit.setCameraTarget(0);
            l2unit.setCameraCW();
            delay(500);
            bno.setZero();
            Flush();
            return true;
        }
        else
        {
            sts3032.straight(60, 30);

            Flush();
            return false;
        }
    }
    bno.read();
    if (bno.pitch > 15)
    {
        sts3032.straight(60, -100);
    }
    if (l2unit.loadcell_detected[0])
    {
        sts3032.drive(60, -50);
    }
    else if (l2unit.loadcell_detected[1])
    {
        sts3032.drive(60, 50);
    }
    else
    {
        sts3032.drive(60, 0);
    }

    return true;
}