
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

#include <IRremote.h>
#include <EEPROM.h>

#include "zombie.h"
#include "pins.h"
#include "lights.h"

Lights lights;

// state machine variables irparams
typedef struct badge_record {
  //char badge_id ;
  uint8_t badge_id ;
  int     first_seen ;
} badge_record_t;

badge_record_t global_badge_buffer[BADGE_BUFFER_SIZE];

// badge display modes
enum colour_state {
  INIT_COLOUR_MODE,
  DOUBLE_WHITE_TORCH,
  DOUBLE_RED_TORCH,
  GAME_STATE
};

uint8_t l_red   = 255;
uint8_t l_green = 32;
uint8_t l_blue  = 32;

uint8_t r_red   = 32;
uint8_t r_green = 32;
uint8_t r_blue  = 255;


uint8_t last_eeprom_read = 1;
uint8_t enable_rgb_led = 1;
uint8_t curr_colour = 0;
uint8_t curr_r = 0;
uint8_t curr_g = 0;
uint8_t curr_b = 0;
uint8_t my_id = 0xff;
uint8_t my_mode = INIT_MODE;
colour_state my_colour_mode = INIT_COLOUR_MODE;
uint8_t debug_modes = 0x00;
uint8_t rgb_colours[3] = { 1, 81, 161 }; // curr_colour values for R, G, and B.
uint8_t buffer_count = 0;
uint8_t factory_reset_keycombo_count = 0;

// counts 'ticks' (kinda-seconds) of main loop
unsigned int     main_loop_counter = 0;

uint8_t bit_by_zombie_count = 0;
int     time_infected = 0;

void delay_ten_us(unsigned int us) {
  unsigned int count;

  while (us != 0) {
    for (count=0; count <= 8; count++) {
            PINB |= bogusMask;
    }
    us--;
  }
}

int calc_eeprom_address_from_id(int id) {
  return (2*id + 10);
}

void display_colour(uint8_t tick) {

}

void HSVtoRGB( uint8_t *r, uint8_t *g, uint8_t *b, uint8_t hue, uint8_t s, uint8_t v )
{
    int f;
    long p, q, t;

    if( s == 0 )
    {
        // easy - just shades of grey.
        *r = *g = *b = v;
        return;
    }

    // special case, treat hue=0 as black
    //if ( hue == 0) {
    //    *r = *g = *b = 0;
    //}
 
    // hue is from 1-240, where 40=60deg, 80=120deg, so we can fit into a byte
    f = ((hue%40)*255)/40;
    hue /= 40;
 
    p = (v * (256 - s))/256;
    q = (v * ( 256 - (s * f)/256 ))/256;
    t = (v * ( 256 - (s * ( 256 - f ))/256))/256;
 
    switch( hue ) {
    case 0:
        *r = v;
        *g = t;
        *b = p;
        break;
    case 1:
        *r = q;
        *g = v;
        *b = p;
        break;
    case 2:
        *r = p;
        *g = v;
        *b = t;
        break;
    case 3:
        *r = p;
        *g = q;
        *b = v;
        break;
    case 4:
        *r = t;
        *g = p;
        *b = v;
        break;
    default:
        *r = v;
        *g = p;
        *b = q;
        break;
    }
}
 
void flash_byte(uint8_t data) {
#ifdef ENABLE_FLASH_BYTE_CODE
    for (uint8_t i=0; i<8; i++) {
        if ( data & 1 ) {
          lights.set(PIN_LED_LEFT, 255, 0, 0);
          delay(500);
          lights.set(PIN_LED_LEFT, 0, 0, 0);
        } else {
          lights.set(PIN_LED_RIGHT, 0, 0, 255);
          delay(500);
          lights.set(PIN_LED_RIGHT, 0, 0, 0);
        }
        data >>= 1;
    }
#endif
}

void update_recd_id_in_eeprom(int id) {
    // have we seen them before?
    // if not, record that we have
    unsigned int times_seen = EEPROM.read(calc_eeprom_address_from_id(id)) << 8 + EEPROM.read(calc_eeprom_address_from_id(id)+1) ;
    if ( times_seen == 65535 ) {
        // new id, since eeprom is initialised to 0xFFFF
        write_to_eeprom_id_slot(id, 1);
    } else if (times_seen < 65000 ) {
        write_to_eeprom_id_slot(id, times_seen + 1);
    } // otherwise seen max (65000) times, leave at that.
}

void write_to_eeprom_id_slot(int id, int v) {
    // need to write value to two EEPROM byte slots
    EEPROM.write(calc_eeprom_address_from_id(id), uint8_t(v >> 8)); // highest byte
    EEPROM.write(calc_eeprom_address_from_id(id) + 1, uint8_t((v << 8) >> 8)); // lowest byte
}

