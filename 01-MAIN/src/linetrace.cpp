#include <Arduino.h>
#include "./device/device.h"

// communication
extern HardwareSerial uart1;

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
//----------------------------------------------

// ライントレース PID用に変数を用意しているがP制御しかしていない
int Kps[15] = {-8, -6, -4, -4, -3, -2, -2, 0, 2, 2, 3, 4, 4, 6, 8}; // 外側のゲインを大きくするための係数
int threshold = 800;
int front_threshould = 900;  // 白と黒の閾値
int silver_threshould = 100; // 銀の閾値
int Kp = 16;
int Kd = 0;
int Ki = 0;
int lastError = 0;
int sumError = 0;           // 積分値
int speed;                  // 走行スピード
const int normalSpeed = 40; // 通常時のスピード
void LineTrace();           // フォトリフレクタの値を読みライントレース。銀を検知すればレスキューモードに移行
void CheckRed();            // 赤テープ検知
void CheckGreen();          // 緑マーカー検知

// 障害物回避
void CheckObject(); // ライントレース中、障害物があるか確かめる
bool TurningObject; // 障害物回避中モード
void TurnObject();  // 障害物回避中の処理

// ジャイロの値を読んで坂検知
void setSlopeStatus();
int SlopeStatus = 0; // 0:平坦 1:上り 2:下り

void LineSetup()
{
  sumError = 0;
  isRescue = false;
  TurningObject = false;
  // servo.initPos();
  line.init();
  line.setBrightness(80);
}

void LineLoop()
{

  line.read();
  if (TurningObject)
  {
    tof.getTofValues();
    TurnObject();
    return;
  }
  LineTrace();
  if (isRescue)
    return;
  CheckRed();
  CheckGreen();
  setSlopeStatus();
  if (SlopeStatus != 2)
  {
    CheckObject();
  }
}

void LineTrace()
{
  int error = 0;
  int pid = 0;

  int turnRate = 0;
  for (int i = 0; i < 15; i++)
  {
    if (line._photoReflector[i] > threshold)
    {
      error += Kps[i];
    }

    if (line._photoReflector[i] < silver_threshould) // 銀を検知したらレスキューモードに移行
    {
      isRescue = true;
      sts3032.stop();
      buzzer.EnterEvacuationZone();
      sts3032.straight(50, 150);
      return;
    }
  }

  // トの字判定。前方に黒があり、外側のセンサーが反応している場合。直角をトの字と誤検知することがあるのでスピードを落としている。
  if (abs(error) > 20 && line._frontPhotoReflector > front_threshould)
  {
    error = 0;
    speed = 20;
  }
  else
  {
    speed = normalSpeed;
  }

  // PID制御
  sumError += error;
  pid = Kp * error + Ki * sumError + Kd * (error - lastError);
  lastError = error;

  turnRate = pid;
  sts3032.drive(speed, turnRate);
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
  if ((line.LastColorL == 0 || line.LastColorR == 0)) // && line._frontPhotoReflector)
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
      if (p == 1)
      {
        sts3032.straight(40, MoveToFront);
        sts3032.turn(30, -80);

        // sts3032.drive(50, -85);
        // delay(1000);
        sts3032.stop();
      }
      if (p == 2)
      {
        sts3032.straight(40, MoveToFront);
        sts3032.turn(30, 80);
        // sts3032.drive(50, 85);
        // delay(1000);
        sts3032.stop();
      }
      if (p == 3)
      {
        sts3032.turn(50, 180);
        sts3032.stop();
      }
    }
  }
}
void CheckObject()
{
  loadcell.read();
  if (loadcell.values[0] > 150 || loadcell.values[1] > 150)
  {
    sts3032.stop();
    buzzer.ObjectDetected();
    sts3032.straight(50, -40);
    sts3032.turn(50, -90);
    sts3032.straight(50, 180);
    sts3032.turn(50, 90);
    sts3032.straight(50, 300);
    sts3032.turn(50, 70);
    sts3032.straight(50, 180);
    sts3032.turn(50, -70);
    sts3032.straight(50, -50);
    // TurningObject = true;
  }
}

void TurnObject()
{
  if (tof.tof_values[1] < 120 && tof.tof_values[1] > 70)
  {
    buzzer.beep(440, 0.5);
    sts3032.drive(40, 0);
  }
  else
  {
    buzzer.beep(880, 0.5);
    sts3032.drive(40, -70);
  }
  bool blackFlag = false;
  for (int i = 0; i < 15; i++)
  {
    if (line._photoReflector[i] > threshold)
    {
      blackFlag = true;
    }
  }
  if (blackFlag)
  {
    sts3032.stop();
    buzzer.ObjectDetected();
    sts3032.turn(50, 60);
    TurningObject = false;
  }
}

void setSlopeStatus()
{
  bno.readEulerAngles();
  if (bno.pitch > 8)
  {
    SlopeStatus = 1;
  }
  else if (bno.pitch < -8)
  {
    SlopeStatus = 2;
  }
  else
  {
    SlopeStatus = 0;
  }
}