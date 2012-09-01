#include "lights.h"
#include "pins.h"

Lights lights;

unsigned long last_button_down = 0;
bool torch_on = false;

bool check_torch_mode(int state) {
  if (state == LOW) { // Low means pressed
    last_button_down = millis();
  } else {
    if (last_button_down != 0 && last_button_down < millis() - 1000) {
      // Engage torch mode
      lights.set(PIN_LED_LEFT, 255, 255, 255);
      torch_on = true;
      return true;
    }
    if (torch_on) {
      lights.set(PIN_LED_LEFT, 0, 0, 0);
      torch_on = false;
      return true;
    }
  }
  return false;
}

void button_press() {
  // Interrupt handler when button state changes
  int state = digitalRead(PIN_BUTTON);
  if (check_torch_mode(state)) {
    return;
  }
}

void setup() {
  led_cycle(&lights, PIN_LED_LEFT, 250);
  led_cycle(&lights, PIN_LED_RIGHT, 250);
  lights.set(PIN_LED_BOTH, 0, 0, 0);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(INT_BUTTON, button_press, CHANGE);
}

long loop_counter = 0;

void loop() {

  // TiLDA can either have one LED lit, or both
  // the same colour.
  
  if ( loop_counter == 0 ) {
      lights.set(PIN_LED_LEFT, 0, 0, 255);
  } else if ( loop_counter == 5000 ) {
      lights.set(PIN_LED_RIGHT, 255, 0, 0);
  } else if ( loop_counter == 7000 ) {
      lights.set(PIN_LED_BOTH, 255, 255, 255);
  } 

  delay(1);
  loop_counter++;

  loop_counter %= 10000;

}


