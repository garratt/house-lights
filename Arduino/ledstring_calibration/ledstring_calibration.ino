#include <FastLED.h>
#define NUM_LEDS 300
CRGB leds[NUM_LEDS];
int cursor  = 0;
uint8_t flags[NUM_LEDS];
int prevcal[] = {};
//#define NUM_LEVELS 7
 //int prevcal[NUM_LEVELS] = {33, 63, 98, 125, 157, 187, NUM_LEDS-1 };

//    int prevcal[] = {25, 48, 66, 88, 103, 118, 131, 143};
    // int prevcal[] = {15, 36, 59, 79, 97, 110, 129, 140};
    // Left side bushes: 53 100 149 197 242 277
    // Left back bushes: 8 26 49 72 91 
    // cbush 17 31 49 64 82 97 115 131 
void setup() {
   for(int dot = 0; dot < NUM_LEDS; dot++) { flags[dot] = 0; }
   for (int d : prevcal) {
    flags[d] = 1;
   }
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, A0>(leds, NUM_LEDS);
}


void SafeSetRGB(int dot, uint8_t r, uint8_t g, uint8_t b) {
  if (dot >= NUM_LEDS || dot < 0) return;
  leds[dot].setRGB(r, g, b);
}

void AddEnd(int loc) {
  if (loc > 0 && loc < NUM_LEDS) {
    flags[loc] = 1;
  }
}

void RemoveEnd(int loc) {
  if (loc > 0 && loc < NUM_LEDS) {
    flags[loc] = 0;
  }
}

void ToggleEnd(int loc) {
  if (loc >= 0 && loc < NUM_LEDS) {
    if (flags[loc] & 1) {
      flags[loc] &= 0xfe;
    } else {
      flags[loc] |= 1;
    }
  }
}

// draw scaled effect around location
void CylonLight(int loc, int len) {
  for (int i = 13; i > 0; --i) {
     SafeSetRGB(loc + (i * len) / 100, 0, 0, 20 * (13-i));
     SafeSetRGB(loc - (i * len) / 100, 0, 0, 20 * (13-i));
  }
}

void Cylon(int phase) {
  if (phase > 100) return;
  int prev = 0;
  bool reverse=false;
    for(int dot = 0; dot < NUM_LEDS; dot++) { 
      if (flags[dot] || dot== NUM_LEDS-1) {
        int len = dot-prev;
        int cloc = (phase * len) /100;
        if (reverse) {
          CylonLight(dot - cloc, len * -1);
          // leds[dot-cloc].setRGB(0,255,0);
        } else {
          CylonLight(prev + cloc, len);
          // leds[prev+cloc].setRGB(0,255,0);
        }
        prev = dot;
        reverse = !reverse;
      }
    }
}
int phase = 0;
int phase_dir = 1;
void PhaseUpdate() {
  phase += phase_dir;
  if (phase < 0) {
    phase=2;
    phase_dir = 1;
  }
  if (phase > 100) {
    phase=98;
    phase_dir = -1;
  }
}


void display() {
   for(int dot = 0; dot < NUM_LEDS; dot++) { 
        leds[dot].setRGB(0, 0, 0);
        // if (!(dot%5)) {
          // leds[dot].setRGB(0,0,30);
        // }
        if(flags[dot]) {
          leds[dot].setRGB(0,100,100);
        }
    }
  Cylon(phase);
  // PhaseUpdate();
  phase = (phase+1)%130;
  SafeSetRGB(cursor, 255, 255, 255);   
  FastLED.show();
}

void PrintCal(){
    for(int dot = 0; dot < NUM_LEDS; dot++) { 
      if (flags[dot]) {
    Serial.print(dot);
    Serial.print(" ");
      }
    }

}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    Serial.print(c);
    switch(c) {
      case 'a':
      cursor++;
      break;
      case 'd':
        cursor--;
        break;
      case 'b':
        ToggleEnd(cursor);
        PrintCal();
        break;
    };
    Serial.println(cursor);

  } else {
    delay(7);
  }
  display();

}
