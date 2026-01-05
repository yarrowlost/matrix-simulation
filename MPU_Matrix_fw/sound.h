#pragma once
#include <AceRoutine.h>
#include <Arduino.h>

using namespace ace_routine;

// 🎵 Nota frekansları (Orta oktav ve çevresi)
#define NOTE_REST 0
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

// 🎼 Nota süreleri (ölçü cinsinden - 1: tam, 2: yarım, 4: çeyrek, vb.)
enum NoteDuration {
  N1 = 1,    // Whole note
  N2 = 2,    // Half note
  N4 = 4,    // Quarter note
  N8 = 8,    // Eighth note
  N16 = 16,  // Sixteenth note
  N32 = 32,
  N64 = 64,
  N8P16 = 6,
};

// 🎶 Nota veri yapısı
struct Note {
  int pitch;              // Frekans (Hz) veya NOTE_REST
  NoteDuration duration;  // N1, N2, N4, N8, N16
};

Note fireSFX[] = {
  { NOTE_C6, N64 }, { NOTE_A5, N64 }//, { NOTE_F5, N64 }, { NOTE_C5, N32 }
};

Note hitEnemySFX[] = {
  { NOTE_GS5, N64 }, { NOTE_C6, N32 }
};

Note takeDamageSFX[] = {
  { NOTE_G3, N32 }, { NOTE_DS3, N32 }, { NOTE_G3, N32 }
};


// 🔊 ESP Music sınıfı
class MusicPlayer : public Coroutine {

public:
  MusicPlayer(int outputPin, int tempoBPM = 120) {
    pin = outputPin;
    bpm = tempoBPM;
    beatMs = 60000 / bpm;  // 1 çeyrek nota kaç ms sürer
    pinMode(pin, OUTPUT);
  }

  int runCoroutine() override {

    COROUTINE_LOOP() {

      while (noteIndex < length) {

        Serial.print("play note");
      
        Serial.println(noteIndex);
        auto note = melody[noteIndex ++];

        if (note.pitch == NOTE_REST) {
          noTone(pin);
        } else {
          static int durMs = beatMs * N4 / note.duration;
          tone(pin, note.pitch, durMs);
          COROUTINE_DELAY(durMs * 1.3);  // %30 boşluk bırak
          noTone(pin);
        }
      }

      COROUTINE_DELAY(100);
    }
  }

  void play(int mel) {
    switch (mel) {
      case 0:
        melody = fireSFX;
        length = sizeof(fireSFX) / sizeof(Note);
        break;
      case 1:
        melody = hitEnemySFX;
        length = sizeof(hitEnemySFX) / sizeof(Note);
        break;
      case 2:
        melody = takeDamageSFX;
        length = sizeof(takeDamageSFX) / sizeof(Note);
        break;
      default: break;
    }
    noteIndex = 0;
  }

  void setBPM(int newBPM) {
    bpm = newBPM;
    beatMs = 60000 / bpm;
  }

private:

  // void playNote(Note note) {
  //   // if (note.pitch == NOTE_REST) {
  //   //   noTone(pin);
  //   // } else {
  //   //   int durMs = beatMs * N4 / note.duration;
  //   //   tone(pin, note.pitch, durMs);
  //   //   COROUTINE_DELAY(durMs * 1.3);  // %30 boşluk bırak
  //   //   noTone(pin);
  //   // }
  // }

  int pin;
  int bpm;
  int beatMs;  // N4 (quarter note) süresi milisaniye cinsinden

  Note* melody = nullptr;
  int length = 0;
  int noteIndex = 0;
};


//p.play(fireSFX, sizeof(fireSFX) / sizeof(Note));
