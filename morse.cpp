/*
  Morse.cpp - Library for flashing Morse code.
*/

#include "Arduino.h"
#include "Morse.h"

Morse::Morse(int pin) {
  _pin = pin;
}

void Morse::begin() {
  pinMode(_pin, OUTPUT);
}

void Morse::dot() {
  digitalWrite(_pin, HIGH);
  delay(250);
  digitalWrite(_pin, LOW);
  delay(250);
}

void Morse::dash() {
  digitalWrite(_pin, HIGH);
  delay(1000);
  digitalWrite(_pin, LOW);
  delay(250);
}