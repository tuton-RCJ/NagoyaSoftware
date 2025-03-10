#include "buzzer.h"

#define C3 131
#define D3 147
#define E3 165
#define F3 175
#define G3 196
#define A3 220
#define B3 247
#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define B4 494
#define C5 523
#define D5 587
#define E5 659
#define F5 698
#define G5 784
#define A5 880
#define B5 988
#define C6 1047

Buzzer::Buzzer(int pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
}

void Buzzer::setFrequency(int freq)
{
    if (isDisabled)
    {
        return;
    }

    analogWriteFrequency(freq);
    analogWrite(_pin, 64);
}

void Buzzer::mute()
{
    analogWrite(_pin, 0);
}

void Buzzer::beep(int note, double duration)
{
    int interbal = 10;
    long beepDuration = 60000.0 / _bpm * duration - interbal;
    setFrequency(note);
    delay(beepDuration);
    mute();
    delay(interbal);
}

//----------------------------------------------------------

void Buzzer::boot()
{
    Note notes[] = {
        {C5, 0.5},
        {E5, 0.5},
        {G5, 0.5},
    };

    PlayMusic(notes, 3, 400);
}

void Buzzer::ObjectDetected()
{
    Note notes[] = {
        {E4, 0.5},
        {A4, 0.5},
        {G4, 0.5},
        {E4, 0.5},
        {A4, 0.5},
        {G4, 0.5},
    };

    PlayMusic(notes, 6, 200);
}

void Buzzer::GreenMarker(int p)
{
    if (p == 1)
    {
        Note notes[] = {
            {C5, 0.5},
            {G5, 0.5},
        };
        PlayMusic(notes, 2, 100);
        return;
    }
    if (p == 2)
    {
        Note notes[] = {
            {C5, 0.5},
            {G5, 0.5},
            {E5, 0.5},
        };
        PlayMusic(notes, 3, 100);
        return;
    }
    if (p == 3)
    {
        Note notes[] = {
            {C5, 0.5},
            {G5, 0.5},
            {E5, 0.5},
            {C5, 0.5},
        };
        PlayMusic(notes, 4, 100);
        return;
    }
}

void Buzzer::EnterEvacuationZone()
{
    Note notes[] = {
        {C5, 0.5},
        {E5, 0.5},
        {G5, 0.5},
        {F5, 0.5},
        {E5, 0.5},
        {G5, 0.5},
        {C6, 0.5},
    };
    PlayMusic(notes, 7, 200);
}

void Buzzer::kouka()
{
    Note notes[] = {
        {C4, 1},
        {E4, 1},
        {G4, 1.5},
        {F4, 0.5},
        {E4, 1},
        {C4, 1},
        {D4, 2},
        {E4, 1},
        {E4, 1},
        {D4, 1.5},
        {C4, 0.5},
        {G4, 4},
        {E4, 1},
        {E4, 1},
        {D4, 1.5},
        {C4, 0.5},
        {A3, 1},
        {C4, 1},
        {G3, 2},
        {C4, 1},
        {E4, 1},
        {D4, 1.5},
        {G3, 0.5},
        {C4, 4},
    };

    PlayMusic(notes, 24, 100);
}

// Noteの配列を受け取り、1つずつbeep()で音を鳴らす、Disabledなら即座にreturn
void Buzzer::PlayMusic(Note *notes, int length, int bpm)
{
    _bpm = bpm;
    for (int i = 0; i < length; i++)
    {
        if (isDisabled)
        {
            return;
        }
        beep(notes[i].note, notes[i].duration);
    }
}

void Buzzer::DetectedBlackBall()
{
    Note notes[] = {
        {C5, 0.5},
        {E5, 0.5},
        {C5, 0.5},
        {E5, 0.5},
    };
    PlayMusic(notes, 4, 200);
}

void Buzzer::DetectedSilverBall()
{
    Note notes[] = {
        {C5, 0.5},
        {E5, 0.5},
        {C5, 0.5},
        {E5, 0.5},
        {C5, 0.5},
        {E5, 0.5},
    };
    PlayMusic(notes, 6, 200);
}

void Buzzer::DetectedGreenCorner()
{
    Note notes[] = {
        {G5, 0.5},
        {E5, 0.5},
        {G5, 0.5},
        {E5, 0.5},
        {G5, 0.5},
        {E5, 0.5},

    };
    PlayMusic(notes, 6, 200);
}

void Buzzer::DetectedRedCorner()
{
    Note notes[] = {
        {G5, 0.5},
        {E5, 0.5},
        {G5, 0.5},
        {E5, 0.5}};
    PlayMusic(notes, 4, 200);
}

void Buzzer::NotFound()
{
    Note notes[] = {
        {C5, 0.5},
        {B4, 0.5},
        {C5, 0.5},
        {B4, 0.5},
        {C5, 0.5}};
    PlayMusic(notes, 5, 200);
}