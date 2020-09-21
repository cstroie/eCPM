/**
  bios.h - CP/M 2.2 compatible Basic Input / Output System (BIOS)

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

#ifndef BIOS_H
#define BIOS_H

#include "Arduino.h"
#include "global.h"
#include "config.h"
#include "i8080.h"
#include "ram.h"


struct DPH_t {
  union {
    struct {
      uint16_t  xlt;  // Address of the logical-to-physical translation vector
      uint16_t  sp1;  // Scratch pad 1
      uint16_t  sp2;  // Scratch pad 2
      uint16_t  sp3;  // Scratch pad 3
      uint16_t  dbf;  // Address of a 128-byte scratch pad area for directory operations
      uint16_t  dpb;  // Address of a disk parameter block for this drive
      uint16_t  csv;  // Address of a 16-byte scratch pad area used for software check for changed disks
      uint16_t  alv;  // Address of a 32-byte scratch pad area for disk storage allocation information
    };
    uint8_t buf[16];
  };
};

struct DPB_t {
  union {
    struct {
      uint16_t  spt;  // Sectors per track
      uint8_t   bsh;  // Data allocation "Block Shift Factor"
      uint8_t   blm;  // Data allocation Block Mask
      uint8_t   exm;  // Extent Mask
      uint16_t  dsm;  // Total storage capacity of the disk drive
      uint16_t  drm;  // Number of the last directory entry
      uint8_t   al0;  // Allocation 0
      uint8_t   al1;  // Allocation 1
      uint16_t  cks;  // Check area Size
      uint16_t  off;  // Number of system reserved tracks at the beginning of the disk
    };
    uint8_t buf[16];
  };
};

class BIOS {
  public:
    BIOS(I8080 *cpu, RAM *ram);
    ~BIOS();
    void init();
    void call(uint16_t code);

    void    boot();
    void    wboot();
    uint8_t consts();
    uint8_t conin();
    void    conout();
    void    conout(uint8_t c);
    void    list();
    void    list(uint8_t c);
    void    punch();
    void    punch(uint8_t c);
    uint8_t reader();
    void    home();
    void    seldsk();
    void    settrk();
    void    setsec();
    void    setdma();
    uint8_t read();
    uint8_t write();
    uint8_t listst();
    void    sectran();

    DPH_t   dph;
    DPB_t   dpb;

  private:
    void    signon();
    void    gocpm();
    void    loadCCP();
    I8080   *cpu;
    RAM     *ram;
    uint8_t result;
};

#endif /* BIOS_H */
