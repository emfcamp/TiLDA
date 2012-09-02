#include "RF24/RF24.h"
#include <avr/io.h>
#include <arduino.h>
#include <inttypes.h>
#include <avr/eeprom.h>
#include "lights.h"
#include <avr/interrupts.h>

uint16_t ID;

void assignID(uint16_t val){
  eeprom_update_word(0x00, val);
}

ISR(Button) {	//update ISR name
    Lights::set(0,0x10,0xFF,0x50);
    ID=requestID()
    assignID(ID)
    Lights::set(0,0x00,0x00,0x00);
}
