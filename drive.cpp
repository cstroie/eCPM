/**
  drive.cpp - Drive interface

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

#include "drive.h"

DRIVE::DRIVE(RAM *ram): ram(ram) {
}

DRIVE::~DRIVE() {
}

void DRIVE::ledOn() {
  digitalWrite(BUILTIN_LED, HIGH ^ LEDinv);
}
void DRIVE::ledOff() {
  digitalWrite(BUILTIN_LED, LOW ^ LEDinv);
}

bool DRIVE::selDrive(uint8_t drive) {
  bool result = false;
  char disk[] = "A";
  // Adjust the drive letter
  disk[0] += drive;
  // Check for directory existence
  ledOn();
  if (file = SD.open(disk, FILE_READ)) {
    result = file.isDirectory();
    file.close();
  }
  ledOff();
  return result;
}

bool DRIVE::open(char* fname) {
  bool result = false;
  // Open the file
  ledOn();
  if (file = SD.open(fname, FILE_READ)) {
    file.close();
    result = true;
  }
  ledOff();
  return result;
}

uint32_t DRIVE::fileSize(char* fname) {
  uint32_t len = -1;
  // Open the file and get the size
  ledOn();
  if (file = SD.open(fname, FILE_READ)) {
    len = file.size();
    file.close();
  }
  ledOff();
  return len;
}

// Find the first file specified by the pattern in fname:
//   0    Drive letter              ("A")
//   1    User hex code             ("0")
//   2-12 File name in CP/M format  ("????????TXT")
//  13    Zero
// On success, fname will contain the following:
//   0    Drive letter              ("A")
//   1    User hex code             ("0")
//   2-12 File name in CP/M format  ("SAMPLE  TXT")
//  13-15 Zero
//  16-.. Full file name on SD card
uint8_t DRIVE::findFirst(char* fname, uint32_t &fsize) {
  // TODO Custom base path
  char path[] = {fname[FNDRIVE], '/', fname[FNUSER], '/', 0};
  // Keep the pattern (convert to uppercase)
  for (uint8_t i = 0; i < 11; i++) {
    char c = *(fname + FNFILE + i) & 0x7F;
    fPattern[i] = toupper(c);
  }
  // Make sure it ends with zero
  fPattern[11] = '\0';
  // Keep the path in fPath
  strncpy(fPath, path, 5);
  // Close the previously opened SD directory, if any
  if (fDir)
    fDir.close();
  // Open the SD directory (aka drive in CP/M)
  fDir = SD.open(path);
  // Check if the directory exists
  if (fDir)
    // Go find it
    return findNext(fname, fsize);
  else
    // Error
    return 0xFF;
}

// On success, fname will contain the following:
//   0    Drive letter              ("A")
//   1    User hex code             ("0")
//   2-12 File name in CP/M format  ("SAMPLE  TXT")
//  13-15 Zero
//  16-.. Full file name on SD card
uint8_t DRIVE::findNext(char *fname, uint32_t &fsize) {
  uint8_t result = 0xFF;
  ledOn();
  // Find the next file, skipping over directories
  while (file = fDir.openNextFile()) {
    // Store the path and file name in fName, starting at FNHOST
    strcpy(fname + FNHOST, fPath);
    strcat(fname + FNHOST, file.name());
    // Store the file size in fSize
    fsize = file.size();
    // Close the file
    file.close();
    // Skip over host directories
    if (file.isDirectory())
      continue;
    // Convert the file name to CP/M name and get user id
    char cname[12];
    uint8_t uid = fname2cname((char*)(fname + FNHOST), (char*)(fname + FNFILE));
    // Match the pattern
    if (match(fname + FNFILE, fPattern)) {
      // Return the drive letter in FNDRIVE and hex user code in FNUSER positions
      fname[FNDRIVE] = 'A'; // FIXME
      fname[FNUSER] = toHEX(uid);
      // Success
      result = 0x00;
      break;
    }
  }
  ledOff();
  return result;
}

// Check if there is a "$$$.SUB" file on the A drive
// FIXME Slow start, use directly 'open'
uint8_t DRIVE::checkSUB(uint8_t drive, uint8_t user) {
  // Filename and file size (not used)
  char      fName[128] = "A0$???????SUB";
  uint32_t  fSize;
  // Update drive and user bytes
  fName[FNDRIVE] = 'A' + drive;
  fName[FNUSER]  = toHEX(user);
  return (findFirst(fName, fSize) == 0x00) ? 0xFF : 0x00;
}

uint8_t DRIVE::read(uint16_t ramDMA, char* fname, uint32_t fpos) {
  uint8_t result = 0xFF;
  uint8_t buf[sizBK];
  ledOn();
  // Open the file
  if (file = SD.open(fname, FILE_READ)) {
    // Seek
    if (file.seek(fpos)) {
      // Clear the buffer (^Z)
      memset(buf, 0x1A, sizBK);
      // Read from file
      if (file.read(&buf[0], sizBK)) {
        // Write into RAM
        ram->write(ramDMA, buf, sizBK);
        result = 0x00;
      }
      else
        // Read error
        result = 0x01;
    } else {
      // Seek error
      if (fpos >= 0x010000UL * sizBK)
        // Seek past 8MB (largest file size in CP/M)
        result = 0x06;
      else {
        uint32_t exSize = file.size();
        // Round the file size up to next full logical extent
        exSize = sizEX * ((exSize / sizEX) + ((exSize % sizEX) ? 1 : 0));
        if (fpos < exSize)
          // Reading unwritten data
          result = 0x01;
        else
          // Seek to unwritten extent
          result = 0x04;
      }
    }
    // Close the file
    file.close();
  } else
    // Open error
    result = 0x10;
  ledOff();
  return result;
}

uint8_t DRIVE::write(uint16_t ramDMA, char* fname, uint32_t fpos) {
  uint8_t result = 0xFF;
  uint8_t buf[sizBK];
  ledOn();
  // Open the file for write
  if (file = SD.open(fname, FILE_WRITE)) {
    // Check if we need to seek beyond end
    if (fpos > file.size()) {
      // Seek to end
      file.seek(file.size());
      // Append
      for (uint32_t i = 0; i < fpos - file.size(); i++)
        if (file.write((uint8_t)0) != 1) {
          // Disk full
          result = 0x02;
          break;
        }
    }
    // Check if the result is unchanged
    if (result == 0xFF)
      // Seek
      if (file.seek(fpos)) {
        // Read from RAM
        ram->read(ramDMA, buf, sizBK);
        // Write to file
        if (file.write(&buf[0], sizBK))
          result = 0x00;
        else
          // Write error
          result = 0x02;
      } else
        // Seek error
        result = 0x06;
    // Close the file
    file.close();
  } else
    // Open error
    result = 0x10;
  ledOff();
  return result;
}

bool DRIVE::create(char* fname) {
  bool result = false;
  ledOn();
  file = SD.open(fname, FILE_WRITE);
  if (file) {
    file.close();
    result = true;
  }
  ledOff();
  return result;
}

// Remove a file from SD card.
bool DRIVE::remove(char* fname) {
  ledOn();
  SD.remove(fname);
  ledOff();
  return true;
}

bool DRIVE::rename(char* fname, char* newname) {
  bool result = false;
  ledOn();
  file = SD.open(fname, FILE_WRITE);
  if (file) {
    // FIXME
    //if (file.rename(newname)) {
    //  file.close();
    //  result = true;
    //}
  }
  ledOff();
  return result;
}

bool DRIVE::truncate(char* fname, uint8_t rec) {
  bool result = false;
  ledOn();
  if (file = SD.open(fname, FILE_WRITE)) {
    if (file.truncate(rec * sizBK)) {
      file.close();
      result = true;
    }
  }
  ledOff();
  return result;
}

// Matches a FCB name to a search pattern
bool DRIVE::match(char *cname, char* pattern) {
  bool result = true;
  for (uint8_t i = 0; i < 11; i++)
    if ((*pattern == *cname) or (*pattern == '?')) {
      cname++;
      pattern++;
      continue;
    }
    else {
      result = false;
      break;
    }
  return result;
}

// Convert a host file name (A/0/AB.TXT) to CP/M name (AB      TXT)
uint8_t DRIVE::fname2cname(char *fname, char *cname) {
  uint8_t i = 0;
  uint8_t uid = 0;
  // Find the last occurence of '/'
  char *pch;
  pch = strrchr(fname, '/');
  // Check if found
  if (pch != NULL) {
    // Get the user id from path (just before '/')
    uid = frHEX((char)(pch - 1)[0]);
    // Advance fname beyond '/'
    fname = pch + 1;
  }
  // Convert file name to uppercase and copy to CP/M name
  i = 0;
  while (*fname != 0 && *fname != '.') {
    *cname = toupper(*fname);
    fname++; cname++; i++;
  }
  // Fill the remainder chars in CP/M name with spaces
  for (; i < 8; i++) {
    *cname = ' ';
    cname++;
  }
  // Skip over the dot
  if (*fname == '.')
    fname++;
  // Convert extension to uppercase and copy to CP/M type
  i = 0;
  while (*fname != 0) {
    *cname = toupper(*fname);
    fname++; cname++; i++;
  }
  // Fill the remainder chars in CP/M type with spaces
  for (; i < 3; i++) {
    *cname = ' ';
    cname++;
  }
  // End with zero
  *cname = '\0';
  // Return the user id
  return uid;
}
