/**
  ram.cpp - SPI RAM management

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

#include "ram.h"

RAM::RAM(int CS, uint16_t bufSize) {
  cs = CS;
  pinMode(cs, OUTPUT);
  digitalWrite(cs, HIGH);
  delay(50);
  digitalWrite(cs, LOW);
  delay(50);
  digitalWrite(cs, HIGH);

  // Page buffer
  this->bufSize = bufSize;
  this->bufHalfSize = bufSize / 2;
  // Allocate one byte more, to make room for 16-bit operations
  buf = (uint8_t*)malloc(bufSize + 1);
}

RAM::~RAM() {
  free(buf);
}

void RAM::init() {
  begin();
  SPI.transfer(CMD_WRMR);
  SPI.transfer(MODE_SEQ);
  end();
}

void RAM::clear() {
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
#if defined(ESP8266)
    yield();
#endif
  }
  // End SPI transfer
  end();
}

void RAM::reset() {
  // Begin SPI transfer
  begin();
  // Command
  SPI.transfer(CMD_RSTIO);
  // End SPI transfer
  end();
}

// Check if the address is contained in buffer
bool RAM::inBuffer(uint16_t addr) {
  return (addr > bufStart and addr < bufEnd);
}

// Flush the buffer, if dirty, and reset it
void RAM::flush() {
  // Check if dirty
  if (bufDirty)
    // Write back the buffer into RAM
    write(bufStart, buf, bufSize + 1, false);
  // Make it clean
  bufDirty = false;
  // Reset the start and end addresses
  bufStart = LASTBYTE;
  bufEnd = LASTBYTE;
}

// Flush the buffer, if dirty and the address is contained, and reset it
void RAM::flush(uint16_t addr) {
  // Check if the address is contained in buffer
  if (inBuffer(addr))
    // Need to flush
    flush();
}

void RAM::bufChange(uint16_t addr) {
  // Check if the address is contained in buffer
  if (inBuffer(addr))
    return;
  // Check if dirty
  if (bufDirty)
    // Write back the buffer into RAM
    write(bufStart, buf, bufSize + 1, false);
  // Set new start address
  if      (addr < bufHalfSize)
    bufStart = 0x0000;
  else if (addr > (LASTBYTE - bufHalfSize))
    bufStart = LASTBYTE - bufHalfSize;
  else
    bufStart = addr - bufHalfSize;
  // End address
  bufEnd = bufStart + bufSize;
  // Fetch the buffer from RAM
  read(bufStart, buf, bufSize + 1, false);
  // Make it clean
  bufDirty = false;
}

uint8_t RAM::getByte(uint16_t addr) {
  // Check if the address is included in buffer
  if (not(inBuffer(addr)))
    // Change the buffer
    bufChange(addr);
  // Directly return the byte from the buffer
  return buf[addr - bufStart];
}

void RAM::setByte(uint16_t addr, uint8_t data) {
  // Check if the address is included in buffer
  if (not(inBuffer(addr)))
    // Change the buffer
    bufChange(addr);
  // Directly set the byte into the buffer
  buf[addr - bufStart] = data;
  bufDirty = true;
}

uint16_t RAM::getWord(uint16_t addr) {
  // Check if the address is included in buffer
  if (not(inBuffer(addr)))
    // Change the buffer
    bufChange(addr);
  // Directly return the byte from the buffer
  return buf[addr - bufStart] + buf[addr - bufStart + 1] * 0x0100;
}

void RAM::setWord(uint16_t addr, uint16_t data) {
  // Check if the address is included in buffer
  if (not(inBuffer(addr)))
    // Change the buffer
    bufChange(addr);
  // Directly set the byte into the buffer
  buf[addr - bufStart]     = lowByte(data);
  buf[addr - bufStart + 1] = highByte(data);
  bufDirty = true;
}


uint8_t RAM::readByte(uint16_t addr) {
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

void RAM::writeByte(uint16_t addr, uint8_t data) {
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

uint16_t RAM::readWord(uint16_t addr) {
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

void RAM::writeWord(uint16_t addr, uint16_t data) {
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

void RAM::read(uint16_t addr, uint8_t *buf, uint16_t len, bool doFlush) {
  uint16_t i = 0;
  // Check first if we need to flush the buffers
  if (doFlush)
    flush(addr);
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

void RAM::write(uint16_t addr, uint8_t *buf, uint16_t len, bool doFlush) {
  uint16_t i = 0;
  // Check first if we need to flush the buffers
  if (doFlush)
    flush(addr);
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


void RAM::hexdump(uint16_t start, uint16_t stop, char* comment) {
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
#if defined(ESP8266)
    // ESP 8266
    yield();
#endif
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


void RAM::begin() {
  SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0));
  digitalWrite(cs, LOW);
}

void RAM::end() {
  digitalWrite(cs, HIGH);
  SPI.endTransaction();
}
