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

#include <SPI.h>
#include "drive.h"

time_t timeCallback() {
  return 1633123449UL;
}

DRIVE::DRIVE(RAM *ram, char *bdir): ram(ram), bDir(bdir) {
}

DRIVE::~DRIVE() {
}

/*
  Init the SD drive
*/
void DRIVE::init() {
  bool result;
  Serial.print(F("eCPM: Initializing SD card: "));
#ifdef ESP8266
  result = SD.begin(SS);
  //result = SD.begin(SS, SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0));
#else
  result = SD.begin(SS);
#endif
  if (not result) {
    Serial.println(F("failed!"));
    while (true) {
      yield();
      // Flash the led
      digitalWrite(LED, HIGH ^ LEDinv);
      delay(50);
      digitalWrite(LED, LOW ^ LEDinv);
      delay(50);
      digitalWrite(LED, HIGH ^ LEDinv);
      delay(50);
      digitalWrite(LED, LOW ^ LEDinv);
      delay(250);
    }
  }
  else {
    switch (SD.type()) {
      case 1:
        Serial.print(F("SD1"));
        break;
      case 2:
        Serial.print(F("SD2"));
        break;
      case 3:
        Serial.print(F("SDHC"));
        break;
      default:
        Serial.println(F("Unknown"));
    }
    Serial.printf(" FAT%d %dMb\r\n", SD.fatType(), SD.size64() / 1048576);
    // Set time callback
    SD.setTimeCallback(timeCallback);
  }
}

/*
  Turn the drive led on
*/
void DRIVE::ledOn() {
  digitalWrite(LED, HIGH ^ LEDinv);
}

/*
  Turn the drive led off
*/
void DRIVE::ledOff() {
  digitalWrite(LED, LOW ^ LEDinv);
}

/*
  Check the directory of the specified drive exists
*/
bool DRIVE::loadCCP(bool verbose) {
  bool result = false;
  uint8_t buf[128];
  uint8_t len = 0xFF;
  // Build the path
  strncpy(fPath, bDir, 16);
  strcat(fPath, "/");
  sprintf_P((char*)buf, CCP_FILE, MEMK);
  strcat(fPath, (char*)buf);
  if (verbose) {
    // Message
    Serial.print(F("eCPM: Loading "));
    Serial.print(fPath);
    Serial.print(F(": "));
  }
  // Check if the file exists
  if (file = SD.open(fPath, FILE_READ)) {
    result = true;
    uint16_t addr = CCPCODE;
    while (len > 0) {
      // Read from file
      ledOn();
      len = file.read(buf, 128);
      ledOff();
      // Write into memory
      ram->write(addr, buf, len);
      // Adjust address
      addr += len;
    }
    file.close();
  }
  if (not result) {
    if (verbose)
      Serial.println(F(" failed!"));
    while (true) {
      yield();
    }
  }
  else if (verbose)
    Serial.printf("0x%04X\r\n", CCPCODE);


#ifdef DEBUG
  ram->hexdump(CCPCODE, CCPCODE + 0x10, "CCP");
#endif

  return result;
}

/*
  Create the user code directory
*/
void DRIVE::mkDir(uint8_t drive, uint8_t user) {
  // The path according to drive letter and user code
  char disk[] = {'/', 'A' + (drive & 0x0F), '/', toHEX(user), '/', 0};
  // Build the path
  strncpy(fPath, bDir, 16);
  strncat(fPath, disk, 4);
  // Check if the drive directory exists
  ledOn();
  if (not SD.exists(fPath))
    SD.mkdir(fPath);
  ledOff();
}

/*
  Check the directory of the specified drive exists
*/
bool DRIVE::selDrive(uint8_t drive) {
  bool result = false;
  // The path according to drive letter
  char disk[] = {'/', 'A' + (drive & 0x0F), 0};
  // Build the path
  strncpy(fPath, bDir, 16);
  strncat(fPath, disk, 4);
  // Check if the drive directory exists
  ledOn();
  if (file = SD.open(fPath, FILE_READ)) {
    result = file.isDirectory();
    file.close();
  }
  ledOff();
  return result;
}

