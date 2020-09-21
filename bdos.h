/**
  bdos.cpp - CP/M 2.2 compatible Basic Disk Operating System (BDOS)

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

#ifndef BDOS_H
#define BDOS_H

#include "Arduino.h"
#include <SD.h>
//#include <SdFat.h>
// Need to specify the namespace for SdFat
//using namespace sdfat;

#include "global.h"
#include "config.h"
#include "i8080.h"
#include "ram.h"
#include "drive.h"
#include "bios.h"

struct FCB_t  {
  union {
    struct {
      uint8_t dr;     // Drive: 0 for default, 1-16 for A-P.
      uint8_t fn[8];  // Filename, 7-bit ASCII.
      uint8_t tp[3];  // Filetype, 7-bit ASCII.
      uint8_t ex;     // Current extent.
      uint8_t s1;     // Reserved.
      uint8_t s2;     // Current extent, high byte.
      uint8_t rc;
      uint8_t al[16]; // File allocation.
      uint8_t cr;     // Current record within extent.
      uint8_t r0;     // Random access record number.
      uint8_t r1;     // Random access record number.
      uint8_t r2;     // Random access record number.
    };
    uint8_t buf[36];
  };
};

struct DIR_t {
  union {
    struct {
      uint8_t uu;     // User number. 0-15
      uint8_t fn[8];  // Filename
      uint8_t tp[3];  // Filetype
      uint8_t ex;     // Extent counter, low byte - takes values from 0-31
      uint8_t s1;     // Extent counter, high byte.
      uint8_t s2;     // Reserved, set to 0.
      uint8_t rc;     // Number of records used in this extent, low byte.
      uint8_t al[16]; // Allocation. Each AL is the number of a block on the disc.
    };
    uint8_t buf[32];
  };
};

class BDOS {
  public:
    BDOS(I8080 *cpu, RAM *ram, DRIVE *drv, BIOS *bios);
    ~BDOS();
    void init();
    void call(uint16_t port);

    bool    selDrive(uint8_t drive);
    bool    fcb2fname(FCB_t fcb, char* fname);

  private:
    I8080     *cpu;
    RAM       *ram;
    DRIVE     *drv;
    BIOS      *bios;

    void      bdosError(uint8_t err);
    void      readFCB();
    void      writeFCB();
    void      showFCB(char* comment = "");
    void      dirEntry(char *cname, uint8_t uid, uint32_t fsize);


    uint8_t   cDrive = 0;         // Current drive
    uint8_t   tDrive = 0;         // Temporary drive
    uint8_t   cUser  = 0;         // Current user
    uint16_t  ramDMA = TBUFF;     // DMA address
    uint16_t  ramFCB;             // FCB address
    uint16_t  rwoVector = 0x0000; // Read-only / Read-write vector
    uint16_t  alcVector = 0x0000; // Allocation vector
    uint16_t  logVector = 0x0000; // Logged drives vector

    FCB_t     fcb;                // FCB object
    char      fName[128];         // Filename
    char      cName[12];          // CP/M file name
    bool      fAllUsers;          // Find files from all users
    bool      fAllExnts;          // Report all extents
    uint32_t  fSize;              // File size
    uint32_t  fPos;               // File position (seek)
    uint32_t  fRec;               // File record (random seek)

    uint32_t  fRecs;              // File records in directory entry
    uint16_t  fExts;              // File extents in directory entry
    uint16_t  fExtU;              // File extents used in directory entry
    uint16_t  fAllB;              // First free allocation block

    uint16_t  result;             // Result from BDOS functions
};

#endif /* CPM_H */
