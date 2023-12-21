
#include <FastLED.h>
#include "tree_tools.h"
#define NUM_LEDS 100
#define NUM_LEDS2 250
CRGB leds[NUM_LEDS];
CRGB leds1[NUM_LEDS];
uint8_t detections[NUM_LEDS];
#define FADE_TIME 20
uint8_t ledstate = 0;
//Fader<NUM_LEDS2> fader2(leds2);
uint8_t stages[10] = {FADEIN|RCHAN, FADEOUT|RCHAN, FADEIN|GCHAN, FADEOUT|GCHAN,
                      FADEIN|RCHAN|GCHAN, FADEOUT|RCHAN|GCHAN, FADEIN|RCHAN|GCHAN|BCHAN,
                      FADEOUT|RCHAN|GCHAN, FADEOUT|BCHAN, BLACKOUT};
void SetupStrip() {
  CRGB leds2[NUM_LEDS2];
  FastLED.addLeds<NEOPIXEL, A2>(leds2, NUM_LEDS2);
  for (int i = 0; i < NUM_LEDS2; ++i) {
      if ((i%4) & 0x2) {
        leds2[i].setRGB(240, 0, 0);
      } else {
        leds2[i].setRGB(180, 180, 180);
      }
  }
  FastLED.show();
  delay(30);
}


void setup() {
  SetupStrip();
  for (int i = 0; i < NUM_LEDS; ++i) {
    detections[i] = 0;
  }
  FastLED.addLeds<NEOPIXEL, A0>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, A4>(leds1, NUM_LEDS);

//  fader2.SetStages(stages);
  // start serial port at 9600 bps:
  Serial.begin(9600);
  while (!Serial);  // wait for serial port to connect. Needed for native USB port only
}

bool has_detections = false;

void CandyStripe(uint8_t offset) {
  for (int i = 0; i < NUM_LEDS; ++i) {
      if (((i+offset)%4) & 0x2) {
        leds[i].setRGB(0, 240, 0);
        leds1[i].setRGB(0, 240, 0);
      } else {
        leds[i].setRGB(230, 230, 230);
        leds1[i].setRGB(230, 230, 230);
      }
  }
}

#define LIT_WIDTH 25
void GreenWave(int green_pos) {
    for (int i = 0; i < NUM_LEDS; ++i) {
        leds[i].setRGB(0, 0, 0);
        leds1[i].setRGB(0, 0, 0);
    }

    for (int i = 0; i < LIT_WIDTH; ++i) {
        int li = green_pos - i;
        if (li >= 0 && li < NUM_LEDS) {
            leds[li].setRGB((LIT_WIDTH-i) * 10, 0, 0);
            leds1[li].setRGB((LIT_WIDTH-i) * 10, 0, 0);
        }
        int ri = green_pos + i;
        if (ri >= 0 && ri < NUM_LEDS) {
            leds[ri].setRGB((LIT_WIDTH-i) * 10, 0, 0);
            leds1[ri].setRGB((LIT_WIDTH-i) * 10, 0, 0);
        }
    }
}



uint8_t subcounter = 0;
int gp = 0;
void StandbyDisplay() {
    subcounter++;
    if (subcounter == 14) {
        subcounter = 0;
        gp++;
    //    fader2.Step();
        if (gp > NUM_LEDS + LIT_WIDTH) {
            gp = -LIT_WIDTH;
        }
    }
    //    GreenWave(gp);
    CandyStripe(gp%4);
}



void loop() {
  if (Serial.available() > 0) {
    int in  = Serial.read();
    if (in == 'D') {
      has_detections = true;
      digitalWrite(13, ledstate);
      ledstate = ledstate ? 0 :1;
      while (!Serial.available());
      uint8_t num_detections = Serial.read();
      for (int i = 0; i< num_detections; ++i) {
      while (!Serial.available());
        uint8_t detection = Serial.read();
        if (detection < NUM_LEDS) {
          detections[detection] = FADE_TIME;
        }
      }
    }
  }
  has_detections = false;
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i].setRGB(10*detections[i], 10*detections[i], 10*detections[i]);
    leds1[i].setRGB(10*detections[i], 10*detections[i], 10*detections[i]);
    if (detections[i]) {
      has_detections=true;
      detections[i]--;
    }
  }
  // add some red around the detections:
  if (has_detections) {
    for (int i = 0; i < NUM_LEDS; ++i) {
      if (detections[i]) {
        for (int j = 1; j < 2; j++) {
            if (i+j < NUM_LEDS && i+j > 0 && !detections[i+j]) {
              leds[i+j].setRGB(0, 100, 0);
            }
            if (i-j < NUM_LEDS && i-j > 0 && !detections[i-j]) {
              leds[i-j].setRGB(0, 100, 0);
            }
        }
      }
    }
  }
  if (!has_detections) {
      // do normal mode
    StandbyDisplay();
  }
  FastLED.show();
  delay(10);
}
