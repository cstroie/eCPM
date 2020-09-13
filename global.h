/**
  global.h - Global parameters

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

#ifndef GLOBAL_H
#define GLOBAL_H

#define MEM         (64 * 1024)
#define RAMSIZE     (MEM - 0x0800)

#define ROMMON      (MEM - 0x0800)
#define BIOSDATA    (MEM - 0x0800 - 0x0380)
#define BIOSCODE    (MEM - 0x0800 - 0x0380 - 0x0380)
#define BDOSCODE    (MEM - 0x0800 - 0x0380 - 0x0380 - 0x0E00)
#define CCPCODE     (MEM - 0x0800 - 0x0380 - 0x0380 - 0x0E00 - 0x0800)

#define DPBADDR     (BIOSDATA + 0x40)

#define TBASE       (0x0100)        // TPA - Transient Program Area
#define CBASE       (CCPCODE)       // Used for sanity checks
#define FBASE       (BDOSCODE + 6)  // Used for sanity checks

// Areas in zero page
#define IOBYTE      (0x03)   // i/o definition byte.
#define TDRIVE      (0x04)   // current drive name and user number.
#define ENTRY       (0x05)   // entry point for the cp/m bdos.
#define TBUFF       (0x80)   // i/o buffer and command line storage.

// BIOS jump vectors
#define BOOT        (BIOSCODE + 0x00)
#define WBOOT       (BIOSCODE + 0x03)
#define CONST       (BIOSCODE + 0x06)
#define CONIN       (BIOSCODE + 0x09)
#define CONOUT      (BIOSCODE + 0x0C)
#define LIST        (BIOSCODE + 0x0F)
#define PUNCH       (BIOSCODE + 0x12)
#define READER      (BIOSCODE + 0x15)
#define HOME        (BIOSCODE + 0x18)
#define SELDSK      (BIOSCODE + 0x1B)
#define SETTRK      (BIOSCODE + 0x1E)
#define SETSEC      (BIOSCODE + 0x21)
#define SETDMA      (BIOSCODE + 0x24)
#define READ        (BIOSCODE + 0x27)
#define WRITE       (BIOSCODE + 0x2A)
#define LISTST      (BIOSCODE + 0x2D)
#define SECTRN      (BIOSCODE + 0x30)

/* CP/M disk definitions */
#define BlkSZ 128  // CP/M block size
#define BlkEX 128 // Number of blocks on an extension
#define ExtSZ (BlkSZ * BlkEX)
#define BlkS2 4096  // Number of blocks on a S2 (module)
#define MaxEX 31  // Maximum value the EX field can take
#define MaxS2 15  // Maximum value the S2 (modules) field can take - Can be set to 63 to emulate CP/M Plus
#define MaxCR 128 // Maximum value the CR field can take
#define MaxRC 128 // Maximum value the RC field can take

#endif /* GLOBAL_H */
