#include <SoftwareSerial.h>

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

RF24 radio(9,10);

//we're only ever sending midi
SoftwareSerial midiSerial(A2, A3); // RX, TX
const int vPin = A0;
const int gndPin = A1;
const int ledPin = 13;

enum midi{
  NOTE_OFF = 0,
  NOTE_ON = 1,
  CC = 3,
  PITCH_BEND = 6,
};

const char SYNC = 248;
const char CONTINUE = 251;
const char START = 250;

const char numColours = 8;
unsigned char colours[8][3] = {
  {
    255, 0, 0  }
  ,
  {
    0, 255, 0  }
  ,
  {
    0, 0, 255   }
  ,
  {
    255, 255, 255  }
  ,
  {
    255, 255, 0  }
  ,
  {
    0, 255, 255  }
  ,
  {
    255, 0, 255   }
  ,
  {
    255, 255, 255  }
};
char current_colour = 0;

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipe = 0xFEFEFEFEE1LL;

//
// Setup
//

void setup(void)
{

  pinMode(ledPin, OUTPUT); 

  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();

  printf("\n\rRF24/Clock send/\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();
  radio.disableCRC();
  radio.setChannel(0x69);
  radio.setPayloadSize(4);

  //
  // Open pipes to other nodes for communication
  //
  //setup midi
  //Source midi v+ from digi pin
  pinMode(vPin, OUTPUT);
  digitalWrite(vPin, HIGH);
  //GND
  pinMode(gndPin, OUTPUT);
  digitalWrite(gndPin, LOW);

  //Debug over the serial
  Serial.begin(57600);
  Serial.println("Initialising midi");
  //midi begin here
  midiSerial.begin(31250);
  //setup radio
  radio.openWritingPipe(pipe);

  //
  // Dump the configuration of the rf unit for debugging
  //
  radio.printDetails();
}

//
// Loop
//

unsigned int ticks = 0;

void loop(void)
{
  if (midiSerial.available()){
    char c = midiSerial.read();
    if(c == SYNC){
      ticks++;
      if(ticks % (24 * 4) == 0){
        printf("Now sending... %u [%u,\t%u,\t%u]", current_colour, colours[current_colour][0],colours[current_colour][1],colours[current_colour][2]);
        bool ok = radio.write(colours[current_colour], 3);
        if(!ok) ok = radio.write(colours[current_colour], 3);
        Serial.println(ok ? "ok":"FAILED");
        current_colour++;
        if(current_colour == numColours){
          current_colour = 0;
        }
      }
    }
    else if(c == CONTINUE || c == START){
      ticks = 0;
      current_colour = 0;
      Serial.println("Restart sync");
    }
  }
  delay(10);
}

void flash(){
  digitalWrite(ledPin, HIGH);
  delay(10);
  digitalWrite(ledPin, LOW);
}
// vim:ai:cin:sts=2 sw=2 ft=cpp






