#include <FastLED.h>
#include <Creep.h>
#define NUM_LEDS 200
CRGB leds1[NUM_LEDS];
#define NUM_LEVELS 7
int prevcal1[NUM_LEVELS] = {33, 63, 98, 125, 157, 187, NUM_LEDS-1 }; //Right Bush
      
 unsigned long gdetect_time = 0;

void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds1, NUM_LEDS);
  Serial.begin(9600);

}

int gcount = 0, gphase=0, gpulse=0;
void UpdateCount() {
  gcount = (gcount +1) % 100;
  if (gcount%4 == 0) gpulse++;
  if (gcount%4 == 0) gphase++;
   if (gpulse >= PULSE_LEN) {
       gpulse = 0;
   }
}


CyPhase cy_phase(CyPhase::Location::RIGHTMOST);
#if 0
class Runner {
public;
  Runner(CRGB *leds, int *prevcal, int num_levels) leds_(leds), prevcal_(prevcal), num_levels_(num_levels) {}

  void Init(int speed, uint8_t level, uint8_t r, uint8_t g, uint8_t b) {
      level_ = level; r_ = r; g_ = g; b_ = b; speed_ = speed;
      phase = 0;
      if (level >= num_levels_) {
          speed_ = 0;
          return;
      }
      int lstart = 0, lend = prevcal_[level];
      // calculate endpoints:
      if (level > 0) {
          lstart = prevcal_[level-1];
      }
      // odd rows are reversed

  }

  void Display() {
      if (speed_ == 0) return;

  }
  CRGB *leds_;
  int *prevcal_;
  int num_levels_;
  uint8_t len_ = 10;
  int phase_ = 0;
  int speed_ = 0;  // set this negative to go the other direction
  uint8_t level_;
  uint8_t r_,g_,b_;
};
#endif

void ClearDisplay() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds1[dot].setRGB(0, 0, 0);
  }
  FastLED.show();
  gphase = -10;
}

void display(bool debugging) {
  unsigned long time_since_detect = millis() - gdetect_time;
  if (!debugging && time_since_detect < 10000) return;
  if (debugging && time_since_detect > 50000) {
     gdetect_time = millis();
     ClearDisplay();
     return;
  }
  if (time_since_detect < 29000) {
    for (int dot = 0; dot < NUM_LEDS; dot++) {
      leds1[dot].setRGB(0, 0, 0);
    }
    UpdateCount();
    CreepDisplay2(gphase, gpulse, leds1, prevcal1, NUM_LEVELS, true);
  }
  if (time_since_detect > 29000) {
    if (!cy_phase.Update()) return;
    int phase = cy_phase.Phase();
    uint8_t phase_count = cy_phase.PhaseCount();
    Cylon(phase, phase_count, leds1, prevcal1, NUM_LEVELS, false);
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
      if (!has_detections) { 
          ClearDisplay();
          gdetect_time = millis();
      }
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
