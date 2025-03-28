#include <Arduino.h>
#include "../device/device.h"
#include "mydef.h"
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
extern bool GetFrontObject(int distance);

void ExitSetup();
bool ExitLoop();

void DriveUntilWall();

void ExitSetup()
{
    sts3032.stop();
    sts3032.turn(30, -90 * ExitTurnDirection);
    Flush();
    for (int i = 0; i < 5; i++)
    {
        tof.read();
        delay(50);
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

/// @brief 脱出のループ
/// @return 抜け出すならtrue そうでないならfalse
bool ExitLoop()
{
    tof.read();

    // 左に壁がない場合、ラインを見て入口/出口を判断。
    if (tof.values[0] > 200)
    {
        sts3032.straight(50, 120);
        sts3032.turn(50, -90 * ExitTurnDirection);
        unsigned long start = millis();
        sts3032.drive(40, 0);
        while (millis() - start < 2000)
        {
            if (ReadLine() == 1) // 黒を読んだ。脱出！！
            {
                sts3032.stop();
                sts3032.straight(30, 30);
                return true;
            }
            if (ReadLine() == 2) // 残念、銀でした。
            {
                sts3032.stop();
                sts3032.straight(30, -50);
                sts3032.turn(50, 90 * ExitTurnDirection);
                delay(500);
                Flush();
                if (GetFrontObject(200)) // 前方が壁。
                {
                    DriveUntilWall();
                    sts3032.straight(30, -50);
                    // 90度右に回る。
                    sts3032.turn(40, 90 * ExitTurnDirection);
                    sts3032.stop();
                    delay(100);
                    Flush();
                    return false;
                }
                sts3032.straight(50, 200);
                Flush();
                return false;
            }
        }
        sts3032.stop();
        sts3032.straight(30, -40);
        sts3032.turn(50, 90 * ExitTurnDirection);
        delay(500);
        Flush();
        if (GetFrontObject(200)) // 前方が壁。
        {
            DriveUntilWall();
            // 90度右に回る。
            sts3032.turn(40, 90 * ExitTurnDirection);
            sts3032.stop();
            delay(1000);
            Flush();
            return false;
        }
        sts3032.straight(50, 200);
        Flush();
        return false;
    }
    else
    {

        // 定数制御にするか、比例制御にするかはあなた次第。
        sts3032.drive(60, 0);
        int readLine = ReadLine();
        if (readLine == 1)
        { // 黒を読んだ！脱出！
            sts3032.stop();
            sts3032.straight(30, 30);
            return true;
        }
        if (readLine == 2)
        { // 銀を読んだ！
            sts3032.stop();
            sts3032.straight(30, -50);
            sts3032.turn(50, 90 * ExitTurnDirection);
            sts3032.straight(50, 300);
            Flush();
            return false;
        }
        while (!l2unit.read())
            ;
        // バンパーが反応した時。
        if (l2unit.loadcell_detected[0] || l2unit.loadcell_detected[1])
        {
            delay(400);
            l2unit.Flush();
            while (!l2unit.read())
                ;
            if (l2unit.loadcell_detected[0] && l2unit.loadcell_detected[1]) // 壁
            {
                sts3032.stop();
                sts3032.straight(30, -50);
                sts3032.turn(50, 90 * ExitTurnDirection);
                Flush();
                return false;
            }
            else if (l2unit.loadcell_detected[0])
            {
                // 45度。避難所。両方反応するまで壁に押し当てて90度回転するようにする。
                sts3032.stop();
                sts3032.drive(50, -40 * ExitTurnDirection);
                delay(2000);
                DriveUntilWall();
                sts3032.stop();
                sts3032.straight(30, -30);
                sts3032.turn(50, 90 * ExitTurnDirection);
                Flush();
                return false;
            }
            else
            {
                return false;
                // これは！！！障害物です！！！どうしましょうね。
            }
        }
        return false;
    }
}

void DriveUntilWall()
{
    sts3032.drive(40, 0);
    while (!(l2unit.read() && l2unit.loadcell_detected[0] && l2unit.loadcell_detected[1]))
        ;
    delay(500);
    sts3032.stop();
}