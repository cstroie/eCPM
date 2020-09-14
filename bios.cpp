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

BIOS::BIOS(I8080 *cpu, RAM *ram): cpu(cpu), ram(ram) {
}

BIOS::~BIOS() {
}

void BIOS::init() {
  uint16_t offset;
  // Patch in the BIOS jump vectors (17 functions)
  for (uint8_t i = 0; i < 17; i++) {
    offset = i * 3;
    // BIOS jump vectors
    ram->writeByte(BIOSCODE + offset, 0xC3);               // JP
    ram->writeWord(BIOSCODE + offset + 1, BIOSCODE + 0x0100 + offset);
    // BIOS routines
    ram->writeByte(BIOSCODE + 0x0100 + offset,     0xD3);  // OUT A
    ram->writeByte(BIOSCODE + 0x0100 + offset + 1, i);
    ram->writeByte(BIOSCODE + 0x0100 + offset + 2, 0xC9);  // RET
  }

  // Define the DPB
  //uint8_t bufDPB[15];
  //DPB* dpb = (DPB*)bufDPB;
  //ram->write(DPBADDR, bufDPB, 15);

  // DPB_t dpb = {.spt = 0x0040, .bsh = 0x05, .blm = 0x1F, .exm = 0x01, .dsm = 0x07FF, .drm = 0x03FF, .al0 = 0xFF, .al1 = 0x00, .cks = 0x0000, .off = 0x0002};
  dpb = {0x0040, 0x05, 0x1F, 0x01, 0x07FF, 0x03FF, 0xFF, 0x00, 0x0000, 0x0002};
  ram->write(DPBADDR, dpb.buf, 16);

  // Load CCP
  loadCCP();

#ifdef DEBUG
  ram->hexdump(BIOSCODE, offset);
  ram->hexdump(BIOSCODE + 0x0100, offset);
#endif
}


void BIOS::call(uint16_t code) {
  //cpu->trace();

  switch (code) {
    case 0x00:  // BOOT
      // Print signon message and go to CCP
      boot();
      break;

    case 0x01:  // WBOOT
      // Back to CCP
      wboot();
      break;

    case 0x02:  // CONST
      // Console status to register A
      consts();
      break;

    case 0x03:  // CONIN
      // Console character input to register A
      conin();
      break;

    case 0x04:  // CONOUT
      // Console device output character in C
      conout();
      break;

    case 0x05:  // LIST
      // List device output character in C
      list();
      break;

    case 0x06:  // PUNCH
      // Punch device output character in C
      punch();
      break;

    case 0x07:  // READER
      // Reader character input to A (0x1A = device not implemented)
      reader();
      break;

    case 0x08:  // HOME
      // Move disk to home position
      home();
      break;

    case 0x09:  // SELDSK
      // Select disk given by register C
      seldsk();
      break;

    case 0x0A:  // SETTRK
      // Set track address given by C
      settrk();
      break;

    case 0x0B:  // SETSEC
      // Set sector number given by C
      setsec();
      break;

    case 0x0C:  // SETDMA
      // Set dma address given by register BC
      setdma();
      break;

    case 0x0D:  // READ
      // Read next disk record (disk/trk/sec/dma set)
      read();
      break;

    case 0x0E:  // WRITE
      // Write next disk record (disk/trk/sec/dma set)
      write();
      break;

    case 0x0F:  // LISTST
      // Return list device status in A
      listst();
      break;

    case 0x10:  // SECTRAN
      // Translate sector BC using table at DE
      sectran();
      break;

    default:
#ifdef DEBUG
      // Show unimplemented BIOS calls only when debugging
      Serial.print("\r\nUnimplemented BIOS call: 0x");
      Serial.print(code, HEX);
      Serial.print("\r\n");
      cpu->trace();
#endif
      break;
  }
}


// Print signon message and go to CCP
void BIOS::boot() {
  signon();
  // Clear IOBYTE and TDRIVE
  ram->writeByte(IOBYTE, 0x00);  // 0x3D
  ram->writeByte(TDRIVE, 0x00);
  // Go to CP/M
  gocpm();
}

// Back to CCP
void BIOS::wboot() {
  // TODO print warm boot message
  Serial.print("\r\n64K CP/M v2.2\r\n");
  // Reload CCP
  loadCCP();
  // Go to CP/M
  gocpm();
}

// Console status to register A
uint8_t BIOS::consts() {
  result = Serial.available() ? 0xFF : 0x00;
  cpu->regA(result);
  return result;
}

// Console character input to register A
uint8_t BIOS::conin() {
  while (!Serial.available()) { }
  result = Serial.read();
  cpu->regA(result);
  return result;
}

// Console device output character in C
void BIOS::conout() {
  conout(cpu->regC());
}
void BIOS::conout(uint8_t c) {
  Serial.write((char)(c & 0x7F));
}

// List device output character in C
void BIOS::list() {
  list(cpu->regC());
}
void BIOS::list(uint8_t c) {
  Serial.write((char)(c & 0x7F));
}

// Punch device output character in C
void BIOS::punch() {
  punch(cpu->regC());
}
void BIOS::punch(uint8_t c) {
  Serial.write((char)(c & 0x7F));
}

// Reader character input to A (0x1A = device not implemented)
uint8_t BIOS::reader() {
  result = 0x1A;
  cpu->regA(result);
  return result;
}

// Move disk to home position
void BIOS::home() {
}

// Select disk given by register C
void BIOS::seldsk() {
  cpu->regHL(0x0000);
}

// Set track address given by C
void BIOS::settrk() {
}

// Set sector number given by C
void BIOS::setsec() {
}

// Set dma address given by register BC
void BIOS::setdma() {
  cpu->regHL(cpu->regBC());
  //dmaAddr = cpu->regBC();
}

// Read next disk record (disk/trk/sec/dma set)
uint8_t BIOS::read() {
  result = 0x00;
  cpu->regA(result);
  return result;
}

// Write next disk record (disk/trk/sec/dma set)
uint8_t BIOS::write() {
  result = 0x00;
  cpu->regA(result);
  return result;
}

// Return list device status in A
uint8_t BIOS::listst() {
  result = 0xFF;
  cpu->regA(result);
  return result;
}

// Translate sector BC using table at DE
void BIOS::sectran() {
  cpu->regHL(cpu->regBC());  // HL=BC=No translation (1:1)
}


void BIOS::signon() {
  Serial.print("\r\n64K CP/M v2.2 (eCPM 0.1)\r\n");
}

void BIOS::gocpm() {
  // Patch in a JP to WBOOT at location 0x0000
  ram->writeByte(0x0000, 0xC3);  // JP
  ram->writeWord(0x0001, WBOOT);
  //  Patch in a JP to the BDOS entry at location 0x0005
  ram->writeByte(ENTRY, 0xC3);   // JP
  ram->writeWord(ENTRY + 1, BDOSCODE + 0x06);
  // Last loged disk number
  cpu->regC(ram->readByte(TDRIVE));
  // Jump to CCP
  cpu->jump(CCPCODE);
}

void BIOS::loadCCP() {
  ram->write(CCPCODE, CCP_BIN, CCP_BIN_len);
}
