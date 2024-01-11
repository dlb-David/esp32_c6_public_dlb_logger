#ifndef dlb_logger_h
#define dlb_logger_h

#include "Arduino.h"

class dlb_logger {
public:
  dlb_logger();
  void P_up(int P);
  void P_down(int P);
private:
  int P1=0;
  int P2=0;
  int P3=0;
};

#endif