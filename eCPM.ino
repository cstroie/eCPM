/**
  Embedded CP/M Machine

  Copyright (C) 2020 Costin STROIE <costinstroie@eridu.eu.org>

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


#include <SPI.h>
#include <SD.h>

#if defined(ESP8266)
#include <Ticker.h>
#endif

// Configuration
#include "config.h"
// Global parameters
#include "global.h"

#ifdef SPI_RAM
#include "spiram.h"
#else
#include "mcuram.h"
#endif
#include "i8080.h"
#include "bios.h"
#include "bdos.h"

// Some declarations
uint8_t callBDOS(int port);
void    callBIOS(int port, int value);

#if defined(ESP8266)
// The ticker
Ticker oneSec;
#endif

#ifdef SPI_RAM
// SPI RAM
SPIRAM ram(RS, RAM_BUFFER_SIZE);
#else
// MCU RAM
MCURAM ram;
#endif

int  I8080::read_word(int addr) {
  return ram.getWord(addr);
}
void I8080::write_word(int addr, int word) {
  return ram.setWord(addr, word);
}

int  I8080::read_byte(int addr) {
  return ram.getByte(addr);
}
void I8080::write_byte(int addr, int byte) {
  ram.setByte(addr, byte);
}

int  I8080::io_input(int port) {
  return callBDOS(port);
}
void I8080::io_output(int port, int value) {
  callBIOS(port, value);
}

void I8080::iff(int on) {
  state = 0;
}


I8080 cpu;
DRIVE drv(&ram, "eCPM");
BIOS bios(&cpu, &ram, &drv);
BDOS bdos(&cpu, &ram, &drv, &bios);


void callBIOS(int port, int value) {
  bios.call(port);
}
uint8_t callBDOS(int port) {
  return bdos.call(port);
}

// Handler for one second ticker
void callTicker() {
  // LST file flush
  drv.fsLST();
}

/**
  Main Arduino setup function
*/
void setup() {
  // LED configuration
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW ^ LEDinv);
  // Serial port configuration
  Serial.begin(SERIAL_SPEED);
  Serial.print("\r\n\r\n");
  // SPI
  SPI.begin();
  // Init the DRIVE
  drv.init();
#ifdef SPI_RAM
  // Init the SPI RAM
  // FIXME This breaks the SPI
  //ram.init();
#endif
  // Init the BIOS
  bios.init();
  // Init the BDOS
  bdos.init();

#ifdef ESP8266
  // Attach the handler to the one second ticker
  oneSec.attach(1, callTicker);
#endif

  // RAM hex dump
  //ram.hexdump(0x0000, 0x0200);

  // Start BIOS
  cpu.jump(BIOSCODE);
}

/**
  Main Arduino loop
*/
void loop() {
  if (cpu.state) {
    cpu.instruction();
    //cpu.trace();
  }

  //delay(100);
}
