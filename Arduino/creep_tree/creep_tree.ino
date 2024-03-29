#include <FastLED.h>
#include <Creep.h>
#define NUM_LEDS 300
CRGB leds[NUM_LEDS];
#define NUM_LEVELS 8
// int prevcal[NUM_LEVELS] = { 25, 48, 66, 88, 103, 118, 131, 143, NUM_LEDS-1};
int prevcal[NUM_LEVELS] = { 23, 53, 85, 123, 169, 209, 257, NUM_LEDS-1};
#define NUM_DROPS 1
int phases[NUM_DROPS];
int levels[NUM_DROPS];
uint8_t colors[NUM_DROPS];
int color_dir = -1;
unsigned long gdetect_time = 0;

CyPhase cy_phase(CyPhase::Location::LEFTMOST);

void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_DROPS; ++i) {
    phases[i] = random(100);
    levels[i] = color_dir * (random(50) + 25);
  }
  cy_phase.SetMaxPhase(150);
  Serial.begin(9600);
}

template <int LEDS_NUM>
class Fader {

  uint8_t phases[LEDS_NUM];
};

void ClearDisplay() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].setRGB(0, 0, 0);
  }
  FastLED.show();
}

void SafeSetRGB(int dot, uint8_t r, uint8_t g, uint8_t b) {
  if (dot >= NUM_LEDS || dot < 0) return;
  leds[dot].setRGB(r, g, b);
}

void SafeSetColor(int dot, uint8_t color, uint8_t blevel, uint8_t clevel) {
  if (dot >= (NUM_LEDS) || dot < 0) return;
  uint8_t r = blevel, g = blevel, b = blevel;
  if (color == 0) r = clevel;
  if (color == 1) g = clevel;
  if (color == 2) b = clevel;
  leds[dot].setRGB(r, g, b);
//  leds[dot + 1].setRGB(r/4, g/4, b/4);
//  leds[dot - 1].setRGB(r/4, g/4, b/4);
}

int GetPixel(int phase, int level) {
  if (level < 1 || level >= NUM_LEVELS) return -1;
  int prev = prevcal[level-1];
  int dot = prevcal[level];
  int len = dot - prev;
  int cloc = (phase * len) / 100;
  if (level % 2) {
    return prev + cloc;
  }
  return dot - cloc;
}

void Drop(int phase, int level, uint8_t color) {
  SafeSetColor(GetPixel(phase, level), color, 0, 200);
  SafeSetColor(GetPixel(phase, level+(color_dir * 1)), color, 0, 200);
  SafeSetColor(GetPixel(phase, level+(color_dir * 2)), color, 0, 200);
  SafeSetColor(GetPixel(phase, level+(color_dir * 3)), color, 0, 100);
  SafeSetColor(GetPixel(phase, level+(color_dir * 4)), color, 0, 100);
  SafeSetColor(GetPixel(phase, level+(color_dir * 5)), color, 0, 100);
  SafeSetColor(GetPixel(phase, level+(color_dir * 6)), color, 0, 50);
  SafeSetColor(GetPixel(phase, level+(color_dir * 7)), color, 0, 20);
  SafeSetColor(GetPixel(phase, level+(color_dir * 8)), color, 0, 10);

}

// void Drop(int phase, int level, uint8_t color) {
//   SafeSetColor(GetPixel(phase, level), color, 200, 200);
//   SafeSetColor(GetPixel(phase, level+(color_dir * 1)), color, 100, 200);
//   SafeSetColor(GetPixel(phase, level+(color_dir * 2)), color, 100, 200);
//   SafeSetColor(GetPixel(phase, level+(color_dir * 3)), color, 50, 200);
//   SafeSetColor(GetPixel(phase, level+(color_dir * 4)), color, 50, 200);
//   SafeSetColor(GetPixel(phase, level+(color_dir * 5)), color, 50, 200);
//   SafeSetColor(GetPixel(phase, level+(color_dir * 6)), color, 20, 200);
//   SafeSetColor(GetPixel(phase, level+(color_dir * 7)), color, 10, 200);
//   SafeSetColor(GetPixel(phase, level+(color_dir * 8)), color, 0, 200);
// 
// }


int gcount=0;

void FirstDetection() {
  gdetect_time = millis();
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].setRGB(0, 0, 0);
  }
  FastLED.show();
}


void display(bool debugging) {
  unsigned long time_since_detect = millis() - gdetect_time;
   if (debugging && time_since_detect > 20000) {
       gdetect_time = millis();
     ClearDisplay();
     return;
   }
   if (time_since_detect < 15000) return;

  if (time_since_detect < 29000) {
    for (int i = 0; i < NUM_DROPS; ++i) {
      Drop(phases[i], levels[i], colors[i]);
      levels[i] -= color_dir;
      if ((color_dir * levels[i]) < -10) {
        levels[i]=5 + (10 * color_dir);
        phases[i] = random(100);
        colors[i] = 1;
      }
    }
  }
  if (time_since_detect > 29000) {
    if (!cy_phase.Update()) return;
    int phase = cy_phase.Phase();
    uint8_t phase_count = cy_phase.PhaseCount();
    if (phase >50)
      Cylon((phase-50), phase_count, leds, prevcal, NUM_LEVELS, true);
  }

  FastLED.show();
}

void detection_display() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].setRGB(0, 0, 0);
  }
  for (int i = 0; i < 55; ++i) {
    int rand = random(NUM_LEDS);
    leds[rand].setRGB(180, 180, 180);
  }
  FastLED.show();
}


bool debugging = false;
bool has_detections=false;
void loop() {
  delay(20);
  if (debugging) {
     display(true);
     return;
  }
  if (Serial.available() > 0) {
    int in  = Serial.read();
    if (in == 'D') {
      if (!has_detections) FirstDetection();
      has_detections = true;
    } else {
    if (in == 'N') 
      has_detections = false;
    }
  }
  if (has_detections) {
    display(false);
  } else {
    ClearDisplay();
    cy_phase.Reset();
  }
}
