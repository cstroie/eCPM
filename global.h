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

#include "config.h"

/* Software name and version */
#define PROGNAME    "eCPM"
#define PROGVERS    "0.3.1"

/* Memory size */
#ifdef SPI_RAM
#  define MEMK          (64)
#else
#  ifdef MMU_IRAM_HEAP
#    define DMEMK       (48)            // DRAM
#    define IMEMK       (16)            // IRAM
#    define MEMK        (DMEMK + IMEMK)
#    define DMEM        (DMEMK * 1024)
#    define IMEM        (IMEMK * 1024)
#  else
#    define MEMK        (48)
#  endif
#endif
#define MEM             (MEMK * 1024)
#define LASTBYTE        (MEM - 1)

// CCP file name
#define CCP_FILE        (PSTR("CCP-DR%02d.BIN"))

// Memory definitions                           64K    48K
#define BIOSCODE    (MEM - 0x0200)        // 0xFE00 0xBE00
#define BIOSENTRY   (BIOSCODE + 0x0040)   // 0xFE40 0xBE40
#define BIOSDATA    (BIOSCODE + 0x0100)   // 0xFF00 0xBF00
#define BIOSDPH     (BIOSDATA + 0x0030)   // 0xFF30 0xBF30
#define BIOSDPB     (BIOSDATA + 0x0040)   // 0xFF40 0xBF40
#define BDOSCODE    (MEM - 0x0400)        // 0xFC00 0xBC00
#define BDOSENTRY   (BDOSCODE + 0x0010)   // 0xFC10 0xBC10
#define DIRBUF      (BDOSCODE + 0x0100)   // 0xFD00 0xBD00
#define CCPCODE     (MEM - 0x0C00)        // 0xF400 0xB400

// Position of the $$$.SUB FCB on this CCP
#define BATCHFCB    (CCPCODE + 0x07AC)



#define TBASE       (0x0100)        // TPA - Transient Program Area
#define CBASE       (CCPCODE)       // Used for sanity checks
#define FBASE       (BDOSCODE + 6)  // Used for sanity checks

// Areas in page zero
#define IOBYTE      (0x03)   // I/O definition byte
#define TDRIVE      (0x04)   // Current drive name and user number
#define ENTRY       (0x05)   // Entry point for the CP/Mm BDOS
#define TBUFF       (0x80)   // I/O buffer and command line storage

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

/* CP/M FCB definitions */
#define mskCR (0x7F)                // CR mask
#define mskEX (0x1F)                // EX mask
#define mskS2 (0x0F)                // S2 mask
#define recEX (0x80)                // Number of records in extent
#define recS2 ((mskEX + 1) * recEX) // Number of records in S2
#define sizBK (0x80)                // CP/M block size (bytes)
#define sizEX (sizBK * recEX)       // Extent size (bytes)
#define maxRC 128                   // Maximum value the RC field can take

/* File name combined char string */
#define FNDRIVE   0
#define FNUSER    1
#define FNFILE    2
#define FNTYPE    10
#define FNZERO    13
#define FNHOST    16

/* HEX conversion macros */
#define toHEX(x)    ((x) < 10 ? (x) + 48 : (x) + 87)
#define frHEX(x)    (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A') : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A') : 0)))

/* LED macro fixing */
#if   defined(BUILTIN_LED)
#define LED         BUILTIN_LED
#elif defined(LED_BUILTIN)
#define LED         LED_BUILTIN
#else
#define LED         (13)
#endif

/* SPI RAM chip select */
#ifdef ESP8266
#define RS          (D0)
#else
#define RS          (9)
#endif

#endif /* GLOBAL_H */
