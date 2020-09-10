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
#include "global.h"
#include "config.h"
#include "i8080.h"
#include "ram.h"

class BDOS {
  public:
    BDOS(I8080 cpu, RAM ram);
    ~BDOS();
    void init();
    void call(uint16_t port);
  private:
    I8080 cpu;
    RAM   ram;
    uint8_t   cDrive = 0;         // Current drive
    uint8_t   cUser  = 0;         // Current user
    uint16_t  addrDMA = TBUFF;    // DMA address
    uint16_t  rwoVector = 0x0000; // Read-only / Read-write vector
    uint16_t  logVector = 0x0000; // Logged drives vector
};

#endif /* CPM_H */
