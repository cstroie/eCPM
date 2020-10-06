/**
  spiram.cpp - SPI RAM management

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

#include "spiram.h"

SPIRAM::SPIRAM(int CS, uint16_t bufSize): cs(CS), bufSize(bufSize) {
  // Initialize the RAM chip
  pinMode(cs, OUTPUT);
  digitalWrite(cs, HIGH);
  delay(50);
  digitalWrite(cs, LOW);
  delay(50);
  digitalWrite(cs, HIGH);

  // Allocate one more byte (to make room for 16-bit operations)
  buf = (uint8_t*)malloc(bufSize + 1);
}

SPIRAM::~SPIRAM() {
  free(buf);
}

void SPIRAM::init() {
  begin();
  SPI.transfer(CMD_WRMR);
  SPI.transfer(MODE_SEQ);
  end();
}

void SPIRAM::clear() {
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_WRITE);
  // Address
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  // Data
  for (int x = 0x00; x <= 0xFF; ++x) {
    for (int y = 0x00; y <= 0xFF; ++y)
      SPI.transfer(0x00);
    yield();
  }
  // End SPI transfer
  end();
}

void SPIRAM::reset() {
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_RSTIO);
  // End SPI transfer
  end();
}

// Check if the address is contained in buffer
bool SPIRAM::inBuffer(uint16_t addr) {
  return (addr >= bufStart and addr <= bufEnd);
}

// Flush the buffer, if dirty, and reset it
void SPIRAM::flush() {
  // Write back the buffer into RAM
  wrBuffer();
  // Reset the start and end addresses
  bufStart = LASTBYTE;
  bufEnd = LASTBYTE;
}

// Flush the buffer, if dirty and the address is contained, and reset it
void SPIRAM::flush(uint16_t addr) {
  // Check if the address is contained in buffer
  if (inBuffer(addr))
    // Need to flush
    flush();
}

void SPIRAM::chBuffer(uint16_t addr) {
  // Check if the address is contained in buffer
  if (inBuffer(addr))
    return;
  // Write back the buffer into RAM
  wrBuffer();
  // Set new start address
  if (addr > (LASTBYTE - bufSize))
    bufStart = LASTBYTE - bufSize + 1;
  else
    bufStart = addr;
  // End address
  bufEnd = bufStart + bufSize - 1;
  // Fetch the buffer from RAM
  rdBuffer();
}

// Read a buffer from RAM and mark it clean
void SPIRAM::rdBuffer() {
  // Read RAM data into buffer
  read(bufStart, buf, bufSize + 1);
  // Make it clean
  bufDirty = false;
}

// Write a buffer to RAM, if dirty, and mark it clean
void SPIRAM::wrBuffer() {
  if (bufDirty) {
    // Write buffer data into RAM
    write(bufStart, buf, bufSize + 1);
    // Make it clean
    bufDirty = false;
  }
}

uint8_t SPIRAM::getByte(uint16_t addr) {
  // Change the buffer
  chBuffer(addr);
  // Directly return the byte from the buffer
  return buf[addr - bufStart];
}

void SPIRAM::setByte(uint16_t addr, uint8_t data) {
  // Change the buffer
  chBuffer(addr);
  // Directly set the byte into the buffer
  buf[addr - bufStart] = data;
  // Mark it dirty
  bufDirty = true;
}

uint16_t SPIRAM::getWord(uint16_t addr) {
  // Change the buffer
  chBuffer(addr);
  // Directly return the byte from the buffer
  return buf[addr - bufStart] +
         buf[addr - bufStart + 1] * 0x0100;
}

void SPIRAM::setWord(uint16_t addr, uint16_t data) {
  // Change the buffer
  chBuffer(addr);
  // Directly set the byte into the buffer
  buf[addr - bufStart]     = lowByte(data);
  buf[addr - bufStart + 1] = highByte(data);
  bufDirty = true;
}


uint8_t SPIRAM::readByte(uint16_t addr) {
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_READ);
  // Address
  SPI.transfer(0x00);
  SPI.transfer(highByte(addr));
  SPI.transfer(lowByte(addr));
  // Data
  uint8_t result = SPI.transfer(0x00);
  // End SPI transfer
  end();
  return result;
}

void SPIRAM::writeByte(uint16_t addr, uint8_t data) {
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_WRITE);
  // Address
  SPI.transfer(0x00);
  SPI.transfer(highByte(addr));
  SPI.transfer(lowByte(addr));
  // Data
  SPI.transfer(data);
  // End SPI transfer
  end();
}

uint16_t SPIRAM::readWord(uint16_t addr) {
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_READ);
  // Address
  SPI.transfer(0x00);
  SPI.transfer(highByte(addr));
  SPI.transfer(lowByte(addr));
  // Data
  uint16_t result = SPI.transfer(0x00) | (SPI.transfer(0x00) << 8);
  // End SPI transfer
  end();
  return result;
}

void SPIRAM::writeWord(uint16_t addr, uint16_t data) {
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_WRITE);
  // Address
  SPI.transfer(0x00);
  SPI.transfer(highByte(addr));
  SPI.transfer(lowByte(addr));
  // Data
  SPI.transfer(lowByte(data));
  SPI.transfer(highByte(data));
  // End SPI transfer
  end();
}

void SPIRAM::read(uint16_t addr, uint8_t *buf, uint16_t len) {
  uint16_t i = 0;
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_READ);
  // Address
  SPI.transfer(0x00);
  SPI.transfer(highByte(addr));
  SPI.transfer(lowByte(addr));
  // Data
  while (len--)
    buf[i++] = SPI.transfer(0x00);
  // End SPI transfer
  end();
}

void SPIRAM::write(uint16_t addr, uint8_t *buf, uint16_t len) {
  uint16_t i = 0;
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_WRITE);
  // Address
  SPI.transfer(0x00);
  SPI.transfer(highByte(addr));
  SPI.transfer(lowByte(addr));
  // Data
  while (len--)
    SPI.transfer(buf[i++]);
  // End SPI transfer
  end();
}


void SPIRAM::hexdump(uint16_t start, uint16_t stop, char* comment) {
  char buf[16];
  char val[4];
  uint8_t data;
  // Adjust start and stop addresses
  start &= 0xFFF0;
  stop  |= 0x000F;
  // Flush the buffer
  flush();
  // Start with a new line
  Serial.print("\r\n");
  // Print the comment
  if (comment[0]) {
    Serial.print("; ");
    Serial.print(comment);
    Serial.print("\r\n");
  }
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_READ);
  // Address
  SPI.transfer(0x00);
  SPI.transfer(highByte(start));
  SPI.transfer(lowByte(start));
  // All bytes
  for (uint16_t addr = start; addr <= stop;) {
    yield();
    // Use the buffer to display the address
    sprintf_P(buf, PSTR("%04X: "), addr);
    Serial.print(buf);
    // Iterate over bytes, 2 sets of 8 bytes
    for (uint8_t set = 0; set < 2; set++) {
      for (uint8_t byt = 0; byt < 8; byt++) {
        // Read data
        data = SPI.transfer(0x00);
        // Prepare and print the hex dump
        sprintf_P(val, PSTR("%02X "), data);
        Serial.print(val);
        // Prepare the ASCII dump
        if (data < 0x20 or data > 0x7F)
          data = '.';
        buf[addr & 0x0F] = data;
        // Increment the address
        addr++;
      }
      // Print a separator
      Serial.write(' ');
    }
    // Print the ASCII column
    Serial.write('|');
    for (uint8_t idx = 0; idx < 0x10; idx++)
      Serial.write(buf[idx]);
    // New line
    Serial.print("|\r\n");
  }
  // End SPI transfer
  end();
}


void SPIRAM::begin() {
  SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
}

void SPIRAM::end() {
  digitalWrite(cs, HIGH);
  SPI.endTransaction();
}
