#include <FastLED.h>
#include <Creep.h>
#define NUM_LEDS 300
#define NUM_LEDS2 100
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS2];
#define NUM_LEVELS 7
#define NUM_LEVELS2 6
    // Left side bushes:
 int prevcal1[NUM_LEVELS] = {53, 100, 149, 197, 242, 277, NUM_LEDS-1};
 int prevcal2[NUM_LEVELS2] = {8, 26, 49, 72, 91, NUM_LEDS2-1};
 unsigned long gdetect_time = 0;

CyPhase cy_phase(CyPhase::Location::LEFTMOST);
class SerialDetector {
  public:
    SerialDetector() {
        Serial.begin(9600);
    }
    bool CheckDetections() {
      if (Serial.available() > 0) {
        int in  = Serial.read();
        if (in == 'D') {
          if (!has_detections_) FirstDetection();
          has_detections_ = true;
        } else {
        if (in == 'N') 
          has_detections_ = false;
        }
      }
      return has_detections_;
    }

 unsigned long TimeSinceDetect() { return millis() - gdetect_time_;}
 void SetOnFirstDetection(void (*on_first_detection)()) {
     on_first_detection_ = on_first_detection;
 }
 
private:

 void FirstDetection() {
     gdetect_time_ = millis();
     if (on_first_detection_ != nullptr) {
         on_first_detection_;
     }
 }
 void (*on_first_detection_)() = nullptr;
 bool has_detections_ = false;
 unsigned long gdetect_time_ = 0;
};

void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds1, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, A2>(leds2, NUM_LEDS2);
  cy_phase.SetMaxPhase(130);
  
//  pinMode(9, OUTPUT);  // Left in
  Serial.begin(9600);
}

// big bush: reverse is true
// small bush: reverse is false

int gcount = 0, gphase=0, gpulse=0;

void ClearDisplay() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds1[dot].setRGB(0, 0, 0);
  }
  for (int dot = 0; dot < NUM_LEDS2; dot++) {
    leds2[dot].setRGB(0, 0, 0);
  }
  FastLED.show();
  gphase = -10;
}

uint8_t prev_phase_count =0;
void display(bool debugging) {
  unsigned long time_since_detect = millis() - gdetect_time;
  gcount = (gcount +1) % 100;
  if (gcount%4 == 0) gpulse++;
  if (gcount%4 == 0) gphase++;
   if (gpulse >= PULSE_LEN) {
       gpulse = 0;
   }
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
    for (int dot = 0; dot < NUM_LEDS2; dot++) {
      leds2[dot].setRGB(0, 0, 0);
    }
    CreepDisplay2(gphase, gpulse, leds1, prevcal1, NUM_LEVELS, true);
    CreepDisplay2((gphase - 100)*4, gpulse, leds2, prevcal2, NUM_LEVELS2, false);
  }

  if (time_since_detect > 29000) {
    if (!cy_phase.Update()) return;
    int phase = cy_phase.Phase();
    uint8_t phase_count = cy_phase.PhaseCount();

    // Signal Tree to change
    if (phase_count != prev_phase_count) {
        prev_phase_count = phase_count;
        // figure out which direction we're going:
        if (phase > 100) { // going left
            digitalWrite(8, 0);
        } else {
            digitalWrite(8, 1);
        }
    }

    if (phase <= 100) {
      Cylon(phase, phase_count, leds1, prevcal1, NUM_LEVELS, true);
    } else {
      Cylon((phase-100)*4, phase_count, leds2, prevcal2, NUM_LEVELS2, false);
    }
  }
  FastLED.show();
}

void detection_display() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds1[dot].setRGB(0, 0, 0);
  }
  for (int dot = 0; dot < NUM_LEDS2; dot++) {
    leds2[dot].setRGB(0, 0, 0);
  }
  for (int i = 0; i < 55; ++i) {
    int rand = random(NUM_LEDS);
    leds1[rand].setRGB(255, 255, 255);
  }
  for (int i = 0; i < 15; ++i) {
    leds2[random(NUM_LEDS2)].setRGB(255, 255, 255);
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
      if (!has_detections) { gdetect_time = millis(); }
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
