#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include "lights.h"
#include "pins.h"
#include "debug.h"

Lights lights;
/* byte colour[] = { 0x55, 0x00, 0xFF }; */
byte colour[] = { 0xff, 0xaa, 0x22 };

void buttonPress() {
    // Interrupt handler when button state changes
    int state = digitalRead(PIN_BUTTON);
    if (state == LOW) {
	Mirf.setTADDR((byte *)"chroma");
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
	debug("%02X %02X %02X\r\n", data[0], data[1], data[2]);
    }
}


