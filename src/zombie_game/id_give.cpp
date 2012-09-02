//Runs on one Tilda to supply unique IDs at start of game
#include "RF24/RF24.h"
#include <avr/io.h>
#include <inttypes.h>
#include <avr/eeprom.h>
#include "lights.h"
#include <avr/interrupts.h>

uint16_t ID=0;

uint8_t assign=0; //assign mode

uint16_t getID(){  
   return eeprom_read_word(0x00); //read current ID from EEPROM
}

void setID(uint16_t val){
  eeprom_update_word(0x00, val); //update current ID in EEPROM
}

ISR(/*Button*/) {	//Triggered when button pushed. TODO: Set up ISR vector
  if (assign==1) {
    assign=0; //toggle assign mode
    Lights::set(0,0x00,0x00,0x00); //mode indicator
    setID(ID)
  }
  else if (assign==0) {
    assign=1;
    Lights::set(0,0x10,0xFF,0x50);
    ID = getID() //restore ID from EEPROM
  }
}

ID_response(){  //called whenever a new ID is requested
  transmitID(ID); //TODO Transmit ID using IR library
  ID++; 
  setID(ID);
}
