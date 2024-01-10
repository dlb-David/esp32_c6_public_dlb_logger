/*
  dlb_glob.cpp - Library for flashing Morse code.
*/

#include "Arduino.h"
#include "dlb_glob.h"

dlb_glob::dlb_glob(int pin) {
  _pin = pin;
}

void dlb_glob::begin() {
}

bool dlb_glob::is_numer_or_char(char myChar) {
  bool out_=false;

    if(myChar=='A') out_=true;
    if(myChar=='B') out_=true;
    if(myChar=='C') out_=true;
    if(myChar=='D') out_=true;
    if(myChar=='E') out_=true;
    if(myChar=='F') out_=true;
    if(myChar=='G') out_=true;
    if(myChar=='H') out_=true;
    if(myChar=='I') out_=true;
    if(myChar=='J') out_=true;
    if(myChar=='K') out_=true;
    if(myChar=='L') out_=true;
    if(myChar=='M') out_=true;
    if(myChar=='N') out_=true;
    if(myChar=='O') out_=true;
    if(myChar=='P') out_=true;
    if(myChar=='Q') out_=true;
    if(myChar=='R') out_=true;
    if(myChar=='S') out_=true;
    if(myChar=='T') out_=true;
    if(myChar=='U') out_=true;
    if(myChar=='V') out_=true;
    if(myChar=='W') out_=true;
    if(myChar=='X') out_=true;
    if(myChar=='Y') out_=true;
    if(myChar=='Z') out_=true;
    if(myChar=='a') out_=true;
    if(myChar=='b') out_=true;
    if(myChar=='c') out_=true;
    if(myChar=='d') out_=true;
    if(myChar=='e') out_=true;
    if(myChar=='f') out_=true;
    if(myChar=='g') out_=true;
    if(myChar=='h') out_=true;
    if(myChar=='i') out_=true;
    if(myChar=='j') out_=true;
    if(myChar=='k') out_=true;
    if(myChar=='l') out_=true;
    if(myChar=='m') out_=true;
    if(myChar=='n') out_=true;
    if(myChar=='o') out_=true;
    if(myChar=='p') out_=true;
    if(myChar=='q') out_=true;
    if(myChar=='r') out_=true;
    if(myChar=='s') out_=true;
    if(myChar=='t') out_=true;
    if(myChar=='u') out_=true;
    if(myChar=='v') out_=true;
    if(myChar=='w') out_=true;
    if(myChar=='x') out_=true;
    if(myChar=='y') out_=true;
    if(myChar=='z') out_=true;
    if(myChar=='_') out_=true;
    if(myChar=='=') out_=true;
    //if(myChar=='#') out_=true;
    if(myChar=='.') out_=true;
    if(myChar==',') out_=true;
    if(myChar==':') out_=true;
    if(myChar=='"') out_=true;
    if(myChar=='*') out_=true;
    if(myChar=='+') out_=true;
    if(myChar=='/') out_=true;
    if(myChar==' ') out_=true;
    if(myChar=='@') out_=true;
    if(myChar=='\r') out_=true;
    if(myChar=='\n') out_=true;
    if(isDigit(myChar)) out_=true;
  return out_;
}