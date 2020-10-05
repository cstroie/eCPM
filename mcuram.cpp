/**
  mcuram.cpp - MCU RAM management

  Copyright (C) 2018 Costin STROIE <costinstroie@eridu.eu.org>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mcuram.h"

MCURAM::MCURAM() {
  // Allocate one more byte (to make room for 16-bit operations)
  //buf = (uint8_t*)malloc(MEM);
}

MCURAM::~MCURAM() {
  //free(buf);
}

void MCURAM::init() {
}

void MCURAM::clear() {
#if defined(ESP8266)
  yield();
#endif
}

void MCURAM::reset() {
}

// Flush the buffer, if dirty, and reset it
void MCURAM::flush() {
}

// Flush the buffer, if dirty and the address is contained, and reset it
void MCURAM::flush(uint16_t addr) {
}

uint8_t MCURAM::getByte(uint16_t addr) {
  // Directly return the byte from the buffer
  return buf[addr];
}

void MCURAM::setByte(uint16_t addr, uint8_t data) {
  // Directly set the byte into the buffer
  buf[addr] = data;
}

uint16_t MCURAM::getWord(uint16_t addr) {
  // Directly return the byte from the buffer
  return buf[addr] + buf[addr + 1] * 0x0100;
}

void MCURAM::setWord(uint16_t addr, uint16_t data) {
  // Directly set the byte into the buffer
  buf[addr]     = lowByte(data);
  buf[addr + 1] = highByte(data);
}

void MCURAM::read(uint16_t addr, uint8_t *data, uint16_t len) {
  memcpy(data, &buf[addr], len);
}

void MCURAM::write(uint16_t addr, uint8_t *data, uint16_t len) {
  memcpy(&buf[addr], data, len);
}

void MCURAM::hexdump(uint16_t start, uint16_t stop, char* comment) {
  char prt[16];
  char val[4];
  uint8_t data;
  // Adjust start and stop addresses
  start &= 0xFFF0;
  stop  |= 0x000F;
  // Start with a new line
  Serial.print("\r\n");
  // Print the comment
  if (comment[0]) {
    Serial.print("; ");
    Serial.print(comment);
    Serial.print("\r\n");
  }
  // All bytes
  for (uint16_t addr = start; addr <= stop;) {
#if defined(ESP8266)
    // ESP 8266
    yield();
#endif
    // Use the buffer to display the address
    sprintf_P(prt, PSTR("%04X: "), addr);
    Serial.print(prt);
    // Iterate over bytes, 2 sets of 8 bytes
    for (uint8_t set = 0; set < 2; set++) {
      for (uint8_t byt = 0; byt < 8; byt++) {
        // Read data
        data = buf[addr];
        // Prepare and print the hex dump
        sprintf_P(val, PSTR("%02X "), data);
        Serial.print(val);
        // Prepare the ASCII dump
        if (data < 0x20 or data > 0x7F)
          data = '.';
        prt[addr & 0x0F] = data;
        // Increment the address
        addr++;
      }
      // Print a separator
      Serial.write(' ');
    }
    // Print the ASCII column
    Serial.write('|');
    for (uint8_t idx = 0; idx < 0x10; idx++)
      Serial.write(prt[idx]);
    // New line
    Serial.print("|\r\n");
  }
}
