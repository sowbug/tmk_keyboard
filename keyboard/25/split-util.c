#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "split-util.h"
#include "matrix.h"
#include "i2c.h"
#include "serial.h"
#include "keyboard.h"
#include "config.h"

volatile bool isLeftHand = true;

static void setup_handedness(void) {
    isLeftHand = eeprom_read_byte(EECONFIG_HANDEDNESS);
}

static void keyboard_master_setup(void) {
#ifdef USE_I2C
    i2c_master_init();
#else
    serial_master_init();
#endif
}

static void keyboard_slave_setup(void) {
#ifdef USE_I2C
    i2c_slave_init(SLAVE_I2C_ADDRESS);
#else
    serial_slave_init();
#endif
}

bool has_usb(void) {
   USBCON |= (1 << OTGPADE); //enables VBUS pad
   _delay_us(5);
   return (USBSTA & (1<<VBUS));  //checks state of VBUS
}

void split_keyboard_setup(void) {
   setup_handedness();

   if (has_usb()) {
      keyboard_master_setup();
   } else {
      keyboard_slave_setup();
   }
   sei();
}

void keyboard_slave_loop(void) {
   matrix_init();

   while (1) {
      matrix_slave_scan();
   }
}

// this code runs before the usb and keyboard is initialized
void matrix_setup(void) {
    split_keyboard_setup();

    if (!has_usb()) {
        keyboard_slave_loop();
    }
}
