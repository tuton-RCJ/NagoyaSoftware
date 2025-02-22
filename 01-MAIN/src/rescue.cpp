#include <Arduino.h>
#include "./device/device.h"
#define PI 3.14159265358979323846

// communication
extern HardwareSerial uart1;
extern HardwareSerial uart4;
extern HardwareSerial uart6;

// Sensor
extern LoadCell loadcell;
extern LineUnit line;
extern ToF tof;
extern BNO055 bno;

// Actuatr
extern STS3032 sts3032;
extern Microservo servo;
extern Buzzer buzzer;

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
    buzzer.EnterEvacuationZone();
    line.setBrightness(0);
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
    servo.BasketOpen();
    delay(2000);
    servo.BasketClose();
    delay(500);
    servo.DetachBasketServo();
}

void BasketLock()
{
    servo.AttachBasketServo();
    servo.BasketClose();
    delay(500);
}

// uart6 には 1 or 0、 x座標 が送られる 避難ゾーンの場合は重心のX座標と幅が送られてくる
bool GetVictimData(int flag) // flag = 0: 生存者, = 1: 死亡者, 2: 生存者避難ゾーン, 3: 死亡者避難ゾーン
{
    uart6.flush();
    uart6.write(flag);

    if (flag == 0) // Silver
    {
        while (uart6.available() < 1)
            ;

        int IsValid = uart6.read();
        if (IsValid == 0)
        {
            return false;
        }
        while (uart6.available() < 1)
            ;
        int x = uart6.read();
        SaveVictimXY[0] = x;
        return true;
    }
    if (flag == 1) // Black
    {
        while (uart6.available() < 1)
            ;

        int IsValid = uart6.read();
        if (IsValid == 0)
        {
            return false;
        }
        while (uart6.available() < 1)
            ;
        int x = uart6.read();
        SaveVictimXY[0] = x;
        return true;
    }
    if (flag == 2) // Green
    {
        while (uart6.available() < 1)
            ;
        int IsValid = uart6.read();
        if (IsValid == 0)
        {
            return false;
        }
        while (uart6.available() < 2)
            ;
        int x = uart6.read();
        int w = uart6.read();
        SaveVictimZone[0] = x;
        SaveVictimZone[1] = w;
    }
    if (flag == 3) // Red
    {
        while (uart6.available() < 1)
            ;

        int IsValid = uart6.read();
        if (IsValid == 0)
        {
            return false;
        }
        while (uart6.available() < 2)
            ;
        int x = uart6.read();
        int w = uart6.read();
        SaveVictimZone[0] = x;
        SaveVictimZone[1] = w;
    }
    return true;
}

int XtoTurnRate(int x)
{
    return (HFOV * (midX - x) / (2 * midX));
}

void Pcontrol(int x)
{
    sts3032.drive(20, (midX - x) * PGain);
}

// ランダムな位置に移動する
void GoRandomPosition()
{
    tof.getTofValues();
    while (tof.tof_values[2] < 500)
    {
        int angle = random(90, 270);
        sts3032.turn(50, angle);
        tof.getTofValues();
    }
    while (tof.tof_values[2] > 500)
    {
        sts3032.drive(50, 0);
        tof.getTofValues();
    }
}

void GoNextDetection()
{
    if (InEntrance)
    {
        if (NowAngle < 90)
        {
            if (EntrancePositon == 0)
            {
                sts3032.turn(50, -45);
            }
            else if (EntrancePositon == 1)
            {
                sts3032.turn(50, 45);
            }
            else if (EntrancePositon == 2)
            {
                tof.getTofValues();
                if (tof.tof_values[5] < 400)
                { // 壁方向に回転することを防ぐ　
                    sts3032.turn(50, -135);
                    EntrancePositon = 0;
                }
                else
                {
                    sts3032.turn(50, -45);
                    EntrancePositon = 1;
                }
            }
            NowAngle += 45;
            return;
        }
        else
        {
            InEntrance = false;
            GoRandomPosition();
        }
    }
    else if (NowAngle < 360)
    {
        sts3032.turn(50, 45);
        NowAngle += 45;
    }
    else
    {
        GoRandomPosition();
        NowAngle = 0;
    }
}

