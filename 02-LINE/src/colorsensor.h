#ifndef colosensor_h
#define colosensor_h

#include <Arduino.h>

class colorsensor
{
public:
    colorsensor(int range, int gate, int ck, int dout);
    // RGB値を保存する配列
    int color[3];
    // 測光開始
    void start();
    // 測光終了
    void end();
private:
    int _range;
    int _gate;
    int _ck;
    int _dout;
    int shiftIn();
};

#endif