#ifndef PINS_H
#define PINS_H

enum Pins { PIN_LED_BOTH = -1,
            PIN_LED_RIGHT = 7,
            PIN_LED_LEFT = 4,
            PIN_LED_BLUE = 5,
            PIN_LED_GREEN = 6,
            PIN_LED_RED = 10,
            PIN_IR_IN = 11,
            PIN_IR_OUT = 3,
            PIN_BUTTON = 2
          };

enum Interrrupts {
            INT_BUTTON = 1,
            INT_RF = 2
};

#endif // PINS_H
