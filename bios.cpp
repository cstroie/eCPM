/**
  bios.cpp - CP/M 2.2 compatible Basic Input / Output System (BIOS)

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
#include "ccp.h"

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
    ram.writeByte(BIOSCODE + offset, 0xC3);               // JP
    ram.writeWord(BIOSCODE + offset + 1, BIOSCODE + 0x0100 + offset);
    // BIOS routines
    ram.writeByte(BIOSCODE + 0x0100 + offset,     0xD3);  // OUT A
    ram.writeByte(BIOSCODE + 0x0100 + offset + 1, i);
    ram.writeByte(BIOSCODE + 0x0100 + offset + 2, 0xC9);  // RET
  }

  // Define the DPB
  //uint8_t bufDPB[15];
  //DPB* dpb = (DPB*)bufDPB;
  //ram.write(DPBADDR, bufDPB, 15);

  // DPB_t dpb = {.spt = 0x0040, .bsh = 0x05, .blm = 0x1F, .exm = 0x01, .dsm = 0x07FF, .drm = 0x03FF, .al0 = 0xFF, .al1 = 0x00, .cks = 0x0000, .off = 0x0002};
  DPB_t dpb = {0x0040, 0x05, 0x1F, 0x01, 0x07FF, 0x03FF, 0xFF, 0x00, 0x0000, 0x0002};
  ram.write(DPBADDR, dpb.buf, 16);


  // Patch in a JP to WBOOT at location 0x0000
  //ram.writeByte(0x0000, JP);
  //ram.writeWord(0x0001, WBOOT);

  // Load CCP
  loadCCP();

#ifdef DEBUG
  ram.hexdump(BIOSCODE, offset);
  ram.hexdump(BIOSCODE + 0x0100, offset);
#endif
}

void BIOS::signon() {
  Serial.print("\r\n64K CP/M v2.2 (eCPM 0.1)\r\n");
}

void BIOS::gocpm() {
  // Patch in a JP to WBOOT at location 0x0000
  ram.writeByte(0x0000, 0xC3);  // JP
  ram.writeWord(0x0001, WBOOT);
  //  Patch in a JP to the BDOS entry at location 0x0005
  ram.writeByte(ENTRY, 0xC3);   // JP
  ram.writeWord(ENTRY + 1, BDOSCODE + 0x06);
  // Last loged disk number
  cpu.regC(ram.readByte(TDRIVE));
  // Jump to CCP
  cpu.jump(CCPCODE);
}

void BIOS::loadCCP() {
  ram.write(CCPCODE, CCP_BIN, CCP_BIN_len);
}

void BIOS::call(uint16_t code) {
  switch (code) {
    case 0x00:  // BOOT
      // Print signon message and go to CCP
      signon();
      // Clear IOBYTE and TDRIVE
      ram.writeByte(IOBYTE, 0x00);  // 0x3D
      ram.writeByte(TDRIVE, 0x00);
      // Go to CP/M
      gocpm();
      break;

    case 0x01:  // WBOOT
      // Back to CCP
      // TODO print warm boot message
      loadCCP();
      // Go to CP/M
      gocpm();
      break;

    case 0x02:  // CONST
      // Console status to register A
      cpu.regA(Serial.available() ? 0xff : 0x00);
      break;

    case 0x03:  // CONIN
      // Console character input to register A
      while (!Serial.available()) { }
      cpu.regA(Serial.read());
      break;

    case 0x04:  // CONOUT
      // Console device output character in C
      Serial.write((char)(cpu.regC() & 0x7F));
      break;

    case 0x05:  // LIST
      // List device output character in C
      break;

    case 0x06:  // PUNCH
      // Punch device output character in C
      break;

    case 0x07:  // READER
      // Reader character input to A (0x1A = device not implemented)
      cpu.regA(0x1A);
      break;

    case 0x08:  // HOME
      // Move disk to home position
      break;

    case 0x09:  // SELDSK
      // Select disk given by register C
      cpu.regHL(0x0000);
      break;

    case 0x0A:  // SETTRK
      // Set track address given by C
      break;

    case 0x0B:  // SETSEC
      // Set sector number given by C
      break;

    case 0x0C:  // SETDMA
      // Set dma address given by register BC
      cpu.regHL(cpu.regBC());
      //dmaAddr = cpu.regBC();
      break;

    case 0x0D:  // READ
      // Read next disk record (disk/trk/sec/dma set)
      cpu.regA(0x00);
      break;

    case 0x0E:  // WRITE
      // Write next disk record (disk/trk/sec/dma set)
      cpu.regA(0x00);
      break;

    case 0x0F:  // LISTST
      // Return list device status in A
      cpu.regA(0xFF);
      break;

    case 0x10:  // SECTRAN
      // Translate sector BC using table at DE
      cpu.regHL(cpu.regBC());  // HL=BC=No translation (1:1)
      break;

    default:
#ifdef DEBUG
      // Show unimplemented BIOS calls only when debugging
      Serial.print("\r\nUnimplemented BIOS call: 0x");
      Serial.print(code, 16);
      Serial.print("\r\n");
      cpu.trace();
#endif
      break;
  }
}