uint8_t have_not_seen_id_recently(int recd_id) {
    for ( uint8_t i=0; i<BADGE_BUFFER_SIZE; i++ ) {
        if ( global_badge_buffer[i].badge_id == recd_id ) {
            if ( global_badge_buffer[i].first_seen + BADGE_LAST_SEEN_MAX > main_loop_counter ) {
                return 0;
            } else {
                // blank that we've seen the badge, otherwise we'll get duplicates
                // in the buffer
                global_badge_buffer[i].badge_id = 0;
                global_badge_buffer[i].first_seen = 0;
            }
        }
    }
    return 1;
}

void record_that_we_have_seen_badge(int id) {
    if ( have_not_seen_id_recently(id) ) {
        global_badge_buffer[buffer_count].badge_id = id;
        global_badge_buffer[buffer_count].first_seen = main_loop_counter;
        update_recd_id_in_eeprom(id);
        buffer_count = ( buffer_count + 1 ) % BADGE_BUFFER_SIZE;
    }
}

void process_badge_message(unsigned long code) {

    // recving from a known badge
    //
    int recd_id = (code & ID_MASK) >> 16;

    //
    //if (recd_id == 0x0 || recd_id == 0xff ) {
        // not badge ids, ignore.
    //    return;
    //}

#ifdef DEBUG_DISPLAY_DATA_RECEIVED
    flash_byte(recd_id);
#endif

    // TODO: only update to eeprom if we have not 
    //       seen the badge for a while
    if ( have_not_seen_id_recently(recd_id) ) {
        record_that_we_have_seen_badge(recd_id);
    }

    // what mode are they in?
    uint8_t recd_mode = (code & MODE_MASK) >> 8;

#ifdef DEBUG_DISPLAY_DATA_RECEIVED
    flash_byte(recd_mode);
#endif

    // what data did they send me?
    uint8_t recd_data = (code & DATA_MASK);

    if (my_mode == INIT_MODE && recd_mode != PROGRAM_BADGE_ID ) {
        return;
    }

    if (my_mode == REFLECT_COLOUR ) {
        curr_colour = recd_data;
        return;
    }

    if (recd_mode == AM_ZOMBIE) {
        // eek
        if ( bit_by_zombie_count > BITTEN_MAX ) {
            // oh noes! 
            my_mode = AM_INFECTED;
            bit_by_zombie_count = 0;
            time_infected = main_loop_counter;
        } else {
            // munch munch
            bit_by_zombie_count++;
        }

    } else if (recd_mode == SEND_ALL_EEPROM) {
        // ignore them, only collection stations need to listen

    } else if (recd_mode == SEND_ME_YOUR_DATA ) {
        // ooh, a collection station, upload!
        my_mode = SEND_ALL_EEPROM;

    } else if (recd_mode == PROGRAM_BADGE_ID ) {
        if ( my_mode == INIT_MODE ) {
            // sweet, getting ID from registration station
            EEPROM.write((int)0, recd_data);
        } // but if we're not in INIT_MODE, we've already got one.

    } else if (recd_mode == CYCLE_COLOURS_SEEN) {
        if ( my_mode == AM_INFECTED ) {
            // phew, found someone to fix me.
            my_mode = CYCLE_COLOURS_SEEN;
        }

    } 

}

uint8_t get_next_colour() {

    // read next valid id from EEPROM
    int i = last_eeprom_read;
    uint8_t count = 0;

    if ( i == 0 ) { i++; } // EEPROM 0x00 reserved for my_id
    uint8_t seen = 0;
    do {
        i++;
        count++;
        if (i > 240) {
            i = 1;
        }
        seen = EEPROM.read(i);
    } while ( ( count < 5 ) && (seen == 0xff) || (seen == 0) );

    last_eeprom_read = i;

    if ( seen != 0xff && seen != 0 ) {
        // new colour
        return last_eeprom_read;
    } else {
        return curr_colour;
    }

}

void factory_reset(void) {
    // blank all EEPROM (to 0xff)
    for (int i=0; i<1024; i++) { 
        FLASH_RED;
        delay_ten_us(100);
        FLASH_GREEN;
        delay_ten_us(100);
        FLASH_BLUE;
        delay_ten_us(100);
        EEPROM.write(i, 0xff);
        my_mode = INIT_MODE;
        enable_rgb_led = 1;
    }
}

