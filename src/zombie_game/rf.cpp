//Use Mirf library to send and receive ID packets
#define MAGICSIG 0xE14F //placed at the start of packets as identifier
#include "rf.h"
#include "ID.h"

void rf_setup(){
  Serial.begin(9600);
  /*
   * Setup pins / SPI.
   */
   
  /* To change CE / CSN Pins:
   * 
   * Mirf.csnPin = 9;
   * Mirf.cePin = 7;
   */
  Mirf.cePin = A0;
  Mirf.csnPin = A1;

  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  
  /*
   * Configure reciving address.
   */
   
  Mirf.setRADDR((byte *)"clie1");
  
  /*
   * Set the payload length to sizeof(unsigned long) the
   * return type of millis().
   *
   * NB: payload on client and server must be the same.
   */
   
  Mirf.payload = 4;
  
  /*
   * Write channel and payload config then power up reciver.
   */
   
  /*
   * To change channel:
   * 
   * Mirf.channel = 10;
   *
   * NB: Make sure channel is legal in your area.
   */
   
  Mirf.config();
}

ISR(SIG_OUTPUT_COMPARE0){
  //ping and poll on OC0
  Lights::set(0,0x10,0xFF,0x50); //Set transmit indicator
  if(!Mirf.isSending() && Mirf.dataReady()){
    /*
     * Get load the packet into the buffer.
     */
     
    uint16_t indata[2];
    Mirf.getData(indata);
    if (indata[0]=MAGICSIG){ 
      incr(indata[1]) //Not quite right. Needs to check if the ID is already visible.
    
    /*
     * Set the send address.
     */
  }
 
  Mirf.setTADDR((byte *)"clie1");
  uint16_t outdata[2];
  outdata[0]=MAGICSIG;
  outdata[1]=ID;
  Mirf.send((byte *)outdata);
  while(Mirf.isSending()){
  }
  Lights::set(0,0x0,0x00,0x00);
} 
