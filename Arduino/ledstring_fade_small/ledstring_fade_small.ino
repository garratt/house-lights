
#include <FastLED.h>
#define NUM_LEDS 300
#define DATA_PIN A0
CRGB leds[NUM_LEDS];
uint8_t phase[NUM_LEDS];


void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; ++i) {
    phase[i] = random(255); 
    //phase[i] = (i*47)%255; 
  }
  Serial.begin(9600);
}

#define SCALE_LEN 28
uint8_t scaled[SCALE_LEN] = {0, 0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 17, 22, 27, 32, 37, 42, 52, 62, 72, 87, 102, 117, 132, 157, 172, 187};//, 192};

#define RCHAN 0x1
#define GCHAN 0x2
#define BCHAN 0x4
#define FADEIN 0x10 // Fade out does not set this bit, or blackout
#define FOUT 0x00
#define BLACKOUT 0x20

uint8_t stages[10] = {FADEIN|RCHAN, FOUT|RCHAN, FADEIN|GCHAN, FOUT|GCHAN, FADEIN|RCHAN|GCHAN, FOUT|RCHAN|GCHAN,
                     FADEIN|RCHAN|GCHAN|BCHAN, FOUT|RCHAN|GCHAN, FOUT|BCHAN, BLACKOUT};
uint8_t stages2[10] = {FADEIN|RCHAN, FADEIN|GCHAN, FOUT|RCHAN, FOUT|GCHAN, BLACKOUT, FADEIN|GCHAN, FADEIN|RCHAN,
                     FOUT|GCHAN, FOUT|RCHAN, BLACKOUT};

void Fade(int dot, uint8_t phase, uint8_t stage) {
    if (stage & BLACKOUT) {
        leds[dot].setRGB(0, 0, 0);
        return;
    }
    if (dot >= NUM_LEDS || dot < 0) return;
    uint8_t value = scaled[(stage & FADEIN) ? phase : (SCALE_LEN-1)-phase];
    leds[dot].setRGB((stage & RCHAN) ? value : leds[dot].r,
                     (stage & GCHAN) ? value : leds[dot].g,
                     (stage & BCHAN) ? value : leds[dot].b);
}

uint8_t iteration = 0;

void display() {
  for (int i = 0; i < NUM_LEDS; ++i) {
    Fade(i, ((phase[i] + iteration)%255) % SCALE_LEN, stages2[((phase[i]+iteration)%255) / SCALE_LEN]);
  }
  FastLED.show();
  iteration++;
}

void detection_display() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].setRGB(0, 0, 0);
  }
  for (int i = 0; i < 55; ++i) {
    int rand = random(NUM_LEDS);
    leds[rand].setRGB(255, 255, 255);
  }
  FastLED.show();
}

bool has_detections=false;
void loop() {
  if (Serial.available() > 0) {
    int in  = Serial.read();
    if (in == 'D') {
      has_detections = true;
    } else {
    if (in == 'N') 
      has_detections = false;
    }
  }
  if (has_detections) {
    detection_display();
  } else {
    display();
  }

  delay(20);
}

