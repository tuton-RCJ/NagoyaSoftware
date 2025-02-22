#include <colorsensor.h>

colorsensor::colorsensor(int range, int gate, int ck, int dout)
{
    _range = range;
    _gate = gate;
    _ck = ck;
    _dout = dout;
    pinMode(_range, OUTPUT);
    pinMode(_gate, OUTPUT);
    pinMode(_ck, OUTPUT);
    pinMode(_dout, INPUT);

}

void colorsensor::start()
{
    // Gate,CK端子をLowに設定
    digitalWrite(_gate, LOW);
    digitalWrite(_ck, LOW);
    delayMicroseconds(2000); // 2000マイクロ秒待機

    // 感度設定（HIGH:高感度に設定）
    digitalWrite(_range, LOW);

    // 測光開始（光量の積算を開始）
    digitalWrite(_gate, HIGH);
}

void colorsensor::end()
{
    // 測光終了（光量の積算を終了）
    digitalWrite(_gate, LOW);
    delayMicroseconds(4); // 4マイクロ秒待機

    color[0] = shiftIn(); // 赤の処理
    color[1] = shiftIn(); // 緑の処理
    color[2] = shiftIn(); // 青の処理

    // Gate端子をHighに戻す
    digitalWrite(_gate, HIGH);
}

//12ビット分のパルス送信と読み込み処理
int colorsensor::shiftIn(){
  int result=0;//検出結果用の変数を用意（0：初期化）
  for(int i=0;i<12;i++){//12ビット分の繰り返し処理
    digitalWrite(_ck,HIGH);//1ビット分のクロックパルス出力（HIGH）
    delayMicroseconds(1);//1マイクロ秒待機
    if(digitalRead(_dout)==HIGH){//Dout端子からの出力がHighの場合
      result+=(1<<i);//12ビットのi桁目に1を代入（i桁分だけ左にシフト）
    }
    digitalWrite(_ck,LOW);//1ビット分のクロックパルス出力（LOW）
    delayMicroseconds(1);//1マイクロ秒待機
  }
  delayMicroseconds(3);//3マイクロ秒待機
  return result;//結果を出力
}

