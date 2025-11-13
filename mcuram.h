/**
  mcuram.h - MCU RAM management

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

#ifndef MCURAM_H
#define MCURAM_H

#include <Arduino.h>
#ifdef MMU_IRAM_HEAP
#include <umm_malloc/umm_heap_select.h>
#endif
#include "config.h"
#include "global.h"

class MCURAM {
  public:
    MCURAM();
    ~MCURAM();
    void      init();
    void      clear();
    void      reset();
    void      flush();
    void      flush(uint16_t addr);
    uint8_t   getByte(uint16_t addr);
    void      setByte(uint16_t addr, uint8_t data);
    uint16_t  getWord(uint16_t addr);
    void      setWord(uint16_t addr, uint16_t data);
    void      read(uint16_t addr, uint8_t *data, uint16_t len);
    void      write(uint16_t addr, uint8_t *data, uint16_t len);
    void      hexdump(uint16_t start = 0x0000, uint16_t stop = LASTBYTE, char* comment = "");

  private:
    // Buffer
    uint8_t*  buf;    // Primary buffer in DRAM
    uint8_t*  ibuf;   // Secondary buffer in IRAM (optional)
};

#endif /* MCURAM_H */
