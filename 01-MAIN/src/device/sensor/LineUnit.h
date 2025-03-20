#ifndef line_H
#define line_H

#include <Arduino.h>

class LineUnit
{
public:
    LineUnit(HardwareSerial *serial);
    void init();
    bool read();
    void Flush();
    void print(HardwareSerial *serial);

    int photoReflector[15]; //0=白、1=黒、2=銀
    int frontPhotoReflector; //0=白、1=黒、2=銀

    int colorL[3];
    int colorR[3]; 
    int LastColorL;
    int LastColorR;

    /// @brief Time of L color detection 0,1,2,3=Black,White,Green,Red
    unsigned long colorLTime[4];
    /// @brief Time of R color detection 0,1,2,3=Black,White,Green,Red
    unsigned long colorRTime[4];


    void setBrightness(int brightness);

private:
    HardwareSerial *_serial;
    void checkColor(int *color, unsigned long *colorTime, int *LastColor);
    
    int _photoReflector[15];
    int _frontPhotoReflector;

    const int threshold = 200; // フォトリフレクタの黒と白の閾値
    const int front_threshould = 225; // 前方フォトの閾値（1つ離れているため閾値が他と異なる）
    const int silver_threshould = 25; // 銀の閾値
};

#endif
