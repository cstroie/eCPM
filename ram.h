/**
  ram.h - SPI RAM management

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

#ifndef RAM_H
#define RAM_H

#include <Arduino.h>
#include "SPI.h"
#include "config.h"
#include "global.h"

// SPI RAM commands
enum SPIRAM_CMNDS {CMD_WRMR  = 0x01, CMD_RDMR = 0x05,
                   CMD_WRITE = 0x02, CMD_READ = 0x03,
                   CMD_RSTIO = 0xFF
                  };
// SPI RAM transfer modes
enum SPIRAM_MODES {MODE_BYTE = 0x00, MODE_SEQ = 0x40, MODE_PAGE = 0x80};


class RAM {
  public:
    RAM(int CS = SS, uint16_t bufSize = 16);
    ~RAM();
    void init();
    void clear();
    void reset();
    uint8_t   getByte(uint16_t addr);
    void      setByte(uint16_t addr, uint8_t data);
    uint16_t  getWord(uint16_t addr);
    void      setWord(uint16_t addr, uint16_t data);
    uint8_t   readByte(uint16_t addr);
    void      writeByte(uint16_t addr, uint8_t data);
    uint16_t  readWord(uint16_t addr);
    void      writeWord(uint16_t addr, uint16_t data);
    void      read(uint16_t addr, uint8_t *buf, uint16_t len);
    void      write(uint16_t addr, uint8_t *buf, uint16_t len);
    void      hexdump(uint16_t start = 0x0000, uint16_t stop = LASTBYTE);

  private:
    // SPI transactions
    void begin();
    void end();

    // Chip select
    int cs;

    // Buffer
    void      bufChange(uint16_t addr);
    uint8_t*  buf;
    bool      bufDirty = false;
    uint16_t  bufSize = 0;
    uint16_t  bufHalfSize = 0;
    uint16_t  bufStart = LASTBYTE;
    uint16_t  bufEnd = LASTBYTE;
};

#endif /* RAM_H */