/*
  Check if the specified file is open in requested mode and,
  if not, open it.  Restore the file position if needed.
*/
bool DRIVE::check(char* cname, uint8_t mode) {
  bool result = false;
  char *fname;
  // Build the path
  fname = cname + FNHOST;
  cname2fname(cname, fname);
  // Check if the file is already open
  if (file) {
    // Check if the open file is the same
    if (strcmp(fname, file.name()) == 0) {
      // Check if the mode is enough
      if (lstMode == FILE_WRITE or lstMode == mode)
        // The file is the same
        result = true;
      else {
        // The file is the same, the mode needs to be changed
        // Keep the last position
        uint32_t lstPos = file.position();
        // Close the file
        file.close();
        // Open the file again in the specified mode
        if (file = SD.open(fname, mode)) {
          // Seek to the old position
          if (file.seek(lstPos)) {
            // Keep the mode
            lstMode = mode;
            result = true;
          }
          else
            // Seek error, close the file
            file.close();
        }
      }
    }
    else {
      // The file is not the same
      // Close the old file
      file.close();
      // Open the new file in the specified mode
      if (file = SD.open(fname, mode)) {
        // Keep the last open mode
        lstMode = mode;
        result = true;
      }
    }
  }
  else {
    // The file is not open, open it
    if (file = SD.open(fname, mode)) {
      // Keep the last mode
      lstMode = mode;
      result = true;
    }
  }
  return result;
}

/*
  Open the file specified by the host file name
*/
bool DRIVE::open(char* cname, uint8_t mode) {
  bool result = false;
  // Open the file
  ledOn();
  // Check the file is open
  if (check(cname, mode))
    result = true;
  ledOff();
  return result;
}

/*
  Close the file specified by the host file name
*/
void DRIVE::close(char* cname) {
  ledOn();
  // Check the file is open
  if (file)
    if (check(cname))
      // Close it
      file.close();
  ledOff();
}

/*
  Return the size of the file specified by the host file name
*/
uint32_t DRIVE::fileSize(char* cname, uint8_t mode) {
  uint32_t len = -1;
  // Open the file and get the size
  ledOn();
  // Check the file is open
  if (check(cname, mode))
    // Get the size
    len = file.size();
  ledOff();
  return len;
}

/*
  Find the first file specified by the pattern in cname:
    0    Drive letter              ("A")
    1    User hex code             ("0")
    2-12 File name in CP/M format  ("????????TXT")
   13    Zero
  On success, cname will contain the following:
    0    Drive letter              ("A")
    1    User hex code             ("0")
    2-12 File name in CP/M format  ("SAMPLE  TXT")
   13-15 Zero
   16-.. Full file name on SD card
*/
uint8_t DRIVE::findFirst(char* cname, uint32_t &fsize) {
  // Keep the drive letter and user hex code
  fDrive = cname[FNDRIVE];
  fUser = cname[FNUSER];
  char path[] = {'/', cname[FNDRIVE], '/', cname[FNUSER], '/', 0};
  // Keep the pattern (convert to uppercase)
  for (uint8_t i = 0; i < 11; i++) {
    char c = *(cname + FNFILE + i) & 0x7F;
    fPattern[i] = toupper(c);
  }
  // Make sure it ends with zero
  fPattern[11] = '\0';
  // Keep the path in fPath
  strncpy(fPath, bDir, 16);
  strncat(fPath, path, 6);
  // Close any previously opened SD directory
  if (fDir)
    fDir.close();
  // Open the SD directory (aka drive in CP/M)
  if (fDir = SD.open(fPath))
    // Check if the directory exists
    if (fDir.isDirectory())
      // Go find the first file
      return findNext(cname, fsize);
  // Error
  return 0xFF;
}

