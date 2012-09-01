#include "lights.h"
#include "pins.h"

Lights lights;

unsigned long last_button_down = 0;
bool torch_on = false;

bool check_torch_mode(int state) {
  if (state == LOW) { // Low means pressed
    led_stop_cycle();
    last_button_down = millis();
  } else {
    if (last_button_down != 0 && last_button_down < millis() - 1000) {
      // Engage torch mode
      lights.set(PIN_LED_BOTH, 255, 255, 255);
      torch_on = true;
      return true;
    }
    if (torch_on) {
      lights.set(PIN_LED_BOTH, 0, 0, 0);
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
  pinMode(PIN_IR_OUT, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  attachInterrupt(INT_BUTTON, button_press, CHANGE);
}

void loop() {
  if(!torch_on) {
    led_cycle(&lights, PIN_LED_BOTH, 1000);
    lights.set(PIN_LED_BOTH, 0, 0, 0);
  } else {
    led_stop_cycle();
  }
}


