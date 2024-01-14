#ifndef dlb_eeprom_h
#define dlb_eeprom_h

#include "Arduino.h"
#include "EEPROM.h"

class dlb_eeprom {
public:
  dlb_eeprom();
  void write();
  void read();
  bool znak(char myChar);
private:
  uint8_t SSID_[50] = { 0 };
  uint8_t PASSWORD_[50] = { 0 };
  uint8_t KEY_[50] = { 0 }; //SHA1
  // Instantiate eeprom objects with parameter/argument names and sizes
};

#endif