/*
  On success, cname will contain the following:
    0    Drive letter              ("A")
    1    User hex code             ("0")
    2-12 File name in CP/M format  ("SAMPLE  TXT")
   13-15 Zero
   16-.. Full file name on SD card
*/
uint8_t DRIVE::findNext(char *cname, uint32_t &fsize) {
  uint8_t result = 0xFF;
  ledOn();
  // Find the next file, skipping over directories
  while (File file = fDir.openNextFile()) {
    // Store the path and file name in fName, starting at FNHOST
    strcpy(cname + FNHOST, fDir.name());
    strcat(cname + FNHOST, file.name());
    // Store the file size in fSize
    fsize = file.size();
    // Close the file
    file.close();
    // Skip over host directories
    if (file.isDirectory())
      continue;
    // Convert the file name to CP/M name and get user id
    uint8_t uid = fname2cname((char*)(cname + FNHOST), (char*)cname);
    // Match the pattern
    if (match(cname + FNFILE, fPattern)) {
      // Success
      result = 0x00;
      break;
    }
  }
  ledOff();
  return result;
}

// Check if there is a "$$$.SUB" file on the A drive
uint8_t DRIVE::checkSUB(uint8_t drive, uint8_t user) {
  char fName[128] = "A0$$$     SUB";
  fName[0] += drive;
  fName[1] = toHEX(user);
  uint8_t result = open(fName) ? 0xFF : 0x00;
  close(fName);
  return result;
}

