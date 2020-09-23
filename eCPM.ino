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
//#include <SdFat.h>

#include <SD.h>

// Need to specify the namespace for SdFat
//using namespace sdfat;

// Global parameters
#include "global.h"
// Configuration
#include "config.h"

#include "ram.h"
#include "i8080.h"
#include "bios.h"
#include "bdos.h"

// Some declarations
void callBIOS(int port, int value);
void callBDOS(int port);


// Persistent configuration
RAM ram(D0, RAM_CACHE_SIZE);

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
  callBDOS(port);
  return regA();
}
void I8080::io_output(int port, int value) {
  callBIOS(port, value);
}

void I8080::iff(int on) {
  state = 0;
}


I8080 cpu;
DRIVE drv(&ram);
BIOS bios(&cpu, &ram);
BDOS bdos(&cpu, &ram, &drv, &bios);


void callBIOS(int port, int value) {
  bios.call(port);
}

void callBDOS(int port) {
  bdos.call(port);
}

/**
  Main Arduino setup function
*/
void setup() {
  // LED configuration
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW ^ LEDinv);
  // Serial port configuration
  Serial.begin(SERIAL_SPEED);
  // SPI
  SPI.begin();
  // SD card
  Serial.print("\r\nInitializing SD card... ");
  if (!SD.begin(SS, SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0))) {
    Serial.println(" failed!");
    while (1);
  }
  Serial.println(" done.");


  // Init the SPI RAM
  // FIXME This breaks the SPI
  //ram.init();

  bios.init();
  bdos.init();

  // Load some data
  //ram.write(0x0100, TST8080_COM, TST8080_COM_len);
  //ram.write(0x0100, DEMO_COM, DEMO_COM_len);
  //ram.write(0x0100, __8080EXM_COM, __8080EXM_COM_len);

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
