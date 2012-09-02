#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include "lights.h"
#include "pins.h"
#include "debug.h"

Lights lights;

#ifndef COLOUR
#define COLOUR 0x00, 0xFF, 0x00
#endif

/* state for the LEDs */
byte my_colour[3] = { COLOUR };
byte peer_colour[3];
/* state for the button */
int last_state = HIGH;

void setup() {
    Serial.begin(9600);
    Mirf.spi = &MirfHardwareSpi;
    Mirf.cePin = A0;
    Mirf.csnPin = A1;
    Mirf.init();
    Mirf.setRADDR((byte *)"chroma");
    Mirf.payload = 3; /* for LEDs */
    Mirf.config();

    pinMode(PIN_BUTTON, INPUT_PULLUP);

    memset(peer_colour, 0, sizeof(peer_colour));
}


void loop() {
    if (!Mirf.isSending() && Mirf.dataReady()) {
        Mirf.getData(peer_colour);
        debug("RCV %02X %02X %02X\r\n", peer_colour[0], peer_colour[1], peer_colour[2]);
	lights.set(PIN_LED_BOTH, peer_colour[0], peer_colour[1], peer_colour[2]);
    }

    int state = digitalRead(PIN_BUTTON);
    if (state == HIGH && last_state == LOW) {
        Mirf.setTADDR((byte *)"chroma");
        debug("SND: %02X %02X %02X\r\n", my_colour[0], my_colour[1], my_colour[2]);
        Mirf.send(my_colour);
	lights.set(PIN_LED_BOTH, my_colour[0], my_colour[1], my_colour[2]);
    }
    last_state = state;
}