uint8_t DRIVE::read(uint16_t ramDMA, char* cname, uint32_t fpos) {
  uint8_t result = 0xFF;
  uint8_t buf[sizBK];
  ledOn();
  // Check the file is open
  if (check(cname)) {
    // Seek success flag
    bool skok = false;
    // Check if we need to seek
    if (file.position() == fpos)
      // No need to seek, already on position
      skok = true;
    else
      // Seek
      skok = file.seek(fpos);
    if (skok) {
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
  } else
    // Open error
    result = 0x10;
  ledOff();
  return result;
}

uint8_t DRIVE::write(uint16_t ramDMA, char* cname, uint32_t fpos) {
  uint8_t result = 0xFF;
  uint8_t buf[sizBK];
  ledOn();
  // Check the file is open in write mode
  if (check(cname, FILE_WRITE)) {
    // Seek success flag
    bool skok = false;
    // Check if we need to seek
    if (file.position() == fpos)
      // No need to seek, already on position
      skok = true;
    else {
      // Check if we need to seek beyond its end
      if (fpos > file.size()) {
        // Yes, seek to end
        if (file.seek(file.size())) {
          // Append
          for (uint32_t i = 0; i < fpos - file.size(); i++)
            if (file.write((uint8_t)0x1A) != 1) {
              // Disk full
              result = 0x02;
              break;
            }
        }
        else
          // Seek error
          result = 0x06;
      }
      else {
        // Seek inside written file
        skok = file.seek(fpos);
      }
    }
    // Check if the file seek was successfull
    if (skok) {
      // Read from RAM after flushing the buffers
      ram->read(ramDMA, buf, sizBK);
      // Write to file
      if (file.write(&buf[0], sizBK))
        result = 0x00;
      else
        // Write error
        result = 0x02;
    }
  }
  else
    // Open error
    result = 0x10;
  ledOff();
  return result;
}

bool DRIVE::create(char* cname) {
  bool result = false;
  ledOn();
  // Check the file is open in write mode
  if (check(cname, FILE_WRITE))
    result = true;
  ledOff();
  return result;
}

// Remove a file from SD card.
bool DRIVE::remove(char* cname) {
  char *fname;
  // Build the path
  fname = cname + FNHOST;
  cname2fname(cname, fname);
  ledOn();
  if (SD.exists(fname))
    SD.remove(fname);
  ledOff();
  return true;
}

/*
  Rename a file

  // Requires the following in SDClass
  bool rename(const char* pathFrom, const char* pathTo) {
    return (boolean)SDFS.rename(pathFrom, pathTo);
  }
*/
bool DRIVE::rename(char* cname, char* newname) {
  bool result = false;
  char *fname, *nfname;
  // Remove the existing file
  remove(newname);
  // Build the paths
  fname = cname + FNHOST;
  cname2fname(cname, fname);
  nfname = newname + FNHOST;
  cname2fname(newname, nfname);
  // The two file handlers
  File frFile, toFile;
  ledOn();
  if (SD.exists(fname)) {
    if (frFile = SD.open(fname, FILE_READ)) {
      if (toFile = SD.open(nfname, FILE_WRITE)) {
        uint8_t len;
        uint8_t buf[128];
        while ((len = frFile.read(buf, sizeof(buf))) > 0) {
          toFile.write(buf, len);
        }
        toFile.close();
      }
      frFile.close();
    }
    // Remove the old file
    if (SD.exists(nfname) and SD.exists(fname))
      SD.remove(fname);
    //SD.rename(fname, nfname);
  }
  ledOff();
  return result;
}

bool DRIVE::truncate(char* cname, uint8_t rec) {
  bool result = false;
  ledOn();
  // Check the file is open in write mode
  if (check(cname, FILE_WRITE))
    if (file.truncate(rec * sizBK))
      result = true;
  ledOff();
  return result;
}

/*
  Write the character c to the LST file
*/
void DRIVE::wrLST(char c) {
  // Check if the file needs to be open
  if (not devLST)
    ckLST();
  // Check is the file is open
  if (devLST) {
    ledOn();
    // Write to file
    devLST.write(c);
    // Keep the timestamp
    tsLST = millis();
    ledOff();
  }
}

/*
  Open, if necessary, the LST file
*/
bool DRIVE::ckLST() {
  bool result = false;
  char cname[128] = "A0DEV-LST TXT";
  char *fname;
  // Build the path
  fname = cname + FNHOST;
  cname2fname(cname, fname);
  // Try to open the file for write
  if (not devLST) {
    // Reset the timestamp if the file is not open
    tsLST = 0UL;
    ledOn();
    // Try to open the file
    if (devLST = SD.open(fname, FILE_WRITE)) {
      // Set the timestamp
      tsLST = millis();
      result = true;
    }
    ledOff();
  }
  return result;
}

/*
  Flush the LST file after a while
*/
void DRIVE::fsLST() {
  // Check if the file is open and the timestamp has been set
  if (devLST and tsLST > 0)
    // Check if timed out (10 seconds)
    if (millis() - tsLST > 10000UL) {
      ledOn();
      // Flush the file
      devLST.flush();
      ledOff();
    }
}

/*
  Close the LST file
*/
void DRIVE::clLST() {
  // Check if the file is open
  if (devLST)
    // Check if the timestamp has been set
    if (tsLST > 0) {
      ledOn();
      // Close the file
      devLST.close();
      ledOff();
    }
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

/*
  Convert a host file name (A/0/FILE.TXT) to CP/M name (A0FILE    TXT)
*/
uint8_t DRIVE::fname2cname(char *fname, char *cname) {
  uint8_t i = 0;
  char drv, usr;
  // Find the last occurence of '/'
  char *pch;
  pch = strrchr(fname, '/');
  // Check if found
  if (pch != NULL) {
    // Get the user id from path (just before '/')
    usr = (char)(pch - 1)[0];
    // Get the drive letter from path
    drv = (char)(pch - 3)[0];
    // Advance fname beyond '/'
    fname = pch + 1;
  }
  // Set the drive letter and user hex code
  *(cname++) = drv;
  *(cname++) = usr;
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
  while (*fname != 0 and i < 3) {
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
  return frHEX(usr);
}

/*
  Convert CP/M name (A0FILE    TXT) to a host file name (A/0/FILE.TXT)
*/
void DRIVE::cname2fname(char *cname, char *fname) {
  char c;
  // Start with the base directory
  strncpy(fname, bDir, 16);
  fname += strlen(bDir);
  *(fname++) = '/';
  // The drive letter
  c = *(cname++);
  *(fname++) = toupper(c);
  // Path separator
  *(fname++) = '/';
  // User hex code
  c = *(cname++);
  *(fname++) = toupper(c);
  // Path separator
  *(fname++) = '/';
  // File name
  for (uint8_t i = 0; i < 8; i++) {
    c = *(cname++) & 0x7F;
    if (c > ' ')
      *(fname++) = toupper(c);
  }
  // File type
  for (uint8_t i = 0; i < 3; i++) {
    c = *(cname++) & 0x7F;
    if (c > ' ') {
      // Only add the dot if there's an extension
      if (i == 0)
        *(fname++) = '.';
      *(fname++) = toupper(c);
    }
  }
  // End with zero
  *(fname++) = '\0';
}
