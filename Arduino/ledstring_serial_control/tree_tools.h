#include <FastLED.h>



// This is a sequence of values that will result in the leds seeming to get
// brighter with constant steps:
#define SCALE_LEN 28
uint8_t scaled[SCALE_LEN] = {0, 0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 17, 22, 27, 32, 37, 42, 52, 62, 72, 87, 102, 117, 132, 157, 172, 187};//, 192};

#define RCHAN 0x1
#define GCHAN 0x2
#define BCHAN 0x4
#define FADEIN 0x10 // Fade out does not set this bit, or blackout
#define FADEOUT 0x00
#define BLACKOUT 0x20
#define PHASE_SIZE 30

// Example fading stages:
// uint8_t stages[10] = {FADEIN|RCHAN, FADEOUT|RCHAN, FADEIN|GCHAN, FADEOUT|GCHAN,
                      // FADEIN|RCHAN|GCHAN, FADEOUT|RCHAN|GCHAN, FADEIN|RCHAN|GCHAN|BCHAN,
                      // FADEOUT|RCHAN|GCHAN, FADEOUT|BCHAN, BLACKOUT};
// uint8_t stages2[10] = {FADEIN|RCHAN, FADEIN|GCHAN, FADEOUT|RCHAN, FADEOUT|GCHAN,
                       // BLACKOUT, FADEIN|GCHAN, FADEIN|RCHAN,
                       // FADEOUT|GCHAN, FADEOUT|RCHAN, BLACKOUT};
uint8_t blackout_stages[10] = {BLACKOUT, BLACKOUT, BLACKOUT, BLACKOUT, BLACKOUT,
               BLACKOUT, BLACKOUT, BLACKOUT, BLACKOUT, BLACKOUT};
void Fade(CRGB &dot, uint8_t phase, uint8_t stage) {
    if (stage & BLACKOUT) {
        dot.setRGB(0, 0, 0);
        return;
    }
    uint8_t value = scaled[(stage & FADEIN) ? phase : (SCALE_LEN-1)-phase];
    dot.setRGB((stage & RCHAN) ? value : dot.r,
               (stage & GCHAN) ? value : dot.g,
               (stage & BCHAN) ? value : dot.b);
}

template <uint32_t LED_NUM>
class Fader {
 public:
  Fader(CRGB *leds) :  leds_(leds) {
    for (int i = 0; i < PHASE_SIZE; ++i) {
      phases_[i] = random(255); 
    }
    SetStages(blackout_stages);
  }

  void SetStages(uint8_t stages[10]) {
    for (int i = 0; i < LED_NUM; ++i) {
      stages_[i] = stages[i];
    }
  }

  void Step() {
    for (int i = 0; i < LED_NUM; ++i) {
      Fade(leds_[i], ((phases_[i % PHASE_SIZE] + iteration)%255) % SCALE_LEN,
              stages_[((phases_[i % PHASE_SIZE]+iteration)%255) / SCALE_LEN]);
    }
    iteration++;
  }

 private:
  uint8_t iteration = 0;
  uint8_t phases_[LED_NUM % PHASE_SIZE];
  CRGB *leds_;
  uint8_t stages_[10];
};



