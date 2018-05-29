/*
 * Powermanager - arduino code for my "powermanager" device
 * Copyright (C) 2016 Benjamin Abendroth
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/sfr_defs.h>
#include <string.h>
#include <Arduino.h>

#include "EEPROM.h"
#include "bitutils.h"

#define CMD_ON 'O'
#define CMD_OFF 'o'
#define CMD_LIST 'L'
#define CMD_RESET 'R'
#define WAIT_TIME 5000

uint16_t enabled_pins = 0;
uint16_t pin_states = 0;
#define enabled_pins_max (sizeof(enabled_pins) * 8 - 1)

void set_pin_state(int pin, int state) {
   pin_states = (((EEPROM[0]) << 8 | EEPROM[1]));
   pin_states = hw_bitWrite(pin_states, pin, state);
   EEPROM[0] = pin_states >> 8;
   EEPROM[1] = pin_states & 0xFF;
   digitalWrite(pin, !state);
}

int get_pin_state(int pin) {
   pin_states = (((EEPROM[0]) << 8 | EEPROM[1]));
   return (pin <= enabled_pins_max && hw_bitRead(pin_states, pin));
}

void enable_pin(int pin) {
   pinMode(pin, OUTPUT);
   set_pin_state(pin, get_pin_state(pin));
   enabled_pins = hw_bitSet(enabled_pins, pin);
}

int pin_enabled(int pin) {
   return (pin <= enabled_pins_max && hw_bitRead(enabled_pins, pin));
}

void setup() {
   //turn off brown-out enable in software
   MCUCR = bit(BODS) | bit(BODSE);
   MCUCR = bit(BODS);

   for (int i = 2; i <= 9; ++i)
      enable_pin(i);

   Serial.begin(9600);
}

int read_blocking() {
   while (Serial.available() <= 0);
   return Serial.read();
}

int peek_blocking() {
   while (Serial.available() <= 0);
   return Serial.peek();
}

void loop() {
   while (read_blocking() != '@');

   int c = read_blocking();

   if (c == CMD_ON || c == CMD_OFF) {
      if (isdigit(peek_blocking())) {
         int pin = Serial.parseInt();

         if (pin_enabled(pin)) {
            set_pin_state(pin, (c == CMD_ON));
            Serial.println("OK");
         } else {
            Serial.println("UNKNOWN PIN");
         }
      }
      else {
         Serial.println("MISSING ARG");
      }
   }
   else if (c == CMD_LIST) {
      for (int i = 0; i <= enabled_pins_max; ++i)
         if (pin_enabled(i)) {
            Serial.print(i, DEC);
            Serial.print(':');
            Serial.println(get_pin_state(i));
         }
   }
   else if (c == CMD_RESET) {
      for (int i = 0; i < 16; ++i)
         if (pin_enabled(i))
            set_pin_state(i, 0);
   }
   else if (c == ' ' || c == '\n' || c == '\r')
      ;
   else if (c == '~')
      Serial.print('~');
   else
      Serial.println("UNKNOWN CMD");
}
