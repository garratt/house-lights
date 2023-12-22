#include <FastLED.h>
#define NUM_LEDS 300
#define NUM_LEDS2 100
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS2];
#define NUM_LEVELS 7
#define NUM_LEVELS2 6
    // Left side bushes:
 int prevcal1[NUM_LEVELS] = {53, 100, 149, 197, 242, 277, NUM_LEDS-1};
 int prevcal2[NUM_LEVELS2] = {8, 26, 49, 72, 91, NUM_LEDS2-1};

void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds1, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, A2>(leds2, NUM_LEDS2);
}

#define CYLON_WIDTH 20
#define CYLON_SCALE 13

void Cylon(int phase, CRGB *leds, int *prevcal, int num_levels) {
  int prev = 0;
  bool reverse = false;
  for (int j = 0; j < num_levels; ++j) {
    int dot = prevcal[j];
    int len = dot - prev;
    // this is normally between 0 and len, but goes off the edge by a bit
    int cloc = (phase * len) / 100; 
    // Location of center of dot:
    int loc  = reverse ? dot - cloc : prev + cloc;
    // fade light out over set distance:
    for (int i = 0; i < CYLON_WIDTH; ++i) {
        int offset = (i*len) / 100;
        if (loc+offset >= prev && loc+offset <= dot) {
            leds[loc+offset].setRGB(0, CYLON_SCALE * (CYLON_WIDTH - i), 0);
        }
        if (loc-offset >= prev && loc-offset <= dot) {
            leds[loc-offset].setRGB(0, CYLON_SCALE * (CYLON_WIDTH - i), 0);
        }
    }
    prev = dot;
    reverse = !reverse;
  }
}


// phase1 is 100, phase 2 is 30, scaled up
class DoublePhase {
public:
    void Update() {
      int scaler = phase_ < 100 ? 3 : 1;
      phase_ += phase_dir_ * scaler;
      if (phase_ < 0 || phase_ > 200) {
        phase_dir_ *= -1;
        phase_ += phase_dir_*3 * scaler;
      }
    }
    int Phase1() { return phase_ > 80 ? phase_ - 100 : -20; }
    int Phase2() { return 100 - phase_; }

private:
    int phase_ = 0;
    int phase_dir_ = 1;
};

DoublePhase dphase;


void display() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds1[dot].setRGB(20, 0, 0);
  }
  for (int dot = 0; dot < NUM_LEDS2; dot++) {
    leds2[dot].setRGB(20, 0, 0);
  }
  leds1[random(NUM_LEDS)].setRGB(180, 180, 180);
  leds1[random(NUM_LEDS)].setRGB(180, 180, 180);
  leds2[random(NUM_LEDS2)].setRGB(180, 180, 180);
  dphase.Update();
  Cylon(dphase.Phase1(), leds1, prevcal1, NUM_LEVELS);
  Cylon(dphase.Phase2(), leds2, prevcal2, NUM_LEVELS2);
  FastLED.show();
}

void loop() {
  display();
  delay(20);
}
