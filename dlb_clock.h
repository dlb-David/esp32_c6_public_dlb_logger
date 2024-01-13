#ifndef dlb_clock_h
#define dlb_clock_h

#include "Arduino.h"

class dlb_clock {
public:
  dlb_clock(uint8_t y, uint8_t m, uint8_t d, uint8_t H, uint8_t i, uint8_t s, int ms);
  void set_time_from_server(String server_buff);
private:
  uint8_t y_;
  uint8_t m_;
  uint8_t d_;
  uint8_t H_;
  uint8_t i_;
  uint8_t s_;
  int ms_;
};

#endif