#include "printf.h"
#include "RF24.h"

RF24 radio(A1,A0);

void setup(void) {
  printf_begin();
}

void begin(void) {

  radio.begin();

  if (radio.setDataRate(RF24_1MBPS)) {
    printf("Setting data rate worked\r\n");
  } else {
    printf("Setting data rate failed\r\n");
  }

  radio.printDetails();
}

void loop(void) {

  if (Serial.available()) {
    char c = tolower(Serial.read());
    if (c == 't') begin();
  }
}
