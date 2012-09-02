#include "RF24/RF24.h"
#include <avr/io.h>
#include <inttypes.h>
#include <avr/eeprom.h>
#include "lights.h"
#include <avr/interrupts.h>

uint16_t ID=0;

uint8_t assign=0; //assign mode

uint16_t getID(){
   return eeprom_read_word(0x00);
}

void setID(uint16_t val){
  eeprom_update_word(0x00, val);
}

ISR(Button) {	//update ISR name
  if (assign==1) {
    assign=0;
    Lights::set(0,0x10,0xFF,0x50);
    setID(ID)
  }
  else if (assign==0) {
    assign=1;
    Lights::set(0,0x10,0xFF,0x50);
    ID = getID()
  }
}

ISR(ID_request){
  transmitID(ID);
  ID++;
  setID(ID);
}