bool GetFrontObject()
{
    int frontthreshold = 80;
    tof.getTofValues();
    for (int i = 2; i < 7; i += 2)
    {
        if (tof.tof_values[i] < frontthreshold)
        {
            return true;
        }
    }
    return false;
}

void ReadTopTof()
{
    uart4.flush();
    String str = uart4.readStringUntil('\n');
    str = uart4.readStringUntil('\n');

    StringToIntValues(str, TopTof);
}

void StringToIntValues(String str, int values[])
{
    int i = 0;
    int j = 0;
    while (i < str.length())
    {
        if (j > 2)
        {
            break;
        }
        if (str[i] == ' ')
        {
            i++;
            continue;
        }
        String value = "";
        while (str[i] != ' ' && i < str.length())
        {
            value += str[i];
            i++;
        }
        values[j] = value.toInt();
        j++;
    }
}

bool RescueVictim(int target)
{

    // 生存者を回収済みでゾーン未検出
    if (HaveVictim && !ZoneDetected)
    {
        // 　最も大きい角度のゾーンを検出
        int MaxI = -1;
        int MaxX = 0;
        int MaxW = 0;

        int turnRate = 30;

        for (int i = 0; i < 360 / turnRate; i++)
        {
            if (GetVictimData(target + 2))
            {
                buzzer.DetectedGreenCorner();
                if (SaveVictimZone[1] > MaxW)
                {
                    MaxI = i;
                    MaxX = SaveVictimZone[0];
                    MaxW = SaveVictimZone[1];
                }
            }
            sts3032.turn(30, turnRate);
            delay(50);
        }

        if (MaxI != -1)
        {
            sts3032.turn(50, MaxI * turnRate + XtoTurnRate(MaxX));
            ZoneDetected = true;
            if (target == 0)
            {

                buzzer.DetectedGreenCorner();
            }
            else
            {
                buzzer.DetectedRedCorner();
            }
            delay(1000);
        }
        else
        {
            buzzer.NotFound();
        }
        return true;
    }
    // 生存者を回収済みでゾーンも見つけた
    else if (HaveVictim && ZoneDetected)
    {
        loadcell.read();
        if (loadcell.values[0] > 200 && loadcell.values[1] > 200)
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

        if (GetFrontObject())
        {
            sts3032.drive(40, 0);
        }
        else
        {
            if (GetVictimData(target + 2)) // 重心についてP制御
            {
                Pcontrol(SaveVictimZone[0]);
            }
            else
            {
                sts3032.straight(20, 0);
            }
            ReadTopTof();
            if (TopTof[0] < 80)
            {
                Kabeyoke(true);
            }
            else if (TopTof[1] < 80)
            {
                Kabeyoke(false);
            }
        }
    }
    // 生存者を発見していない
    else if (!VictimDetected)
    {
        if (InEntrance)
        {
            tof.getTofValues();
            if (tof.tof_values[0] < tof.tof_values[1])
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
        }
        if (GetVictimData(target))
        {
            if (target == 0)
            {

                buzzer.DetectedSilverBall();
            }
            else
            {
                buzzer.DetectedBlackBall();
            }
            TargetX = SaveVictimXY[0];
            VictimDetected = true;
            sts3032.turn(50, XtoTurnRate(TargetX));
            servo.AttachArmServo();
            delay(500);
            servo.ArmDown();
            delay(1000);
        }
        else
        {
            if (EntranceWallRight)
            {
                sts3032.turn(50, -24);
            }
            else
            {
                sts3032.turn(50, 24);
            }
            rotation += 24;
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
                if (GetVictimData(target))
                {
                    Pcontrol(SaveVictimXY[0]);
                }
                else
                {
                    sts3032.drive(20, 0);
                }
            }
        }
        else
        {
            BasketLock();
            delay(500);
            servo.HandClose();
            delay(200);
            servo.HandClose();
            delay(200);
            servo.ArmUp();
            delay(1000);
            servo.DetachArmServo();
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