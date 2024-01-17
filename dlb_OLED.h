#ifndef dlb_OLED_h
#define dlb_OLED_h

#include "Arduino.h"

class dlb_OLED {
public:
  dlb_OLED();
 //Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
private:
  char NUMFLAKES = 10;      // Number of snowflakes in the animation example
  char SCREEN_WIDTH = 128;  // OLED display width, in pixels
  char SCREEN_HEIGHT = 32;  // OLED display height, in pixels
  char LOGO_HEIGHT = 16;
  char LOGO_WIDTH = 16;
  char OLED_RESET = -1; // Reset pin # (or -1 if sharing Arduino reset pin)
};
#endif