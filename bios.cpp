/**
  bios.cpp - BIOS emulator

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

#include "bios.h"

BIOS::BIOS(I8080 cpu, RAM ram) {
  this->cpu = cpu;
  this->ram = ram;
}

BIOS::~BIOS() {
}

void BIOS::init() {
  uint16_t offset;
  // Patch in the BIOS jump vectors (17 functions)
  for (uint8_t i = 0; i < 17; i++) {
    offset = i * 3;
    // BIOS jump vectors
    ram.writeByte(BIOSCODE + offset, JP);
    ram.writeWord(BIOSCODE + offset + 1, BIOSCODE + 0x0100 + offset);
    // BIOS routines
    ram.writeByte(BIOSCODE + 0x0100 + offset, OUTa);
    ram.writeByte(BIOSCODE + 0x0100 + offset + 1, i);
    ram.writeByte(BIOSCODE + 0x0100 + offset + 2, RET);
  }

  // Patch in a JP to WBOOT at location 0x0000
  ram.writeByte(0x0000, JP);
  ram.writeWord(0x0001, WBOOT);

#ifdef DEBUG
  ram.hexdump(BIOSCODE, offset);
  ram.hexdump(BIOSCODE + 0x0100, offset);
#endif
}

void BIOS::call(uint16_t port) {
  switch (port) {

    case 0x00:
      // BOOT: Ends CPM
      cpu.state = 0;
      break;

    case 0x01:
      // WBOOT: Back to CCP
      break;

    case 0x02:
      // CONST: Console status
      cpu.regA(Serial.available() ? 0xff : 0x00);
      break;

    case 0x03:
      // CONIN: Console input
      while (!Serial.available()) { }
      cpu.regA(Serial.read());
      break;

    case 0x04:
      // CONOUT Console output
      Serial.write((char)(cpu.regC() & 0x7F));
      break;

    case 0x05:
      // LIST List output
      break;

    case 0x06:
      // PUNCH/AUXOUT Punch output
      break;

    case 0x07:
      // READER Reader input (0x1A = device not implemented)
      cpu.regA(0x1A);
      break;

    case 0x08:
      // HOME Home disk head
      break;

    case 0x09:
      // SELDSK Select disk drive
      cpu.regHL(0x0000);
      break;

    case 0x0A:
      // SETTRK Set track number
      break;

    case 0x0B:
      // SETSEC Set sector number
      break;

    case 0x0C:
      // SETDMA Set DMA address
      cpu.regHL(cpu.regBC());
      //dmaAddr = cpu.regBC();
      break;

    case 0x0D:
      // READ Read selected sector
      cpu.regA(0x00);
      break;

    case 0x0E:
      // WRITE Write selected sector
      cpu.regA(0x00);
      break;

    case 0x0F:
      // LISTST Get list device status
      cpu.regA(0xFF);
      break;

    case 0x10:
      // SECTRAN Sector translate
      cpu.regHL(cpu.regBC());  // HL=BC=No translation (1:1)
      break;

    default:
#ifdef DEBUG
      // Show unimplemented BIOS calls only when debugging
      Serial.print("\r\nUnimplemented BIOS call: 0x");
      Serial.print(port, 16);
      Serial.print("\r\n");
      cpu.trace();
#endif
      break;
  }
}
