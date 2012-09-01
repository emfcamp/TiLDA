//

// chip specifics
// tiny45
#define EEPROM_SIZE 256
// tiny25
//#define EEPROM_SIZE 128

#define EEPROM_SEND_DELAY 1000

// common code so we know def is a badge
#define OUR_COMMON_CODE 0xbb

#define MY_CODE_HEADER  (long)(OUR_COMMON_CODE)<<24 | (long)(my_id) <<16

// how many badges do we keep an record of, for calculating "i've seen you recently"
#define BADGE_BUFFER_SIZE 20
// and how long before we say, 'ok, seen you again'
#define BADGE_LAST_SEEN_MAX 30

// debugging flags
//#define TURN_OFF_COLOUR_DISPLAY 1
//#define TURN_OFF_PWM_COLOUR 1
//#define DISABLE_IR_SENDING_CODE 1
//#define DISABLE_EEPROM_SENDING_CODE 1
#define ENABLE_FLASH_BYTE_CODE 1

// our data packet is OUR_COMMON_CODE << 24 | MY_ID << 16 | mode | data(=0b00{colour})
#define COMMON_CODE_MASK 0xff000000
#define ID_MASK          0x00ff0000
#define MODE_MASK        0x0000ff00
#define DATA_MASK        0x000000ff

// our 'game modes'
#define INIT_MODE            0x01
#define SENDING_COLOUR       0x02
#define AM_ZOMBIE            0x03
#define AM_INFECTED          0x04
#define SEND_ALL_EEPROM      0x05
#define REFLECT_COLOUR       0x06
#define CYCLE_COLOURS_SEEN   0x07
#define SEND_ME_YOUR_DATA    0x08
#define PROGRAM_BADGE_ID     0x09

// some debugging modes
#define DEBUG_TURN_OFF_DISPLAY 0b00000001
#define DEBUG_TURN_OFF_SENDNEC 0b00000001

// Zombie game
#define BITTEN_MAX         3
#define MAX_TIME_INFECTED  30
#define MAX_TIME_DEAD      10

// Apple codes (device id is first byte, mine is 5c)
#define APPLE_PLAY              0x000487ee
#define APPLE_VOLUME_UP         0x000b87ee
#define APPLE_VOLUME_DOWN       0x000d87ee
#define APPLE_NEXT_TRACK        0x000787ee
#define APPLE_PREV_TRACK        0x000887ee
#define APPLE_MENU              0x000287ee

// delay for flashing out the recd code
#define IR_DATA_PRINT_DELAY 50000

#define MAX_SAME_COLOUR_RECV 5

// for matt's design
#define bogusMask    0b00100000
#define redMask      0b00000100
#define grnMask      0b00000010
#define bluMask      0b00000001
#define rgbMask      0b00000111
#define irInMask     0b00001000
#define irOutMask    0b00010000
// 4 = PB3
#define irInPortBPin  4

#define JUST_RED_ON      PORTB |= rgbMask; PORTB &= ~(redMask);
#define JUST_GREEN_ON    PORTB |= rgbMask; PORTB &= ~(grnMask);
#define JUST_BLUE_ON     PORTB |= rgbMask; PORTB &= ~(bluMask);

#define FLASH_BLUE      PORTB ^= bluMask; delay_ten_us(10000); PORTB ^= bluMask;
#define FLASH_GREEN     PORTB ^= grnMask; delay_ten_us(10000); PORTB ^= grnMask;
#define FLASH_RED       PORTB ^= redMask; delay_ten_us(10000); PORTB ^= redMask;

#define RED 0
#define GREEN 1
#define BLUE 2
#define displayRedMask 0b00000011
#define displayGrnMask 0b00001100
#define displayBluMask 0b00110000
#define displayRGBMask 0b00111111

// how many times to send our IR code in each 1s loop.
#define NUM_SENDS 1

#define CLKFUDGE 5        // fudge factor for clock interrupt overhead
#define CLK 256           // max value for clock (timer 0)
#define PRESCALE 8        // timer0 clock prescale
#define SYSCLOCK 8000000  // main clock speed
#define CLKSPERUSEC (SYSCLOCK/PRESCALE/1000000)   // timer clocks per microsecond
#define USECPERTICK 50  // microseconds per clock interrupt tick

#define INIT_TIMER_COUNT0 (CLK - USECPERTICK*CLKSPERUSEC + CLKFUDGE)
#define RESET_TIMER0 TCNT0 = INIT_TIMER_COUNT0

// IR detector output is active low
#define MARK  0
#define SPACE 1

#define NBITS 32         // bits in IR code

#define TOPBIT 0x80000000

#define TOLERANCE 25  // percent tolerance in measurements
#define LTOL (1.0 - TOLERANCE/100.)
#define UTOL (1.0 + TOLERANCE/100.)

#define STARTNOM      9000
#define SPACENOM      4500
#define BITMARKNOM    620
#define ONESPACENOM   1600
#define ZEROSPACENOM  480
#define RPTSPACENOM   2180

#define NEC_HDR_MARK    9000
#define NEC_HDR_SPACE   4500
#define NEC_BIT_MARK    560
#define NEC_ONE_SPACE   1600
#define NEC_ZERO_SPACE  560
#define NEC_RPT_SPACE   2250

#define TICKS_LOW(us) (int) (((us)*LTOL/USECPERTICK))
#define TICKS_HIGH(us) (int) (((us)*UTOL/USECPERTICK + 1))

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

#define IRBUF_CUR irparams.irbuf[j]

// macros
#define nextstate(X) (irparams.rcvstate = X)

// repeat code for NEC
#define REPEAT 0xffffffff

// Values for decode_type
#define NONE 0
#define NEC 1

#define _GAP 5000 // Minimum gap between transmissions
#define GAP_TICKS (_GAP/USECPERTICK)

#define MAXBUF 4       // IR command code buffer length (circular buffer)



