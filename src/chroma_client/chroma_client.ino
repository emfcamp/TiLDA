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

byte colour[] = { COLOUR };

void buttonPress() {
    // Interrupt handler when button state changes
    int state = digitalRead(PIN_BUTTON);
    if (state == LOW) {
	Mirf.setTADDR((byte *)"chroma");
	debug("SND: %002X %02X %02X\r\n", colour[0], colour[1], colour[2]);
	Mirf.send(colour);
    }
}

void setup() {
    Serial.begin(9600);
    Mirf.spi = &MirfHardwareSpi;
    Mirf.cePin = A0;
    Mirf.csnPin = A1;
    Mirf.init();
    Mirf.setRADDR((byte *)"colour");
    Mirf.payload = 3; /* for LEDs */
    Mirf.config();

    pinMode(PIN_BUTTON, INPUT_PULLUP);
    attachInterrupt(INT_BUTTON, buttonPress, CHANGE);
}

void loop() {
    if (!Mirf.isSending() && Mirf.dataReady()) {
	byte data[Mirf.payload];
	memset(data, 0, sizeof(data));
	Mirf.getData(data);
	lights.set(PIN_LED_BOTH, data[0], data[1], data[2]);
	debug("RCV %02X %02X %02X\r\n", data[0], data[1], data[2]);
    }
}


