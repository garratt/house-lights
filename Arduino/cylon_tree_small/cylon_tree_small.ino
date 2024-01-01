#include <FastLED.h>
#define NUM_LEDS 300
CRGB leds[NUM_LEDS];
//#define NUM_LEVELS 8
    // Left side bushes:
// int prevcal[NUM_LEVELS] = {53, 100, 149, 197, 242, 277, NUM_LEDS-1};
//int prevcal[NUM_LEVELS] = { 23, 53, 85, 123, 169, 209, 257, NUM_LEDS-1}; //tree
#define NUM_LEVELS 7
int prevcal[NUM_LEVELS] = {33, 63, 98, 125, 157, 187, NUM_LEDS-1 }; //Right Bush


void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds, NUM_LEDS);
}


void SafeSetRGB(int dot, uint8_t r, uint8_t g, uint8_t b) {
  if (dot >= NUM_LEDS || dot < 0) return;
  leds[dot].setRGB(r, g, b);
}

// draw scaled effect around location
void CylonLight(int loc, int len) {
  for (int i = 20; i > 0; --i) {
    SafeSetRGB(loc + (i * len) / 100, 0, 13 * (20 - i), 0);
    SafeSetRGB(loc - (i * len) / 100, 0, 13 * (20 - i), 0);
  }
}

void Cylon(int phase) {
  int prev = 0;
  bool reverse = false;
  for (int dot : prevcal) {
    int len = dot - prev;
    int cloc = (phase * len) / 100;
    if (reverse) {
      CylonLight(dot - cloc, len * -1);
    } else {
      CylonLight(prev + cloc, len);
    }
    prev = dot;
    reverse = !reverse;
  }
}

int phase = 0;
int phase_dir = 1;
int Phase() {
  phase += phase_dir;
  if (phase < 0 || phase > 100) {
    phase_dir *= -1;
    phase += phase_dir*3;
  }
  return phase;
}


void display() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].setRGB(20, 0, 0);
  }
  for (int i = 0; i < 2; ++i) {
    int rand = random(NUM_LEDS);
    leds[rand].setRGB(180, 180, 180);
  }
  Cylon(Phase());
  FastLED.show();
}

void loop() {
  display();
  delay(20);
}
