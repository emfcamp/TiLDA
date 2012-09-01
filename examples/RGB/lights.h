#ifndef LIGHTS_H
#define LIGHTS_H

#include "Arduino.h"
#include "pins.h"

class Lights {
  public:
  Lights();
  void set(int light, unsigned char red, unsigned char green, unsigned char blue);
};

void led_cycle(Lights *lights, int light, int time);
void led_stop_cycle();

#endif // LIGHTS_H
