#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

struct Note
{
    int note;
    double duration;
};

class Buzzer
{
public:
    Buzzer(int pin);
    void beep(int note, double duration);
    void mute();
    void kouka();
    void boot();
    void EnterEvacuationZone();
    void PlayMusic(Note *notes, int length, int bpm);
    void ObjectDetected();
    void GreenMarker(int p);
    void DetectedSilverBall();  
    void DetectedBlackBall();
    void DetectedGreenCorner();
    void DetectedRedCorner();
    void NotFound();
    volatile bool isDisabled = false;

private:
    int _pin;
    int _bpm;
    void setFrequency(int freq);
};

#endif