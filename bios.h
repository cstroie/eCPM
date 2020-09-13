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

    void boot();
    void wboot();
    void consts();
    void conin();
    void conout();
    void list();
    void punch();
    void reader();
    void home();
    void seldsk();
    void settrk();
    void setsec();
    void setdma();
    void read();
    void write();
    void listst();
    void sectran();

  private:
    void  signon();
    void  gocpm();
    void  loadCCP();
    I8080 *cpu;
    RAM   *ram;

};

#endif /* BIOS_H */
