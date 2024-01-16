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
 unsigned long gdetect_time = 0;

void setup() {
  FastLED.addLeds<NEOPIXEL, A0>(leds1, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, A2>(leds2, NUM_LEDS2);
  Serial.begin(9600);
}
#define DEBUG_LOOP 0
#define PULSE_LEN 26
uint8_t pulse_val[PULSE_LEN] = {27, 32, 42, 52, 62, 72, 87, 102, 117, 132, 157, 172, 187,
                             187, 172, 157, 132, 117, 102, 87, 72, 62, 52, 42, 32,27};

// big bush: reverse is true
// small bush: reverse is false
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

  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds1[dot].setRGB(0, 0, 0);
  }
  for (int dot = 0; dot < NUM_LEDS2; dot++) {
    leds2[dot].setRGB(0, 0, 0);
  }
  CreepDisplay2(gphase, gpulse, leds1, prevcal1, NUM_LEVELS, true);
  CreepDisplay2((gphase - 100)*4, gpulse, leds2, prevcal2, NUM_LEVELS2, false);

  if (time_since_detect > 29000) {
     for (int i = 0; i < 2; ++i) {
        leds1[random(NUM_LEDS)].setRGB(180, 180, 180);
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
  }

}