void update_my_state(int counter) {

    if ( factory_reset_keycombo_count == 5 ) {
        // magic keycombo hit. Reset!
        factory_reset();
        factory_reset_keycombo_count = 0;
        return;
    }

    if ( my_mode == AM_INFECTED ) {
        // yikes. 
        //curr_colour = ( main_loop_counter % 2 ) ? 0 : 81;
        curr_colour = ( counter % 10 > 5 ) ? 0 : 81;
        if ( (main_loop_counter - time_infected) > MAX_TIME_INFECTED ) {
            // you die
            my_mode = AM_ZOMBIE;
        }

    } else if ( my_mode == AM_ZOMBIE ) {
        // hnnnngggg... brains...
        // TODO: Make this more ugly slow pulsing rather than flashing.
        curr_colour = ( counter % 20 > 10 ) ? 0 : 1;
        curr_colour = ( main_loop_counter % 2 ) ? 0 : 1;

    } else if ( my_mode == CYCLE_COLOURS_SEEN ) {
        curr_colour = get_next_colour();

    } else if ( my_mode == INIT_MODE ) {
        // need to work out badge id.
        //  - will be set in EEPROM address 0
        int eeprom_id = EEPROM.read(0) << 8 + EEPROM.read(1);
        if ( (eeprom_id > 0) && (eeprom_id < 500) ) {
            my_id = eeprom_id;
            EEPROM.write( calc_eeprom_address_from_id(my_id), 1); // ensure we're in our own colour db
            my_mode = CYCLE_COLOURS_SEEN;
        } else {
	          // display a bunch of colours, so we can select one
            curr_colour = (curr_colour + 7) % 240;
        }
    }

    // curr_colour is 1-240, 0=off
    if ( curr_colour == 0 ) {
        // converts to RGB 0x0 - & LEDs off.
        HSVtoRGB(&curr_r, &curr_g, &curr_b, 0, 0, 0);
    } else {
	//int v = (16*((counter % 15)+2) - 1); // 32-255 in blocks of 16
	int v = 255;
        HSVtoRGB(&curr_r, &curr_g, &curr_b, (curr_colour - 1), 255, v);
    }

}

void pre_loop_setup() {
    // initialise our globals
    for (uint8_t i=0; i<BADGE_BUFFER_SIZE; i++) {
        global_badge_buffer[i].badge_id = 0;
        global_badge_buffer[i].first_seen = 0;
    }
}

void setup() {

    Serial.begin(57600);
    long my_code = 0;

    pre_loop_setup();

}

void update_my_colour() {
  if ( my_colour_mode == DOUBLE_WHITE_TORCH ) {
    lights.set(PIN_LED_BOTH, 255, 255, 255);
  } else if ( my_colour_mode == DOUBLE_RED_TORCH ) {
    lights.set(PIN_LED_BOTH, 255, 0, 0);
  } else {
    // do other stuff?
    if ( main_loop_counter % 1000 == 0 ) {
      //lights.set(PIN_LED_BOTH, (main_loop_counter % 255), (255 - (main_loop_counter % 255) ), (main_loop_counter % 255));
      //lights.set(PIN_LED_BOTH, (main_loop_counter % 255), (255 - (main_loop_counter % 255) ), (main_loop_counter % 255));
      lights.set(PIN_LED_LEFT, l_red, l_green, l_blue);
    } else {
      // default - set 
      if ( main_loop_counter % 2 == 0 ) {
        lights.set(PIN_LED_LEFT, l_red, l_green, l_blue);
      } else {
        lights.set(PIN_LED_RIGHT, r_red, r_green, r_blue);
      }
    }
  }
}

bool last_button_state = LOW;

bool button_pressed() {
  // button is LOW when pressed, fix logic
  if ( digitalRead(2) == LOW ) {
    Serial.println("button pressed");
    return 1;
  } else {
    return 0;
  }
}

void update_colour_mode() {
  if ( button_pressed() == 1 and last_button_state != 1 ) {
    if ( my_colour_mode == DOUBLE_WHITE_TORCH ) {
      my_colour_mode = DOUBLE_RED_TORCH;
    } else if ( my_colour_mode == DOUBLE_RED_TORCH ) {
      my_colour_mode = GAME_STATE;
    } else {
      my_colour_mode = DOUBLE_WHITE_TORCH;
    }
    last_button_state = 1;
  } else {
    last_button_state = button_pressed();
  }
}

void loop() {

    update_colour_mode();
    
    update_my_colour();

    if ( main_loop_counter % 30 == 0 ) {
        // every n cycles, reset the keycombo-ometer, so
        // we don't accidentally enable it.
        factory_reset_keycombo_count = 0;
    }

    main_loop_counter++;

    main_loop_counter %= 10000; // keep it decimal

}

