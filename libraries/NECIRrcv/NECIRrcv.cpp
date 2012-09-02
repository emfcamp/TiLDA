// NECIRrcv
// Joe Knapp   jmknapp AT gmail DOT com

#include "WProgram.h"
#include "NECIRrcv.h"

NECIRrcv::NECIRrcv(int irpin)
{
  irparams.irpin = irpin ;
}

// initialization
void NECIRrcv::begin() {
  // setup pulse clock timer interrupt
  TCCR2A = 0;  // normal mode

  //Prescale /8 (16M/8 = 0.5 microseconds per tick)
  // Therefore, the timer interval can range from 0.5 to 128 microseconds
  // depending on the reset value (255 to 0)
  cbi(TCCR2B,CS22) ;
  sbi(TCCR2B,CS21) ;
  cbi(TCCR2B,CS20) ;

  //Timer2 Overflow Interrupt Enable
  sbi(TIMSK2,TOIE2) ;

  RESET_TIMER2;

  sei();  // enable interrupts
  
  // initialize state machine variables
  irparams.rcvstate = IDLE ;
  irparams.bitcounter = 0 ;
  irparams.ircode = 0 ;
  irparams.fptr = 0 ;
  irparams.rptr = 0 ;
  irparams.blinkflag = 0 ;

  // set pin modes
  pinMode(irparams.irpin, INPUT) ;
}

// return next IR code from buffer, or -1 if none
unsigned long NECIRrcv::read()
{
  unsigned long ircode ;
  if (irparams.fptr != irparams.rptr) {
    ircode = irparams.irbuf[irparams.rptr] ;
    irparams.rptr = (irparams.rptr + 1) % MAXBUF ;
    return((unsigned long)ircode) ;
  }
  else
    return((unsigned long)-1) ;
}

// return number of IR codes in buffer
int NECIRrcv::available()
{
  int n ;
  n = irparams.fptr - irparams.rptr ; 
  if (n < 0)
    n += MAXBUF ;
  return(n) ;
}

// enable/disable blinking of pin 13 on IR processing
void NECIRrcv::blink13(int blinkflag)
{
  irparams.blinkflag = blinkflag ;
  if (blinkflag)
    pinMode(BLINKLED, OUTPUT) ;
}

// flush IR code buffer
void NECIRrcv::flush()
{
  irparams.rptr = irparams.fptr ;
}

