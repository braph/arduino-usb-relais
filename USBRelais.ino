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

/*
 * Helper function for reading single bit of an integer.
 * Returns value of bit.
 */
static inline __attribute__((always_inline))
int hw_bitRead(unsigned int n, int bit) {
   return ((n >> bit) & 1U);
}

/*
 * Helper function for setting single bit of an integer.
 * Returns new integer.
 */
static inline __attribute__((always_inline))
unsigned int hw_bitSet(unsigned int n, int bit) {
   return (n | 1U << bit);
}

uint16_t enabled_pins = 0;
#define enabled_pins_max  (sizeof(enabled_pins) * 8 - 1)

static inline __attribute__((always_inline))
void enable_pin(int pin) {
   enabled_pins = hw_bitSet(enabled_pins, pin);
}

static inline __attribute__((always_inline))
int pin_enabled(int pin) {
   return (pin <= enabled_pins_max && hw_bitRead(enabled_pins, pin));
}

#define CMD_ON 'O'
#define CMD_OFF 'o'
#define CMD_LIST 'L'
#define WAIT_TIME 5000

unsigned long lastSleep;

void setup() {
   //turn off brown-out enable in software
   MCUCR = bit(BODS) | bit(BODSE);
   MCUCR = bit(BODS);

   enable_pin(1);
   enable_pin(2);
   enable_pin(3);
   enable_pin(4);
   enable_pin(13);

   for (int i = 0; i < 16; ++i)
      if (pin_enabled(i))
         pinMode(i, OUTPUT);

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
   if (millis () - lastSleep >= WAIT_TIME)
   {
      lastSleep = millis ();

      noInterrupts ();

      byte old_ADCSRA = ADCSRA;
      // disable ADC
      ADCSRA = 0;  
      // pin change interrupt (example for D0)
      PCMSK2 |= bit (PCINT16); // want pin 0
      PCIFR  |= bit (PCIF2);   // clear any outstanding interrupts
      PCICR  |= bit (PCIE2);   // enable pin change interrupts for D0 to D7

      set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
      power_adc_disable();
      power_spi_disable();
      power_timer0_disable();
      power_timer1_disable();
      power_timer2_disable();
      power_twi_disable();

      UCSR0B &= ~bit (RXEN0);  // disable receiver
      UCSR0B &= ~bit (TXEN0);  // disable transmitter

      sleep_enable();
      digitalWrite (13, LOW);
      interrupts ();
      sleep_cpu ();      
      digitalWrite (13, HIGH);
      sleep_disable();
      power_all_enable();

      ADCSRA = old_ADCSRA;
      PCICR  &= ~bit (PCIE2);   // disable pin change interrupts for D0 to D7
      UCSR0B |= bit (RXEN0);  // enable receiver
      UCSR0B |= bit (TXEN0);  // enable transmitter
   }  // end of time to sleep

   if (Serial.available() <= 0)
      return;

   //delay(1000);
   while (read_blocking() != '@');

   int c = read_blocking();

   if (c == CMD_ON || c == CMD_OFF) {
      if (isdigit(peek_blocking())) {
         int pin = Serial.parseInt();

         if (pin_enabled(pin)) {
            digitalWrite(pin, (c == CMD_ON));
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
            Serial.println(digitalRead(i));
         }
   }
   else if (c == '~') {
      Serial.print('~');
   }
   else if (c == '\n' || c == '\r') {
   }
   else {
      Serial.println("UNKNOWN CMD");
   }
}
