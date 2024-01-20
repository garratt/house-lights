#include "Arduino.h"
#ifndef CREEP_H
#define CREEP_H

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



// bounce between 0 and 100
class CyPhase {
public:
    enum : byte {ACTIVE, WAITLEFT, WAITRIGHT} state_ = ACTIVE;
    enum  Location: byte {LEFTMOST, MIDDLE, RIGHTMOST};
    CyPhase(Location location) : location_(location) {
        pinMode(6, INPUT_PULLUP);  // Right in
        pinMode(7, OUTPUT);  // Right out
        pinMode(8, OUTPUT);  // Left out
        pinMode(9, INPUT_PULLUP);  // Left in
        Reset();
    }

    void Reset() {
      state_ = (location_ == LEFTMOST) ? ACTIVE : WAITLEFT;
      digitalWrite(7, 1);
      digitalWrite(8,1);
      waitcount_ = 0;
      phase_ = 0;
      phase_dir_ = 1;
      phase_count_ = 1;
    }

    bool CheckLeft() { return digitalRead(9) == 0; }
    bool CheckRight() { return digitalRead(6) == 0; }
    void SignalLeft() {
        state_ = WAITLEFT;
        waitcount_ = 10;
        digitalWrite(8, 0);
        }
    void SignalRight() {
        state_ = WAITRIGHT;
        waitcount_ = 10;
        digitalWrite(7, 0);
        }
    void ResetLeft() { digitalWrite(8, 1); }
    void ResetRight() { digitalWrite(7, 1); }

    bool CheckState() {
      if (state_ == WAITRIGHT) {
          if (CheckRight()) {
              ResetRight();
              state_ = ACTIVE;
          } else {
              if(waitcount_ > 0) {
                  waitcount_--;
              } else {
                  ResetRight();
              }
          }
      }
      if (state_ == WAITLEFT) {
          if (CheckLeft()) {
              ResetLeft();
              state_ = ACTIVE;
          } else {
              if(waitcount_ > 0) {
                  waitcount_--;
              } else {
                  ResetLeft();
              }
          }
      }
      return IsActive();
    }


    bool Update() {
      if (!CheckState()) return false;
      phase_ += phase_dir_;
      if (phase_ < 0 || phase_ > 100) {
        phase_dir_ *= -1;
        phase_ += phase_dir_*3;
        phase_count_++;
        if (phase_count_ % 8 == 7) phase_count_+=2;
        if (location_ != LEFTMOST && phase_ < 10) {
            SignalLeft();
        }
        if (location_ != RIGHTMOST && phase_ > 90) {
            SignalRight();
        }
      }
      return IsActive();
    }
    bool IsActive() { return state_ == ACTIVE; }
    int Phase() { return phase_; }
    uint8_t PhaseCount() { return phase_count_; }

private:
    Location location_;
    int waitcount_ = 0;
    int phase_ = 0;
    int phase_dir_ = 1;
    uint8_t phase_count_ = 1;

};




#endif
