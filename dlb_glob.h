/*
  dlb_glob.h - Library for flashing Morse code.
*/
#ifndef dlb_glob_h
#define dlb_glob_h

#include "Arduino.h"

class dlb_glob
{
  public:
    dlb_glob(int pin);
    void begin();
    bool is_numer_or_char(char myChar);
  private:
    int _pin;
};

#endif