#include <SoftwareSerial.h>
#include "lights.h"
#include "pins.h"

Lights lights;

/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example LED Remote
 *
 * This is an example of how to use the RF24 class to control a remote
 * bank of LED's using buttons on a remote control.
 *
 * On the 'remote', connect any number of buttons or switches from
 * an arduino pin to ground.  Update 'acc_axes' to reflect the
 * pins used.
 *
 * On the 'led' board, connect the same number of LED's from an
 * arduino pin to a resistor to ground.  Update 'led_pins' to reflect
 * the pins used.  Also connect a separate pin to ground and change
 * the 'role_pin'.  This tells the sketch it's running on the LED board.
 *
 * Every time the buttons change on the remote, the entire state of
 * buttons is send to the led board, which displays the state.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(A0,A1);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipe = 0xFEFEFEFEE1LL;

enum eMode{
  IDLE = 0,
  SYNC = 1,
};

eMode mode = IDLE;
const unsigned long idleTime = 60000; //one min
unsigned long lastMsg = idleTime;//start up idling
//
// Setup
//

void setup(void)
{
  //
  // Print preamble
  //
  Serial.begin(57600);
  printf_begin();

  printf("\n\rRF24/Light reciever/\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();
  radio.setChannel(0x69);
  radio.setPayloadSize(4);
  radio.disableCRC();

  //
  // Open pipes to other nodes for communication
  //

  //setup radio
  radio.openReadingPipe(1,pipe);
  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //
  radio.printDetails();
}

//
// Loop
//

unsigned char rec_buffer[3];

void loop(void)
{
  // if there is data ready
  if ( radio.available() ) {
    // Dump the payloads until we've gotten everything
    bool done = false;
    while (!done){
      done = radio.read( rec_buffer, 3);
      //update idling time
      lastMsg = millis();
    }
    Serial.println(rec_buffer[0]);
//    printf("Setting LEDs to %d %d %d ", rec_buffer[0], rec_buffer[1], rec_buffer[2]);
    lights.set(PIN_LED_BOTH, rec_buffer[0], rec_buffer[1], rec_buffer[2]);
  }
  
  if(millis() - lastMsg > idleTime){
    mode = IDLE;
  }else{
    mode = SYNC;
  }
  

  if(mode == IDLE){
    led_cycle_nonblocking(&lights, PIN_LED_BOTH);
  }

}
// vim:ai:cin:sts=2 sw=2 ft=cpp