// IR receiver state machine (TIMER2 interrupt) 
ISR(TIMER2_OVF_vect)
{
  RESET_TIMER2 ;

  irparams.irdata = GETIR(irparams.irpin) ;
  
  if (irparams.blinkflag && (irparams.rcvstate != IDLE))
    PORTB |= B00100000 ;  // turn pin 13 LED on
  
  // process current state
  switch(irparams.rcvstate) {
    case IDLE:
      if (irparams.irdata == MARK) {  // got some activity
          nextstate(STARTH) ;
          irparams.timer = 0 ;
      }
      break ;
    case STARTH:   // looking for initial start MARK
      // entered on MARK
      if (irparams.irdata == SPACE) {   // MARK ended, check time
        if ((irparams.timer >= STARTMIN) && (irparams.timer <= STARTMAX)) {
          nextstate(STARTL) ;  // time OK, now look for start SPACE
          irparams.timer = 0 ;
        }
        else
          nextstate(IDLE) ;  // bad MARK time, go back to IDLE
      }
      else
        irparams.timer++ ;  // still MARK, increment timer
      break ;
    case STARTL:
      // entered on SPACE
      if (irparams.irdata == MARK) {  // SPACE ended, check time
        if ((irparams.timer >= SPACEMIN) && (irparams.timer <= SPACEMAX)) {
          nextstate(BITMARK) ;  // time OK, check first bit MARK 
          irparams.timer = 0 ;
          irparams.bitcounter = 0 ;  // initialize ircode vars
          irparams.irmask = (unsigned long)0x1 ;
          irparams.ircode = 0 ;
        }
        else if ((irparams.timer >= RPTSPACEMIN) && (irparams.timer <= RPTSPACEMAX)) {  // not a start SPACE, maybe this is a repeat signal
          nextstate(RPTMARK) ;   // yep, it's a repeat signal
          irparams.timer = 0 ;
        }
        else
          nextstate(IDLE) ;  // bad start SPACE time, go back to IDLE
      }
      else {   // still SPACE
        irparams.timer++ ;    // increment time
        if (irparams.timer >= SPACEMAX)  // check against max time for SPACE
          nextstate(IDLE) ;  // max time exceeded, go back to IDLE
      }
      break ;
    case RPTMARK:
      irparams.timer++ ;  // measuring MARK
      if (irparams.irdata == SPACE) {  // MARK ended, check time
        if ((irparams.timer >= BITMARKMIN) && (irparams.timer <= BITMARKMAX))
          nextstate(IDLE) ;  // repeats are ignored here, just go back to IDLE
        else
          nextstate(IDLE) ;  // bad repeat MARK time, go back to IDLE
      }
      break ;
    case BITMARK:
      irparams.timer++ ;   // timing MARK
      if (irparams.irdata == SPACE) {   // MARK ended, check time
        if ((irparams.timer < BITMARKMIN) || (irparams.timer > BITMARKMAX))
          nextstate(IDLE) ;  // bad MARK time, go back to idle
        else {
          irparams.rcvstate = BIT ;  // MARK time OK, go to BIT
          irparams.timer = 0 ;
        }
      }
      break ;
    case BIT:
      irparams.timer++ ; // measuring SPACE
      if (irparams.irdata == MARK) {  // bit SPACE ended, check time
        if ((irparams.timer >= ONESPACEMIN) && (irparams.timer <= ONESPACEMAX)) {
          nextstate(ONE) ;   // SPACE matched ONE timing
          irparams.timer = 0 ;
        }
        else if ((irparams.timer >= ZEROSPACEMIN) && (irparams.timer <= ZEROSPACEMAX)) {
          nextstate(ZERO) ;  // SPACE matched ZERO timimg
          irparams.timer = 0 ;
        }
        else
          nextstate(IDLE) ;  // bad SPACE time, go back to IDLE
      }
      else {  // still SPACE, check against max time
        if (irparams.timer > ONESPACEMAX)
          nextstate(IDLE) ;  // SPACE exceeded max time, go back to IDLE
      }
      break ;
    case ONE:
      irparams.ircode |= irparams.irmask ;  // got a ONE, update ircode
      irparams.irmask <<= 1 ;  // set mask to next bit
      irparams.bitcounter++ ;  // update bitcounter
      if (irparams.bitcounter < NBITS)  // if not done, look for next bit
        nextstate(BITMARK) ;
      else
        nextstate(STOP) ;  // done, got NBITS, go to STOP
      break ;
    case ZERO:
      irparams.irmask <<= 1 ;  // got a ZERO, update mask
      irparams.bitcounter++ ;  // update bitcounter
      if (irparams.bitcounter < NBITS)  // if not done, look for next bit
        nextstate(BITMARK) ;
      else
        nextstate(STOP) ;  // done, got NBITS, go to STOP
      break ;
    case STOP:
      irparams.timer++ ;  //measuring MARK
      if (irparams.irdata == SPACE) {  // got a SPACE, check stop MARK time
        if ((irparams.timer >= BITMARKMIN) && (irparams.timer <= BITMARKMAX)) {
          // time OK -- got an IR code
          irparams.irbuf[irparams.fptr] = irparams.ircode ;   // store code at fptr position
          irparams.fptr = (irparams.fptr + 1) % MAXBUF ; // move fptr to next empty slot
        }
        nextstate(IDLE) ;  // finished with this code, go back to IDLE
      }
      break ;
  }
  // end state processing

  if (irparams.blinkflag)
    PORTB &= B11011111 ;  // turn pin 13 LED off
}
