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

/*
 * Helper function for writing single bit of an integer.
 * Returns new integer.
 */
static inline __attribute__((always_inline))
unsigned int hw_bitWrite(unsigned int n, int bit, int value) {
   if (value)
      return (n | 1U << bit);
   else
      return (n & ~(1U << bit));
}
