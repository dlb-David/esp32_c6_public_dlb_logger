#include "dlb_clock.h"

dlb_clock::dlb_clock(uint8_t y, uint8_t m, uint8_t d, uint8_t H, uint8_t i, uint8_t s, int ms) {
  y_ = y;
  m_ = m;
  d_ = d;
  H_ = H;
  i_ = i;
  s_ = s;
  ms_ = ms;
}

void dlb_clock::set_time_from_server(String server_buff) {

  int firstClosingBracket = server_buff.indexOf(':');

  //TIME 18chars -> {"result":"24.01.13 20:43:59"}

  if ((firstClosingBracket + 19) < server_buff.length()) {
    Serial.println();
    // Serial.println(server_buff);
    H_ = 10 * (uint8_t(server_buff[firstClosingBracket + 11]) - 48) + (uint8_t(server_buff[firstClosingBracket + 12]) - 48);
    i_ = 10 * (uint8_t(server_buff[firstClosingBracket + 14]) - 48) + (uint8_t(server_buff[firstClosingBracket + 15]) - 48);
    s_ = 10 * (uint8_t(server_buff[firstClosingBracket + 17]) - 48) + (uint8_t(server_buff[firstClosingBracket + 18]) - 48);

    Serial.printf("\n TIME -> %i.%i.%i %i:%i:%i \n", y_, m_, d_, H_, i_, s_);
  }
}