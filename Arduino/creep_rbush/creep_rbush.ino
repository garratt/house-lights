#include <FastLED.h>
#define NUM_LEDS 200
CRGB leds1[NUM_LEDS];
#define NUM_LEVELS 7
int prevcal1[NUM_LEVELS] = {33, 63, 98, 125, 157, 187, NUM_LEDS-1 }; //Right Bush
      
 unsigned long gdetect_time = 0;

void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds1, NUM_LEDS);
  Serial.begin(9600);
}
#define PULSE_LEN 26
uint8_t pulse_val[PULSE_LEN] = {27, 32, 42, 52, 62, 72, 87, 102, 117, 132, 157, 172, 187,
                             187, 172, 157, 132, 117, 102, 87, 72, 62, 52, 42, 32,27};

// center bush: reverse is true
void CreepDisplay2(int phase, int pulse, CRGB *leds, int *prevcal, int num_levels, bool reverse) {
  if (phase < 0) return;
  if (phase > 100 ) {
    for (int dot = 0; dot < prevcal[num_levels-1]; dot++) {
      leds[dot].setRGB(0,  pulse_val[pulse], 0);
    }
    return;
  }
      
  int prev = 0;
  for (int j = 0; j < num_levels; ++j) {
    int dot = prevcal[j];
    int len = dot - prev;
    // this is normally between 0 and len, but goes off the edge by a bit
    int clen = (phase * len) / 100; 
    // Location of center of dot:
    int loc  = reverse ? dot - clen : prev + clen;
    int start = reverse ? dot : prev;
    int dir = reverse ? -1 : 1;
    for (int i = 0; i < clen; ++i) {
      float intensity = (clen - i) > 10? 1.0 : (clen-i) / 10.0;
      if (start + dir*i >= prev && start+dir*i < dot)
        leds[start + dir*i].setRGB(0, pulse_val[pulse] * intensity, 0);
    }
    prev = dot;
    reverse = !reverse;
  }
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

// bounce between 0 and 100
class CyPhase {
public:
    void Update() {
      phase_ += phase_dir_;
      if (phase_ < 0 || phase_ > 100) {
        phase_dir_ *= -1;
        phase_ += phase_dir_*3;
        phase_count_++;
        if (phase_count_ % 8 == 0) phase_count_++;
      }
    }
    int Phase() { return phase_; }
    uint8_t PhaseCount() { return phase_count_; }

private:
    int phase_ = 0;
    int phase_dir_ = 1;
    uint8_t phase_count_ = 1;
};

CyPhase cy_phase;
#define CYLON_WIDTH 20
#define CYLON_SCALE 13

void Cylon(CRGB *leds, int *prevcal, int num_levels) {
  int prev = 0;
  cy_phase.Update();
  int phase = cy_phase.Phase();
  uint8_t r = cy_phase.PhaseCount() & 0x01;
  uint8_t g = (cy_phase.PhaseCount() & 0x02) >> 1;
  uint8_t b = (cy_phase.PhaseCount() & 0x04) >> 2;
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
        int intensity = CYLON_SCALE * (CYLON_WIDTH - i);

        if (loc+offset >= prev && loc+offset <= dot) {
            leds[loc+offset].setRGB(g*intensity, r*intensity, b*intensity);
        }
        if (loc-offset >= prev && loc-offset <= dot) {
            leds[loc-offset].setRGB(g*intensity, r*intensity, b*intensity);
        }
    }
    prev = dot;
    reverse = !reverse;
  }
}
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
     for (int i = 0; i < 2; ++i) {
        leds1[random(NUM_LEDS)].setRGB(180, 180, 180);
     }
     Cylon(leds1, prevcal1, NUM_LEVELS);
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
  }

}
