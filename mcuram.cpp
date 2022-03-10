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
  // Allocate RAM in DRAM
#ifdef MMU_IRAM_HEAP
  buf = (uint8_t*)malloc(DMEMK * 1024);
#else
  buf = (uint8_t*)malloc(MEMK * 1024);
#endif
}

MCURAM::~MCURAM() {
  free(buf);
}

void MCURAM::init() {
#ifdef MMU_IRAM_HEAP
  Serial.print(F("eCPM: Using additional "));
  Serial.print(IMEMK);
  Serial.print(F("K from IRAM.\r\n"));
  {
    HeapSelectIram ephemeral;
    ibuf = (uint8_t*)malloc(IMEMK * 1024);
  }
#endif
}

void MCURAM::clear() {
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
  // Return one byte from the correct buffer
#ifdef MMU_IRAM_HEAP
  if (addr < DMEM)
    return buf[addr];
  else if (addr <= LASTBYTE)
    return ibuf[addr - DMEM];
  else
    return 0xFF;
#else
  return addr <= LASTBYTE ? buf[addr] : 0xFF;
#endif
}

void MCURAM::setByte(uint16_t addr, uint8_t data) {
  // Set one byte into the correct buffer
#ifdef MMU_IRAM_HEAP
  if (addr < DMEM)
    buf[addr] = data;
  else if (addr <= LASTBYTE)
    ibuf[addr - DMEM] = data;
#else
  if (addr <= LASTBYTE)
    buf[addr] = data;
#endif
}

uint16_t MCURAM::getWord(uint16_t addr) {
  // Return one word from the correct buffer
#ifdef MMU_IRAM_HEAP
  if (addr < DMEM - 1)
    return buf[addr] + buf[addr + 1] * 0x0100;
  else if (addr == DMEM)
    return buf[DMEM] + ibuf[0] * 0x0100;
  else if (addr < LASTBYTE) {
    uint16_t idx = addr - DMEM;
    return ibuf[idx] + ibuf[idx + 1] * 0x0100;
  }
  else
    return 0xFFFF;
#else
  return addr < LASTBYTE ? (buf[addr] + buf[addr + 1] * 0x0100) : 0xFFFF;
#endif
}

void MCURAM::setWord(uint16_t addr, uint16_t data) {
  // Set one word into the correct buffer
#ifdef MMU_IRAM_HEAP
  if (addr < DMEM - 1) {
    buf[addr]     = lowByte(data);
    buf[addr + 1] = highByte(data);

  }
  else if (addr == DMEM) {
    buf[DMEM]     = lowByte(data);
    ibuf[0]       = highByte(data);
  }
  else if (addr < LASTBYTE) {
    uint16_t idx = addr - DMEM;
    ibuf[addr - DMEM]     = lowByte(data);
    ibuf[addr - DMEM + 1] = highByte(data);
  }
#else
  if (addr < LASTBYTE) {
    buf[addr]     = lowByte(data);
    buf[addr + 1] = highByte(data);
  }
#endif
}

void MCURAM::read(uint16_t addr, uint8_t *data, uint16_t len) {
  for (uint16_t i = 0; i < len; i++)
    data[i] = getByte(addr++);
}

void MCURAM::write(uint16_t addr, uint8_t *data, uint16_t len) {
  for (uint16_t i = 0; i < len; i++)
    setByte(addr++, data[i]);
}

void MCURAM::hexdump(uint16_t start, uint16_t stop, char* comment) {
  char prt[16];
  char val[4];
  uint8_t data;
  // Adjust start and stop addresses
  start &= 0xFFF0;
  stop  |= 0x000F;
  // Start with a new line
  Serial.print(F("\r\n"));
  // Print the comment
  if (comment[0]) {
    Serial.print(F("; "));
    Serial.print(comment);
    Serial.print(F("\r\n"));
  }
  // All bytes
  for (uint16_t addr = start; addr <= stop;) {
    yield();
    // Use the buffer to display the address
    sprintf_P(prt, PSTR("%04X: "), addr);
    Serial.print(prt);
    // Iterate over bytes, 2 sets of 8 bytes
    for (uint8_t set = 0; set < 2; set++) {
      for (uint8_t byt = 0; byt < 8; byt++) {
        // Read data
        data = getByte(addr);
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
    Serial.print(F("|\r\n"));
  }
}
