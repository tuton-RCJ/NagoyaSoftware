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
extern int GetFrontObject();

void ExitSetup();
bool ExitLoop();

void ExitSetup()
{
    sts3032.stop();
    sts3032.turn(50, -90); // 時計回りにまわる準備。
}


/// @brief ラインを読み取る。
/// @return 0=白、1=黒、2=銀
int ReadLine()
{
    if (line.read())
    {
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
    }
    return 0;
}

/// @brief 脱出のループ
/// @return 抜け出すならtrue そうでないならfalse
bool ExitLoop()
{
    tof.read();

    // 左に壁がある場合、ラインを見て入口/出口を判断。
    if (tof.values[0] > 150)
    {
        sts3032.straight(50, 100);
        sts3032.turn(50, -90);
        unsigned long start = millis();
        sts3032.drive(30, 0);
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
                sts3032.turn(50, 90);
                if (GetFrontObject() < 200) // 前方が壁。
                {
                    // 前50mmまで進む。
                    while (GetFrontObject() < 50)
                    {
                        sts3032.drive(30, 0);
                    }
                    sts3032.stop();
                    // 90度右に回る。
                    sts3032.turn(40, 90);
                    return false;
                }
                sts3032.straight(50, 200);
                return false;
            }
        }
        sts3032.stop();
        sts3032.straight(30, -150);
        sts3032.turn(50, 90);
        return false;
    }
    else
    {
        tof.read();

        // 定数制御にするか、比例制御にするかはあなた次第。
        // sts3032.drive(50, 0);
        sts3032.drive(50, -(tof.values[0] - 50)); // 値は適当。要調整。

        if (ReadLine() == 1)
        { // 黒を読んだ！脱出！
            sts3032.stop();
            sts3032.straight(30, 30);
            return true;
        }
        if (ReadLine() == 2)
        { // 銀を読んだ！
            sts3032.stop();
            sts3032.straight(30, -50);
            sts3032.turn(50, 90);
            sts3032.straight(50, 300);
            return false;
        }
        l2unit.read();

        // バンパーが反応した時。
        if (l2unit.loadcell_detected[0] || l2unit.loadcell_detected[1])
        {
            delay(800);
            l2unit.Flush();
            while (!l2unit.read())
                ;
            if (l2unit.loadcell_detected[0] && l2unit.loadcell_detected[1]) // 壁
            {
                sts3032.stop();
                sts3032.straight(30, -50);
                sts3032.turn(50, 90);
                return false;
            }
            else if (l2unit.loadcell_detected[0])
            {
                // 45度。避難所。これで上手くいかなかったら、両方反応するまで壁に押し当てて90度回転するようにする。
                sts3032.stop();
                sts3032.turn(50, 45);
                return false;
            }
            else
            {
                // これは！！！障害物です！！！どうしましょうね。
            }
        }
    }
}