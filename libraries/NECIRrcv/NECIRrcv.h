#ifndef NECIRrcv_h
#define NECIRrcv_h

#include "WConstants.h"

#define USECPERTICK 50  // microseconds per clock interrupt tick
#define CLKFUDGE 5      // fudge factor for clock interrupt overhead
#define CLKMAX 256      // max value for clock (timer 2)
#define PRESCALE 8      // timer2 clock prescale
#define SYSCLOCK 16000000  // main Arduino clock
#define CLKSPERUSEC (SYSCLOCK/PRESCALE/1000000)   // timer clocks per microsecond

#define MAXBUF 8       // IR command code buffer length (circular buffer)

// IR detector output is active low
#define MARK  0
#define SPACE 1

#define NBITS 32         // bits in IR code

#define BLINKLED 13

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// clock timer reset value
#define INIT_TIMER_COUNT2 (CLKMAX - USECPERTICK*CLKSPERUSEC + CLKFUDGE)
#define RESET_TIMER2 TCNT2 = INIT_TIMER_COUNT2

// pulse parameters -- nominal usec
#define STARTNOM      9000
#define SPACENOM      4500
#define BITMARKNOM    620
#define ONESPACENOM   1600
#define ZEROSPACENOM  480
#define RPTSPACENOM   2180

#define TOLERANCE 20  // percent
#define LTOL (1.0 - TOLERANCE/100.) 
#define UTOL (1.0 + TOLERANCE/100.) 

// pulse parameters (tick counts)
#define STARTMIN (int)((STARTNOM/USECPERTICK)*LTOL) // start MARK
#define STARTMAX (int)((STARTNOM/USECPERTICK)*UTOL) 
#define SPACEMIN (int)((SPACENOM/USECPERTICK)*LTOL) 
#define SPACEMAX (int)((SPACENOM/USECPERTICK)*UTOL) 
#define BITMARKMIN (int)((BITMARKNOM/USECPERTICK)*LTOL-2) // extra tolerance for low counts
#define BITMARKMAX (int)((BITMARKNOM/USECPERTICK)*UTOL+2) 
#define ONESPACEMIN (int)((ONESPACENOM/USECPERTICK)*LTOL) 
#define ONESPACEMAX (int)((ONESPACENOM/USECPERTICK)*UTOL) 
#define ZEROSPACEMIN (int)((ZEROSPACENOM/USECPERTICK)*LTOL-2) 
#define ZEROSPACEMAX (int)((ZEROSPACENOM/USECPERTICK)*UTOL+2) 
#define RPTSPACEMIN (int)((RPTSPACENOM/USECPERTICK)*LTOL) 
#define RPTSPACEMAX (int)((RPTSPACENOM/USECPERTICK)*UTOL) 

// receiver states
#define IDLE     1
#define STARTH   2
#define STARTL   3
#define BIT      4
#define ONE      5
#define ZERO     6
#define STOP     7
#define BITMARK  8
#define RPTMARK  9

// macros
#define GETIR(X) ((byte)digitalRead(X))    // used to read IR pin
#define nextstate(X) (irparams.rcvstate = X)

// state machine variables irparams
static volatile struct {
  byte rcvstate ;          // IR receiver state
  byte bitcounter ;        // bit counter
  byte irdata ;            // MARK or SPACE read from IR input pin
  byte fptr ;              // irbuf front pointer
  byte rptr ;              // irbuf rear pointer
  byte irpin ;             // pin for IR data from detector
  byte blinkflag ;         // TRUE to enable blinking of pin 13 on IR processing
  unsigned int timer ;     // state timer
  unsigned long irmask ;   // one-bit mask for constructing IR code
  unsigned long ircode ;   // IR code
  unsigned long irbuf[MAXBUF] ;    // circular buffer for IR codes
} irparams ;

// main class
class NECIRrcv
{
  public:
    NECIRrcv(int irpin);
    unsigned long read();
    void begin();
    int available() ;
    void flush() ;
    void blink13(int blinkflag) ;
  private:
} ;
#endif
