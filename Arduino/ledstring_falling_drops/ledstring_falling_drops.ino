#include <FastLED.h>
#define NUM_LEDS 300
CRGB leds[NUM_LEDS];
#define NUM_LEVELS 8
// int prevcal[NUM_LEVELS] = { 25, 48, 66, 88, 103, 118, 131, 143, NUM_LEDS-1};
int prevcal[NUM_LEVELS] = { 23, 53, 85, 123, 169, 209, 257, NUM_LEDS-1};
#define NUM_DROPS 17
int phases[NUM_DROPS];
int levels[NUM_DROPS];
uint8_t colors[NUM_DROPS];

void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_DROPS; ++i) {
    phases[i] = random(100);
    levels[i] = random(50) + 25;
  }
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
  if (dot >= NUM_LEDS || dot < 0) return;
  if (color == 0) leds[dot].setRGB(clevel, blevel, blevel);
  if (color == 1) leds[dot].setRGB(blevel, clevel, blevel);
  if (color == 2) leds[dot].setRGB(blevel, blevel, clevel);
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
  SafeSetColor(GetPixel(phase, level), color, 200, 200);
  SafeSetColor(GetPixel(phase, level+1), color, 100, 200);
  SafeSetColor(GetPixel(phase, level+2), color, 50, 200);
  SafeSetColor(GetPixel(phase, level+3), color, 25, 200);
  SafeSetColor(GetPixel(phase, level+4), color, 10, 200);
  SafeSetColor(GetPixel(phase, level+5), color, 5, 200);
  SafeSetColor(GetPixel(phase, level+6), color, 2, 200);

}

void display() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].setRGB(0, 0, 0);
  }
  for (int i = 0; i < NUM_DROPS; ++i) {
    Drop(phases[i], levels[i]--, colors[i]);
    if (levels[i] < -10) {
      levels[i]=15;
      phases[i] = random(100);
      colors[i] = random(3);
    }
  }

  FastLED.show();
}

void loop() {
  display();
  delay(100);
}
