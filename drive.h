/**
  drive.h - Drive interface

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

#ifndef DRIVE_H
#define DRIVE_H

#include "Arduino.h"
#include <SD.h>
#include "global.h"
#include "config.h"
#ifdef SPI_RAM
#include "spiram.h"
#else
#include "mcuram.h"
#endif


class DRIVE {
  public:
    DRIVE(RAM *ram, char *bdir = "");
    ~DRIVE();
    void      init();
    bool      loadCCP(bool verbose = false);
    void      mkDir(uint8_t drive, uint8_t user);
    bool      selDrive(uint8_t drive);
    uint32_t  fileSize(char* fname, uint8_t mode = FILE_READ);
    uint8_t   findFirst(char* fname, uint32_t &fsize);
    uint8_t   findNext(char* fname, uint32_t &fsize);
    uint8_t   checkSUB(uint8_t drive, uint8_t user);
    uint8_t   read(uint16_t ramDMA, char* fname, uint32_t fpos);
    uint8_t   write(uint16_t ramDMA, char* fname, uint32_t fpos);
    bool      check(char* fname, uint8_t mode = FILE_READ);
    bool      open(char* fname, uint8_t mode = FILE_READ);
    void      close(char* fname);
    bool      create(char* fname);
    bool      remove(char* fname);
    bool      rename(char* fname, char* newname);
    bool      truncate(char* fname, uint8_t rec);

    bool      ckLST();
    void      wrLST(char c);
    void      fsLST();
    void      clLST();

  private:
    RAM       *ram;

    void      ledOn();
    void      ledOff();

    uint8_t   fname2cname(char *fname, char *cname);
    void      cname2fname(char *cname, char *fname);
    bool      match(char *cname, char* pattern);

    char      *bDir;              // Base directory on SD card
    char      fPath[64];          // Base file path
    char      fDrive;             // The drive letter of the file to find
    char      fUser;              // The user hex code of the file to find
    char      fPattern[12];       // File name pattern for searching

    File      file;               // Current file in use
    File      fDir;               // The directory to look into
    uint8_t   lstMode;            // Last file open mode

    File      devLST;             // The LIST device as file
    uint32_t  tsLST;              // The LIST device timestamp
};

#endif /* DRIVE_H */
