#include <FastLED.h>
#define NUM_LEDS 300
CRGB leds[NUM_LEDS];
#define NUM_LEVELS 8
// int prevcal[NUM_LEVELS] = { 25, 48, 66, 88, 103, 118, 131, 143, NUM_LEDS-1};
int prevcal[NUM_LEVELS] = { 23, 53, 85, 123, 169, 209, 257, NUM_LEDS-1};
#define NUM_DROPS 10
int phases[NUM_DROPS];
int levels[NUM_DROPS];
uint8_t colors[NUM_DROPS];
int color_dir = -1;

void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_DROPS; ++i) {
    phases[i] = random(100);
    levels[i] = color_dir * (random(50) + 25);
  }
  Serial.begin(9600);
}

template <int LEDS_NUM>
class Fader {

  uint8_t phases[LEDS_NUM];
};


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

void display() {
//  for (int dot = 0; dot < NUM_LEDS; dot++) {
//    leds[dot].setRGB(0, 0, 0);
//  }
  for (int i = 0; i < NUM_DROPS; ++i) {
    Drop(phases[i], levels[i], colors[i]);
    levels[i] -= color_dir;
    if ((color_dir * levels[i]) < -10) {
      levels[i]=5 + (10 * color_dir);
      phases[i] = random(100);
      colors[i] = random(2);
    }
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


bool has_detections=false;
void loop() {
  if (Serial.available() > 0) {
    int in  = Serial.read();
    if (in == 'D') {
      has_detections = true;
    } else {
    if (in == 'N') 
      has_detections = false;
    }
  }
  if (has_detections) {
    detection_display();
  } else {
    display();
  }

  delay(20);
}
