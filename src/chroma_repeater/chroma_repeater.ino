#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include "lights.h"
#include "pins.h"
#include "debug.h"

Lights lights;

void setup() {
    Serial.begin(9600);
    Mirf.spi = &MirfHardwareSpi;
    Mirf.cePin = A0;
    Mirf.csnPin = A1;
    Mirf.init();
    Mirf.setRADDR((byte *)"chroma");
    Mirf.payload = 3; /* for LEDs */
    Mirf.config();
}

void loop() {
    byte data[Mirf.payload];

    while(Mirf.isSending()); 

    if (Mirf.dataReady()) {
    	memset(data, 0, sizeof(data));
    	Mirf.getData(data);
    	lights.set(PIN_LED_BOTH, data[0], data[1], data[2]);
    	debug("%02X %02X %02X\r\n", data[0], data[1], data[2]);
    	Mirf.setTADDR((byte *)"colour");
    	for(int i=1; i<5; i++) {
    	    Mirf.send(data);
    	    delay(10);
    	}
    }
}


