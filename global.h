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
#define LASTBYTE    (MEM - 1)
#define RAMSIZE     (MEM - 0x0800)

/*
  #define ROMMON      (MEM - 0x0800)
  #define BIOSDATA    (MEM - 0x0800 - 0x0380)
  #define BIOSCODE    (MEM - 0x0800 - 0x0380 - 0x0380)
  #define BIOSENTRY   (BIOSCODE + 0x0040)
  #define BIOSDPB     (BIOSDATA + 0x0080)
  #define BDOSCODE    (MEM - 0x0800 - 0x0380 - 0x0380 - 0x0E00 + 0x0100)
  #define BDOSENTRY   (BDOSCODE + 0x0010)
  #define CCPCODE     (MEM - 0x0800 - 0x0380 - 0x0380 - 0x0E00 - 0x0800)
*/

#define BIOSCODE    (MEM - 0x0200)        // 0xFE00
#define BIOSENTRY   (BIOSCODE + 0x0040)   // 0xFE40
#define BIOSDATA    (BIOSCODE + 0x0100)   // 0xFF00
#define BIOSDPH     (BIOSDATA + 0x0030)   // 0xFF80
#define BIOSDPB     (BIOSDATA + 0x0040)   // 0xFF80
#define BDOSCODE    (MEM - 0x0400)        // 0xFC00
#define DIRBUF      (BDOSCODE + 0x0100)   // 0xFD00
#define BDOSENTRY   (BDOSCODE + 0x0010)   // 0xFC10
#define CCPCODE     (MEM - 0x0C00)        // 0xF400

// Position of the $$$.SUB FCB on this CCP
#define BatchFCB    (CCPCODE + 0x07AC)



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
#define sizBK 128               // CP/M block size (bytes)
#define recEX 128               // Number of records in extent
#define sizEX (sizBK * recEX)   // Extent size (bytes)
#define blkS2 4096              // Number of blocks in S2
#define maxEX 31                // Maximum value the EX field can take
#define maxS2 15                // Maximum value the S2 field can take
#define maxCR 128               // Maximum value the CR field can take
#define maxRC 128               // Maximum value the RC field can take

// File name combined char string
#define FNDRIVE   0
#define FNUSER    1
#define FNFILE    2
#define FNTYPE    10
#define FNZERO    13
#define FNHOST    16

#define toHEX(x)    ((x) < 10 ? (x) + 48 : (x) + 87)
#define frHEX(x)    (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A') : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A') : 0)))

#endif /* GLOBAL_H */
