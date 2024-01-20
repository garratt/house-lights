#include <FastLED.h>
#include <Creep.h>

#define NUM_LEDS 150
CRGB leds1[NUM_LEDS];
#define NUM_LEVELS 9
    // center bushes:
 int prevcal1[NUM_LEVELS] = {17, 31, 49, 64, 82, 97, 115, 131, NUM_LEDS-1};
      
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

CyPhase cy_phase(CyPhase::Location::LEFTMOST);
#define CYLON_WIDTH 40
#define CYLON_SCALE 13

// phase shifts a triangle function.
//        n  <-- 255 at peak, center at phase
//       / \
//      /   \
//_____/     \_____ <-- Base width W
// L   L   L   L   L <-- Levels sample function
// levels are distributed between W/2 and 100-W/2
// for 8 levels (NUM_LEVELS==9), and W=20
// they would be at 10, 20, 30, 40, 50, 60, 70, 80
// 
uint8_t GetIntensity(int phase, int curr, int num_levels) {
  int loc = CYLON_WIDTH/2 + (100-CYLON_WIDTH)/(num_levels) * curr;
 // int loc = curr*10 + 10;
  int dist = loc>=phase ? loc-phase : phase - loc;
  if (dist > CYLON_WIDTH/2) return 0;
  return max(min(180,(dist * 360) / CYLON_WIDTH),0); 
}

// go between the levels, from 0 to 100 phase

void VCylon(CRGB *leds, int *prevcal, int num_levels) {
  if (!cy_phase.Update()) return;
  int prev = 0;
  int phase = cy_phase.Phase();
  uint8_t r = cy_phase.PhaseCount() & 0x01;
  uint8_t g = (cy_phase.PhaseCount() & 0x02) >> 1;
  uint8_t b = (cy_phase.PhaseCount() & 0x04) >> 2;
  for (int j = 0; j < num_levels; ++j) {
    uint8_t intensity  = GetIntensity(phase, j, num_levels);
    if (intensity > 0) {
        for (int i = prev; i < prevcal[j]; ++i) {
            leds[i].setRGB(g*intensity, r*intensity, b*intensity);
        }
    }
    prev = prevcal[j];
  }
}


void ClearDisplay() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds1[dot].setRGB(0, 0, 0);
  }
  FastLED.show();
  gphase = -10;
}

void display(bool debugging) {
  unsigned long time_since_detect = millis() - gdetect_time;
 if (time_since_detect < 10000) return;
   // Uncomment for debugging without detections:
   if (debugging && time_since_detect > 20000) {
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
//     for (int i = 0; i < 2; ++i) {
//        leds1[random(NUM_LEDS)].setRGB(180, 180, 180);
//     }
     VCylon(leds1, prevcal1, NUM_LEVELS);
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
