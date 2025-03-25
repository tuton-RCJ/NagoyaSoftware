#include <Arduino.h>
#include "./device/device.h"
#include "mydef.h"

// communication
extern HardwareSerial uart1;

// Sensor
// extern LoadCell loadcell;
extern LineUnit line;
extern ToF tof;
extern BNO055 bno;

// Actuatr
extern STS3032 sts3032;
// extern Microservo servo;
extern Buzzer buzzer;
extern L2Unit l2unit;
extern bool isRescue;
//----------------------------------------------

// ライントレース PID用に変数を用意しているがP制御しかしていない
int Kps[15] = {-7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7}; // 外側のゲインを大きくするための係数

int Kp = 15; // 12
int Kd = 0;  // 10
int Ki = 0;
int lastError = 0;
int sumError = 0;           // 積分値
int speed;                  // 走行スピード
const int normalSpeed = 30; // 通常時のスピード
void LineTrace();           // フォトリフレクタの値を読みライントレース。銀を検知すればレスキューモードに移行
void CheckRed();            // 赤テープ検知
void CheckGreen();          // 緑マーカー検知

// 障害物回避
void CheckObject(); // ライントレース中、障害物があるか確かめる
bool TurningObject; // 障害物回避中モード
void TurnObject();  // 障害物回避中の処理
bool checkBlackLine(bool isLeft);

// ジャイロの値を読んで坂検知
void setSlopeStatus();
int SlopeStatus = 0; // 0:平坦 1:上り 2:下り

extern void Flush();

void LineSetup()
{
  sumError = 0;
  isRescue = false;
  TurningObject = false;
  // servo.initPos();
  line.init();
  line.setBrightness(80);
  Flush();
}

void LineLoop()
{

  if (TurningObject)
  {
    tof.read();
    TurnObject();
    return;
  }
  if (!line.read())
  {
    return;
  }
  LineTrace();
  if (isRescue)
    return;

  CheckRed();
  CheckGreen();
  setSlopeStatus();
  if (l2unit.read())
  {
    CheckObject();
  }
}

void LineTrace()
{
  int black_sum = 0;
  int black_cnt = 0;
  int pid = 0;
  bool isEdgeLBlack = false;
  bool isEdgeRBlack = false;
  int turnRate = 0;
  for (int i = 0; i < 15; i++)
  {
    if (line.photoReflector[i] == 1)
    {
      black_sum += Kps[i];
      black_cnt++;
      if (i == 0)
        isEdgeLBlack = true;

      if (i == 14)
        isEdgeRBlack = true;
    }

    if (line.photoReflector[i] == 2) // 銀を検知したらレスキューモードに移行
    {
      isRescue = true;
      sts3032.stop();
      return;
    }
  }
  float error = 0;
  if (black_cnt > 0)
  {
    error = black_sum / black_cnt;
  }
  if (isEdgeLBlack)
  {
    error = Kps[0];
  }
  if (isEdgeRBlack)
  {
    error = Kps[14];
  }
  if (isEdgeLBlack && isEdgeRBlack)
  {
    error = 0;
  }

  // トの字判定。前方に黒があり、外側のセンサーが反応している場合。直角をトの字と誤検知することがあるのでスピードを落としている。
  if (abs(black_sum) > 20 && line.frontPhotoReflector == 1)
  {
    error = 0;
    speed = 20;
  }
  else
  {
    speed = normalSpeed;
  }
  if (abs(error) <= 1)
  {
    speed = 60;
  }

  // PID制御
  sumError += error;
  pid = Kp * error + Ki * sumError + Kd * (error - lastError);
  lastError = error;

  turnRate = pid;
  // sts3032.drive(speed, turnRate);
  sts3032.LeftDrive(speed + turnRate, 0);
  sts3032.RightDrive(speed - turnRate, 0);
}

void CheckRed()
{
  if (line.colorLTime[3] > 0 && millis() - line.colorLTime[3] < 100)
  {
    sts3032.stop();
    buzzer.kouka();
  }
}

