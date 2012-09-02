#include "id_receive.h"
#include "RF24/RF24.h"
#include <avr/io.h>
#include <Arduino.h>
#include <inttypes.h>
#include <avr/eeprom.h>
#include "lights.h"
#include <avr/interrupts.h>

void main(){
  OCR0A= PING_DELAY; //set interrupt interval
  TCCR0B=(1<<WGM1)|(1<<CS02)|(1<<CS00); //set CTC mode and enable clock with 1/1024 scaler
  TIMSK0=(1<<OCIE0A); //enable interrupt
  rf_setup();
}
