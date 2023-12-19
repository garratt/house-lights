
#include <FastLED.h>
#define NUM_LEDS 100
CRGB leds[NUM_LEDS];
uint8_t detections[NUM_LEDS];
#define FADE_TIME 20
uint8_t ledstate = 0;
void setup() {
  for (int i = 0; i < NUM_LEDS; ++i) {
    detections[i] = 0;
  }
  FastLED.addLeds<NEOPIXEL, A0>(leds, NUM_LEDS);
  // start serial port at 9600 bps:
  Serial.begin(9600);
  while (!Serial);  // wait for serial port to connect. Needed for native USB port only
}

void loop() {
  if (Serial.available() > 0) {
    char in  = Serial.read();
    if (in == 'D') {
      digitalWrite(13, ledstate);
      ledstate = ledstate ? 0 :1;
      uint8_t num_detections = Serial.read();
      for (int i = 0; i< num_detections; ++i) {
        uint8_t detection = Serial.read();
        if (detection < NUM_LEDS) {
          detections[detection] = FADE_TIME;
        }
      }
    }
  }
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i].setRGB(10*detections[i], 10*detections[i], 10*detections[i]);
    if (detections[i]) {
      detections[i]--;
    }
  }
  FastLED.show();
  delay(10);
}
