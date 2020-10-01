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

// BIOS CALLS
const char* BIOS_CALLS[] = {"BOOT", "WBOOT", "CONST", "CONIN", "CONOUT", "LIST",
                            "PUNCH", "READER", "HOME", "SELDSK", "SETTRK", "SETSEC",
                            "SETDMA", "READ", "WRITE", "LISTST", "SECTRN"
                           };

BIOS::BIOS(I8080 *cpu, RAM *ram, DRIVE *drv): cpu(cpu), ram(ram), drv(drv) {
}

BIOS::~BIOS() {
}

void BIOS::init() {
  uint16_t j;

  Serial.print("eCPM: Initializing BIOS...");
  // Patch in the BIOS jump vectors (17 functions)
  for (uint8_t i = 0; i < 17; i++) {
    // Compute an offset
    j = i * 3;
    // BIOS jump vectors
    ram->setByte(BIOSCODE + j,      0xC3);          // JP BIOSENTRY + j
    ram->setWord(BIOSCODE + j + 1,  BIOSENTRY + j);
    // BIOS routines
    ram->setByte(BIOSENTRY + j,     0xD3);          // OUT (i), A
    ram->setByte(BIOSENTRY + j + 1, i);
    ram->setByte(BIOSENTRY + j + 2, 0xC9);          // RET
  }

  // Define the DPH (use only one DPH, since BDOS is emulated)
  dph = {0x0000, 0x0000, 0x0000, 0x0000, DIRBUF, BIOSDPB, BIOSDATA, BIOSDATA + 0x0010};
  ram->write(BIOSDPH, dph.buf, 16);

  // Define the DPB
  // Disc size    DKS:  8MB = 0x00800000    => DSM = DKS / BLS - 1
  // Block size   BLS:  4KB = 0x01000

  //   BLS  BSH BLM   EXM       DRM
  //  1024    3   7     0   -    32*AL
  //  2048    4  15     1   0    64*AL
  //* 4096    5  31     3   1   128*AL
  //  8192    6  63     7   3   256*AL
  // 16384    7 127    15   7   512*AL

#ifdef BLS_2048
  dpb.spt = 0x0020; // Total number of sectors per track
  dpb.bsh = 0x04;   // Data allocation block shift factor
  dpb.blm = 0x0F;   // Data allocation block mask (2^[BSH-1])
  dpb.exm = 0x00;   // Extent mask
  dpb.dsm = 0x07FF; // Maximum data block number (in BLS units)
  dpb.drm = 0x01FF; // Total number of directory entries
  dpb.al0 = 0xFF;   // Allocation of reserved directory blocks (8 bits)
  dpb.al1 = 0x00;
  dpb.cks = 0x0000; // Size of the directory check vector (fixed media)
  dpb.off = 0x0002; // Number of reserved tracks at the beginning of the disk
#else
  dpb.spt = 0x0020; // Total number of sectors per track
  dpb.bsh = 0x05;   // Data allocation block shift factor
  dpb.blm = 0x1F;   // Data allocation block mask (2^[BSH-1])
  dpb.exm = 0x01;   // Extent mask
  dpb.dsm = 0x07FF; // Maximum data block number (in BLS units)
  dpb.drm = 0x03FF; // Total number of directory entries
  dpb.al0 = 0xFF;   // Allocation of reserved directory blocks (8 bits)
  dpb.al1 = 0x00;
  dpb.cks = 0x0000; // Size of the directory check vector (fixed media)
  dpb.off = 0x0002; // Number of reserved tracks at the beginning of the disk
#endif

  // Write the DPB into RAM
  // Because of the byte alignment on 32-bit CPU,
  // we need to store each field separately
  uint16_t addr = BIOSDPB;
  ram->setWord(addr++, dpb.spt); addr++;
  ram->setByte(addr++, dpb.bsh);
  ram->setByte(addr++, dpb.blm);
  ram->setByte(addr++, dpb.exm);
  ram->setWord(addr++, dpb.dsm); addr++;
  ram->setWord(addr++, dpb.drm); addr++;
  ram->setByte(addr++, dpb.al0);
  ram->setByte(addr++, dpb.al1);
  ram->setWord(addr++, dpb.cks); addr++;
  ram->setWord(addr++, dpb.off);
  ram->flush();
  Serial.print(" done.\r\n");

  // Load CCP
  drv->loadCCP(true);

#ifdef DEBUG
  ram->hexdump(BIOSCODE,  BIOSCODE  + j, "BIOS");
  ram->hexdump(BIOSENTRY, BIOSENTRY + j, "BIOS ENTRY");
#endif
}


void BIOS::call(uint16_t code) {
#ifdef DEBUG_BIOS_CALLS
  Serial.print("\r\n\t\tBIOS call 0x");
  Serial.print(code, HEX);
  if (code <= 17) {
    Serial.print("\t");
    Serial.print(BIOS_CALLS[code]);
  }
  Serial.print("\r\n");
  cpu->trace();
#endif

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
#ifdef DEBUG_BIOS_CALLS
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
  ram->setByte(IOBYTE, 0x94);
  ram->setByte(TDRIVE, 0x00);
  // Go to CP/M
  gocpm();
}

// Back to CCP
void BIOS::wboot() {
  // Reload CCP
  drv->loadCCP();
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
  while (consts() == 0x00) { }
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
  //Serial.write((char)(c & 0x7F));
  drv->wrLST((char)(c & 0x7F));
}

// Punch device output character in C
void BIOS::punch() {
  punch(cpu->regC());
}
void BIOS::punch(uint8_t c) {
  Serial.write((char)(c));
}

// Reader character input to A (0x1A = device not implemented)
uint8_t BIOS::reader() {
  while (consts() == 0x00) { }
  result = Serial.read();
  cpu->regA(result);
  return result;
}

// Move disk to home position
void BIOS::home() {
}

// Select disk given by register C
void BIOS::seldsk() {
  // A responsibility of the SELDSK subroutine
  // is to return the base address of the DPH for
  // the selected drive. The following sequence of
  // operations returns the table address, with a
  // 0000H returned if the selected drive does not exist.
  // SEE http://www.gaby.de/cpm/manuals/archive/cpm22htm/ch6.htm
  cpu->regHL(BIOSDPH);
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
  char buf[80] = "\r\n64K CP/M v2.2 (eCPM 0.1)\r\n";
  uint8_t i = 0;
  while (buf[i])
    conout(buf[i++]);
}

void BIOS::gocpm() {
  // Close the LST dev file
  drv->clLST();
  // Patch in a JP to WBOOT at location 0x0000
  ram->setByte(0x0000, 0xC3);   // JP WBOOT
  ram->setWord(0x0001, WBOOT);
  //  Patch in a JP to the BDOS entry at location 0x0005
  ram->setByte(ENTRY, 0xC3);    // JP BDOSCODE + 0x06
  ram->setWord(ENTRY + 1, BDOSCODE + 0x06);
  // Last loged disk number
  cpu->regC(ram->getByte(TDRIVE));
  // Jump to CCP
  cpu->jump(CCPCODE);
}

// Keep the device mappings
void BIOS::ioByte(uint8_t iobyte) {
  this->ioCON = iobyte & 0x03;
  this->ioRDR = iobyte & 0x0C >> 2;
  this->ioPUN = iobyte & 0x30 >> 4;
  this->ioLST = iobyte & 0xC0 >> 6;
}
