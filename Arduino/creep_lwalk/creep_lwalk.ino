#include <FastLED.h>
#include <Creep.h>
#define NUM_LEDS 100
CRGB leds1[NUM_LEDS];
#define NUM_LEVELS 1
    // just make a line up the string:
 int prevcal1[NUM_LEVELS] = { NUM_LEDS-1};
 int prevcal2[2] = {70, NUM_LEDS-1};
      
 unsigned long gdetect_time = 0;

CyPhase cy_phase(CyPhase::Location::MIDDLE);

void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds1, NUM_LEDS);
  Serial.begin(9600);
  cy_phase.SetMaxPhase(70);
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

void ClearDisplay() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds1[dot].setRGB(0, 0, 0);
  }
  FastLED.show();
  gphase = -10;
}

void display(bool debugging) {
  unsigned long time_since_detect = millis() - gdetect_time;
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
    if (!cy_phase.Update()) return;
    int phase = cy_phase.Phase();
    uint8_t phase_count = cy_phase.PhaseCount();
    Cylon(phase, phase_count, leds1, prevcal2, NUM_LEVELS, false);
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
