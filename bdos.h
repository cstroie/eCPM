/**
  bdos.cpp - BDOS emulator

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

// BDOS function calls
enum BDOS_CALLS {BDOS_WBOOT, BDOS_GETCON, BDOS_OUTCON, BDOS_GETRDR, BDOS_PUNCH,
                 BDOS_LIST, BDOS_DIRCIO, BDOS_GETIOB, BDOS_SETIOB, BDOS_PRTSTR,
                 BDOS_RDBUFF, BDOS_GETCSTS, BDOS_GETVER, BDOS_RSTDSK, BDOS_SETDSK,
                 BDOS_OPENFIL, BDOS_CLOSEFIL, BDOS_GETFST, BDOS_GETNXT, BDOS_DELFILE,
                 BDOS_READSEQ, BDOS_WRTSEQ, BDOS_FCREATE, BDOS_RENFILE, BDOS_GETLOG,
                 BDOS_GETCRNT, BDOS_PUTDMA, BDOS_GETALOC, BDOS_WRTPRTD, BDOS_GETROV,
                 BDOS_SETATTR, BDOS_GETPARM, BDOS_GETUSER, BDOS_RDRANDOM, BDOS_WTRANDOM,
                 BDOS_FILESIZE, BDOS_SETRAN, BDOS_LOGOFF, BDOS_RTN_0, BDOS_RTN_1, BDOS_WTSPECL,
                };

class BDOS {
  public:
    BDOS(I8080 cpu, RAM ram);
    ~BDOS();
    void init();
    void call(uint16_t port);
  private:
    I8080 cpu;
    RAM   ram;
};

#endif /* CPM_H */