void CheckGreen()
{
  if ((line.LastColorL == 0 || line.LastColorR == 0)) // && line.frontPhotoReflector==1)
  {
    int p = 0;
    // if (line.LastColorL == 0)
    // {
    if (line.colorLTime[2] > 0 && millis() - line.colorLTime[2] < 400)
    {
      p += 1;
    }
    // }
    // if (line.LastColorR == 0)
    // {
    if (line.colorRTime[2] > 0 && millis() - line.colorRTime[2] < 400)
    {
      p += 2;
    }
    //}

    if (p > 0)
    {
      sts3032.stop();
      buzzer.GreenMarker(p);
      int MoveToFront = 60;
      if (SlopeStatus == 1)
      {
        MoveToFront = 100;
      }
      else if (SlopeStatus == 2)
      {
        MoveToFront = 20;
      }

      if (p == 1)
      {
        sts3032.straight(40, MoveToFront);
        sts3032.turn(30, -80);

        // sts3032.drive(50, -85);
        // delay(1000);
        sts3032.stop();
        if (SlopeStatus == 3)
        {
          sts3032.straight(30, 50);
        }
      }
      if (p == 2)
      {
        sts3032.straight(40, MoveToFront);
        sts3032.turn(30, 80);
        // sts3032.drive(50, 85);
        // delay(1000);
        sts3032.stop();
        if (SlopeStatus == 4)
        {
          sts3032.straight(30, 50);
        }
      }
      if (p == 3)
      {
        if (SlopeStatus == 3)
        {
          sts3032.turn(30, 90);
          sts3032.straight(50, -50);
          sts3032.turn(30, 90);
        }
        else if (SlopeStatus == 4)
        {
          sts3032.turn(30, 90);
          sts3032.straight(50, 50);
          sts3032.turn(30, 90);
        }
        else
        {
          sts3032.turn(50, 180);
        }

        sts3032.stop();
      }
      Flush();
    }
  }
}

void CheckObject()
{
  if (l2unit.loadcell_values[0] > 155 || l2unit.loadcell_values[1] > 155)
  {
    sts3032.stop();
    buzzer.ObjectDetected();
    sts3032.straight(50, -20);
    sts3032.turn(50, 80 * ObjectTurnDirection);

    sts3032.straight(50, 30);
    TurningObject = true;
    Flush();
  }
}

void TurnObject()
{
  bool blackFlag = false;

  if (tof.values[0] < 80)
  {
    // buzzer.beep(440, 0.5);
    if (tof.values[0] < 50)
    {
      sts3032.drive(30, 45 * ObjectTurnDirection);
    }
    else if (tof.values[0] < 70)
    {
      sts3032.drive(30, 0);
    }
    else
    {
      sts3032.drive(30, -30 * ObjectTurnDirection);
    }
    while (!line.read())
      ;
    for (int i = 0; i < 6; i++)
    {
      if (line.photoReflector[i] == 1)
      {
        blackFlag = true;
      }
    }
  }
  else
  {
    // buzzer.beep(880, 0.5);
    sts3032.turn(30, -30 * ObjectTurnDirection);
    unsigned long _start = millis();
    sts3032.drive(30, 0);
    while (millis() - _start < 900)
    {
      if (checkBlackLine(ObjectTurnDirection == 1))
      {
        blackFlag = true;
        break;
      }
    }
  }

  if (blackFlag)
  {
    sts3032.stop();
    buzzer.ObjectDetected();
    sts3032.straight(50, 30);
    sts3032.turn(50, 80 * ObjectTurnDirection);
    sts3032.straight(50, -60);
    sts3032.stop();
    TurningObject = false;
  }
  Flush();
}

void setSlopeStatus()
{
  bno.read();
  if (bno.pitch > 10)
  {
    SlopeStatus = 1; // 上り
  }
  else if (bno.pitch < -10)
  {
    SlopeStatus = 2; // 下り
  }
  else if (bno.roll > 10)
  {
    SlopeStatus = 3; // 右に傾いている
  }
  else if (bno.roll < -10)
  {
    SlopeStatus = 4; // 左に傾いている
  }
  else
  {
    SlopeStatus = 0;
  }
}

bool checkBlackLine(bool isLeft)
{
  if (!line.read())
  {
    return false;
  }
  if (isLeft)
  {
    for (int i = 0; i < 7; i++)
    {
      if (line.photoReflector[i] == 1)
      {
        return true;
      }
    }
  }
  else
  {
    for (int i = 8; i < 15; i++)
    {
      if (line.photoReflector[i] == 1)
      {
        return true;
      }
    }
  }
  return false;
}
