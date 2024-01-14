#ifndef dlb_LCD_h
#define dlb_LCD_h

#include "Arduino.h"

class dlb_LCD {
public:
  dlb_LCD();
private:
  uint8_t STATUS = {0};
  String worker;
  String job;
  String variant;
};

#endif