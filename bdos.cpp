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

#include "bdos.h"
#include <SD.h>

// BDOS CALLS
const char* BDOS_CALLS[] = {"WBOOT", "GETCON", "OUTCON", "GETRDR", "PUNCH", "LIST",
                            "DIRCIO", "GETIOB", "SETIOB", "PRTSTR", "RDBUFF", "GETCSTS",
                            "GETVER", "RSTDSK", "SETDSK", "OPENFIL", "CLOSEFIL",
                            "GETFST", "GETNXT", "DELFILE", "READSEQ", "WRTSEQ",
                            "FCREATE", "RENFILE", "GETLOG", "GETCRNT", "PUTDMA",
                            "GETALOC", "WRTPRTD", "GETROV", "SETATTR", "GETPARM",
                            "GETUSER", "RDRANDOM", "WTRANDOM", "FILESIZE", "SETRAN",
                            "LOGOFF", "RTN", "RTN", "WTSPECL",
                           };

BDOS::BDOS(I8080 *cpu, RAM *ram, DRIVE *drv, BIOS *bios): cpu(cpu), ram(ram), drv(drv), bios(bios) {
}

BDOS::~BDOS() {
}

void BDOS::init() {
  Serial.print("eCPM: Initializing BDOS...");
  // Serial number, 6 bytes
  uint8_t cpmSerialNo[] = {0, 22, 0, 0, 0, 0};
  ram->write(BDOSCODE, cpmSerialNo, sizeof(cpmSerialNo));

  // BDOS entry (FBASE)
  ram->setByte(BDOSCODE + 6, 0xC3);       // JP BDOSENTRY
  ram->setWord(BDOSCODE + 7, BDOSENTRY);

  // BDOS dispatcher (FBASE1)
  ram->setByte(BDOSENTRY,     0xDB);      // IN A, (0x00)
  ram->setByte(BDOSENTRY + 1, 0x00);
  ram->setByte(BDOSENTRY + 2, 0xC9);      // RET
  Serial.print(" done.\r\n");

#ifdef DEBUG
  ram->hexdump(BDOSCODE,  BDOSCODE  + 0x10, "BDOS");
  ram->hexdump(BDOSENTRY, BDOSENTRY + 0x10, "BDOS ENTRY");
#endif
}

uint8_t BDOS::call(uint16_t port) {
  uint16_t  w;
  uint8_t   b, count;
  char      c;

  // Save the (DE) parameters
  params = cpu->regDE();
  // Get function number
  func = cpu->regC();
  // Keep single register function here
  cpu->regC(cpu->regE());
  // HL is address of the function
  cpu->regHL(BDOSENTRY);
  // Clear return status
  result = 0x0000;

#ifdef DEBUG_BDOS_CALLS
  Serial.print("\r\n\t\tBDOS call 0x");
  Serial.print(func, HEX);
  if (func <= 41) {
    Serial.print("\t");
    Serial.print(BDOS_CALLS[func]);
  }
  Serial.print("\r\n");
  cpu->trace();
#endif

  // Dispatch call
  switch (func) {
    case 0x00:  // WBOOT
      // System reset
      bios->wboot();
      break;

    case 0x01:  // GETCON
      // Function to get a character from the console device.
      c = bios->conin();
      result = c;
      if (c == 0x0A or c == 0x0D or c == 0x09 or c == 0x08 or c >= ' ')
        cpu->regE(c);
      else
        // A regular control char so don't echo
        break;

    case 0x02:  // OUTCON
      // Function to output (E) to the console device and expand tabs if necessary.
      bios->conout(cpu->regE());
      break;

    case 0x03:  // GETRDR
      // Function to get a character from the tape reader device.
      result = bios->reader();
      break;

    case 0x04:  // PUNCH
      // Auxiliary (Punch) output
      bios->punch(cpu->regE());
      break;

    case 0x05:  // LIST
      // Printer output
      bios->list(cpu->regE());
      break;

    case 0x06:  // DIRCIO
      // Function to perform direct console i/o. If (C) contains (FF)
      // then this is an input request. Otherwise we are to output (C).
      if (cpu->regE() == 0xFF) {
        if (bios->consts() == 0xFF)
          result = bios->conin();
        else
          result = 0x0000;
      }
      else {
        // TODO outcon()
        bios->conout(cpu->regE());
      }
      break;

    case 0x07:  // GETIOB
      // Function to return the i/o byte.
      b = ram->getByte(IOBYTE);
      bios->ioByte(b);
      result = b;
      break;

    case 0x08:  // SETIOB
      // Function to set the i/o byte.
      b = cpu->regE();
      bios->ioByte(b);
      ram->setByte(IOBYTE, b);
      break;

    case 0x09:  // PRTSTR
      // Function to print the character string pointed to by (DE)
      // on the console device. The string ends with a '$'.
      w = cpu->regDE();
      b = ram->getByte(w++);
      while (b != '$') {
        bios->conout(b);
        b = ram->getByte(w++);
      }
      cpu->regDE(w);
      break;

    case 0x0A:  // RDBUFF
      // Function to execute a buffered read.
      // DE = Address of buffer
      // Read (DE) bytes from the console
      // Return: A = Number of chars read
      // (DE) = First char
      // Get the number of characters to read
      w = cpu->regDE();
      b = ram->getByte(w);
      w++;
      count = 0;
      // Very simple line input
      while (b) {
        c = bios->conin();
        if (c == 3 && count == 0) {
          // ^C
          Serial.write("^C");
          //Status = 2;
          break;
        }
        else if (c == 5)
          // ^E
          Serial.write("\r\n");
        else if ((c == 0x08 || c == 0x7F) && count > 0) {
          // ^H and DEL
          Serial.write("\b \b");
          count--;
          continue;
        }
        else if (c == 0x0A || c == 0x0D) {
          // ^J and ^M
          break;
        }
        else if (c == 18) {
          // ^R
          Serial.write("#\r\n   ");
          for (uint8_t j = 1; j <= count; ++j)
            bios->conout(ram->getByte(w + j));
        }
        else if (c == 21) {
          // ^U
          Serial.write("#\r\n   ");
          w = cpu->regDE();
          b = ram->getByte(w);
          w++;
          count = 0;
        }
        else if (c == 24) {
          // ^X
          for (uint8_t j = 0; j < count; ++j)
            Serial.write("\b \b");
          w = cpu->regDE();
          b = ram->getByte(w);
          w++;
          count = 0;
        }
        else if (c < 0x20 || c > 0x7E)
          // Invalid character
          continue;
        bios->conout(c);
        ++count;
        ram->setByte(w + count, c);
        // Reached the expected count
        if (count == b)
          break;
      }
      // Save the number of characters read
      ram->setByte(w, count);
      // Gives a visual feedback that read ended
      bios->conout('\r');
      break;

    case 0x0B:  // GETCSTS
      // Function to interigate the console device.
      result = bios->consts();
      break;

    case 0x0C:  // GETVER
      // Function to return the current cp/m version number.
      // A=L=0x22 => 2.2
      // B=H=0x00 => 8080, CP/M
      result = 0x0022;
      break;

    case 0x0D:  // RSTDSK
      // Function to reset the disk system.
      cDrive = 0;           // Select drive 'A'
      rwoVector = 0x0000;   // Clear write protect vector
      logVector = 0x0001;   // Reset log in vector
      ramDMA = TBUFF;       // Setup default DMA address
      // Check if there is a $$$.SUB on the boot disk
      result = drv->checkSUB(cDrive, cUser);
      break;

    case 0x0E:  // SETDSK
      // Function to set the active disk number.
      result = 0xFF;
      // Check if this is a drive change
      if (cpu->regE() != cDrive) {
        // Save the current drive number
        tDrive = cDrive;
        // Change the drive
        cDrive = cpu->regE();
        // Check it
        if (selDrive(cDrive + 1)) {
          // Set the drive log in vector
          logVector = logVector | (1 << cDrive);
          // Check if there is a $$$.SUB on the boot disk
          result = drv->checkSUB(cDrive, cUser);
        }
        else
          // Error, restore the current drive
          cDrive = tDrive;
      }
      break;

    case 0x0F:  // OPENFIL
      // Function to open a specified file.
      result = 0xFF;
      // Read the FCB from RAM
      readFCB();
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Get the filename
        fcb2cname(fcb, fName);
        // Try to open it
        if (drv->open(fName)) {
          // Get the file size (in blocks)
          fSize = drv->fileSize(fName) / sizBK;
          // Reset S1 and S2
          fcb.s1 = 0x00;
          fcb.s2 = 0x80; // set unmodified flag
          // Set the record count
          fcb.rc = fSize > maxRC ? maxRC : (uint8_t)fSize;
          // Clean up allocation
          for (uint8_t i = 0; i < 16; i++)
            fcb.al[i] = 0x00;
          // Success
          result = 0x00;
        }
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x10:  // CLOSEFIL
      // Function to close a specified file.
      result = 0xFF;
      // Read the FCB from RAM
      readFCB();
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Check if the file has been modifed
        if (!(fcb.s2 & 0x80)) {
          // Check if the drive is write protected
          if (!(rwoVector & (1 << fcb.dr))) {
            // Get the filename on SD card
            fcb2cname(fcb, fName);
            // Check if this file is '$$$.SUB', whose FCB is at BATCHFCB RAM address
            // This file is written by SUBMIT.COM from the *.SUB file, from the last
            // line to the first, one line per record.  CCP reads the last record
            // (which contains the next line) then decrements the RC field and closes
            // the file, efectivelly trimming the last record.
            if (ramFCB == BATCHFCB)
              // Truncate it to fcb.rc CP/M records so SUBMIT.COM can work
              drv->truncate(fName, fcb.rc);
            // Close the file
            drv->close(fName);
            result = 0x00;
          }
          else
            // Return error 4 if write protected
            bdosError(4);
        }
        else
          // Success
          result = 0x00;
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x11:  // GETFST
      // Function to return the first occurence of a specified file name.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Keep the drive letter, user hexcode and CP/M filename
        // (pattern, actually) in fName: 'A0???????????'
        /*
          fName[FNDRIVE] = (fcb.dr && fcb.dr != '?') ? (fcb.dr + 'A' - 1) : (cDrive + 'A');
          fName[FNUSER]  = toHEX(cUser);
          memcpy(fName + FNFILE, fcb.fn, 11);
          fName[FNZERO]  = '\0';
        */
        fcb2cname(fcb, fName);
        // Check if we need to find file from all users
        fAllUsers = fcb.dr == '?';
        // Check if we should report all extents
        fAllExnts = fcb.ex == '?';
        // FIXME
        if (fAllUsers)
          result = drv->findFirst(fName, fSize);
        else
          result = drv->findFirst(fName, fSize);
        if (result == 0x00) {
          // Reset direntry file records, extents and allocation blocks
          fRecs = 0;
          fExts = 0;
          fExtU = 0;
          fAllB = 0x10;
          // On return, fName contains the drive letter,
          // hex user code and the CP/M name.
          // Create a directory entry
          dirEntry(fName + FNFILE, frHEX(fName[FNUSER]), fSize);
        }
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x12:  // GETNXT
      // Function to return the next occurence of a file name.
      result = 0xFF;
      // Check if all file records have been reported
      if (fRecs > 0 and fAllExnts) {
        // Create a new directory entry
        dirEntry(fName + FNFILE, frHEX(fName[FNUSER]), fSize);
        result = 0x00;
      }
      else
        // Select the drive
        if (selDrive(fcb.dr)) {
          // FIXME
          if (fAllUsers)
            result = drv->findNext(fName, fSize);
          else
            result = drv->findNext(fName, fSize);
          if (result == 0x00) {
            // Reset direntry file records, extents and allocation blocks
            fRecs = 0;
            fExts = 0;
            fExtU = 0;
            fAllB = 0x10;
            // On return, fName contains the drive letter,
            // hex user code and CP/M name
            // Create a new directory entry
            dirEntry(fName + FNFILE, frHEX(fName[1]), fSize);
          }
        }
      break;

    case 0x13:  // DELFILE
      // Function to delete a file by name.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Keep the drive letter, user hexcode and CP/M filename
          // (pattern, actually) in fName: 'A0???????????'
          /*
            fName[FNDRIVE] = fcb.dr + 'A';
            fName[FNUSER] = toHEX(cUser);
            memcpy(fName + FNFILE, fcb.fn, 11);
            fName[FNZERO] = '\0';
          */
          fcb2cname(fcb, fName);
          result = drv->findFirst(fName, fSize);
          // Now fName will contain individual matching files
          while (result != 0xFF) {
            // Delete it, the full file name starts at FNHOST
            drv->remove(fName);
            // Find the next one
            result = drv->findNext(fName, fSize);
          }
          // Success
          result = 0x00;
        }
        else
          // Return error 4 if write protected
          bdosError(4);
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x14:  // READSEQ
      // Function to execute a sequential read of the specified record number.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Get position from FCB
      fRec = (fcb.s2 & mskS2) * recS2 + fcb.ex * recEX + fcb.cr;
      fPos = fRec * sizBK;
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Get the filename
        fcb2cname(fcb, fName);
        // Read one block
        result = drv->read(ramDMA, fName, fPos);
        // Check the result
        if (!result) {
          // Increase file record and seek position with one block
          fRec++;
          fPos += sizBK;
          // Adjust FCB
          fcb.s2 = (fRec / recS2) | (fcb.s2 & 0x80);  // extent, high byte
          fcb.ex = (fRec % recS2) / recEX;            // extent, low byte
          fcb.cr =  fRec % recEX;                     // cr (current record)
        }
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x15:  // WRTSEQ
      // Function to write the net sequential record.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Get position from FCB
      fRec = (fcb.s2 & mskS2) * recS2 + fcb.ex * recEX + fcb.cr;
      fPos = fRec * sizBK;
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Get the filename on SD card
          fcb2cname(fcb, fName);
          // Write one block
          result = drv->write(ramDMA, fName, fPos);
          // Check the result
          if (!result) {
            // Increase file record and seek position with one block
            fRec++;
            fPos += sizBK;
            // Adjust FCB
            fcb.s2 = (fRec / recS2) & 0x7F;   // extent, high byte, reset unmodified flag
            fcb.ex = (fRec % recS2) / recEX;  // extent, low byte
            fcb.cr =  fRec % recEX;           // cr (current record)
          }
        }
        else
          // Return error 4 if write protected
          bdosError(4);
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x16:  // FCREATE
      // Create a file function.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Get the filename
          fcb2cname(fcb, fName);
          // Create the file
          if (drv->create(fName)) {
            // Initializes the FCB
            fcb.ex = 0x00;
            fcb.s1 = 0x00;
            fcb.s2 = 0x00;
            fcb.rc = 0x00;
            // Clean allocation
            for (uint8_t i = 0; i < 16; i++)
              fcb.al[i] = 0x00;
            fcb.cr = 0x00;
            result = 0x00;
          }
        }
        else
          // Return error 4 if write protected
          bdosError(4);
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x17:  // RENFILE
      // Function to rename a file.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          uint16_t ramNewFCB = ramFCB + 16;
          // Prevents rename from moving files among drives
          ram->setByte(ramNewFCB, ram->getByte(ramFCB));
          // Create the new FCB object
          FCB_t newfcb;
          ram->read(ramNewFCB, newfcb.buf, 36);
          // Get the filename on SD card
          fcb2cname(fcb, fName);
          // Get the new filename on SD card
          char newName[128];
          fcb2cname(newfcb, newName);
          // Rename the file
          if (drv->rename(fName, newName))
            result = 0x00;
        }
        else
          // Return error 4 if write protected
          bdosError(4);
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x18:  // GETLOG
      // Function to return the login vector.
      result = logVector;
      break;

    case 0x19:  // GETCRNT
      // Function to return the current disk assignment.
      result = cDrive;
      break;

    case 0x1A:  // PUTDMA
      // Function to set the dma address.
      ramDMA = cpu->regDE();
      cpu->regBC(ramDMA);
      bios->setdma();
      break;

    case 0x1B:  // GETALOC
      // Function to return the allocation vector.
      result = alcVector;
      break;

    case 0x1C:  // WRTPRTD
      // Function to write protect the current disk.
      logVector = logVector | (1 << cDrive);
      break;

    case 0x1D:  // GETROV
      // Function to return the read-only status vector.
      result = rwoVector;
      break;

    case 0x1E:  // SETATTR
      // Function to set the file attributes (read-only, system).
      break;

    case 0x1F:  // GETPARM
      // Function to return the address of the disk parameter block for the current drive.
      result = BIOSDPB;
      break;

    case 0x20:  // GETUSER
      // Function to get or set the user number. If (E) was (FF)
      // then this is a request to return the current user number.
      // Else set the user number from (E).
      if (cpu->regE() == 0xFF)
        result = cUser & 0x000F;
      else
        // TODO Make user dir
        cUser = cpu->regE() & 0x0F;
      break;

    case 0x21:  // RDRANDOM
      // Function to read a random record from a file.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Compute the file record and seek position
      fRec = fcb.r2 * 0x010000UL + fcb.r1 * 0x0100UL + fcb.r0;
      fPos = fRec * sizBK;
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Get the filename
        fcb2cname(fcb, fName);
        // Read one block
        result = drv->read(ramDMA, fName, fPos);
        // Check the result
        if (result == 0 or result == 1 or result == 4) {
          // Adjust FCB
          fcb.s2 = (fRec / recS2) | (fcb.s2 & 0x80);  // extent, high byte
          fcb.ex = (fRec % recS2) / recEX;            // extent, low byte
          fcb.cr =  fRec % recEX;                     // cr (current record)
        }
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x22:  // WTRANDOM
      // Function to write a random record to a file.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Compute the file record and seek position
      fRec = fcb.r2 * 0x010000UL + fcb.r1 * 0x0100UL + fcb.r0;
      fPos = fRec * sizBK;
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Get the filename
          fcb2cname(fcb, fName);
          // Write one block
          result = drv->write(ramDMA, fName, fPos);
          // Check the result
          if (!result) {
            // Adjust FCB
            fcb.s2 = (fRec / recS2) & 0x7F;   // extent, high byte, reset unmodified flag
            fcb.ex = (fRec % recS2) / recEX;  // extent, low byte
            fcb.cr =  fRec % recEX;           // cr (current record)
          }
        }
        else
          // Return error 4 if write protected
          bdosError(4);
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x23:  // FILESIZE
      // Function to compute the size of a random file.
      result = 0xFF;
      // Read the FCB from RAM
      readFCB();
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Get the filename
        fcb2cname(fcb, fName);
        // Get the file size (in blocks)
        fSize = drv->fileSize(fName) / sizBK;
        if (fSize != -1) {
          fcb.r0 =  fSize        & 0xFF;
          fcb.r1 = (fSize >>  8) & 0xFF;
          fcb.r2 = (fSize >> 16) & 0xFF;
          // Success
          result = 0x00;
        }
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x24:  // SETRAN
      // Function to return the random record position of a given
      // file which has been read in sequential mode up to now.
      result = 0x00;
      // Read the FCB from RAM
      readFCB();
      // Get the last used record (CR, EX, S2)
      fRec  =  fcb.cr & mskCR;
      fRec += (fcb.ex & mskEX) * recEX;
      fRec += (fcb.s2 & mskS2) * recS2;
      // Compute the random position and store it into R0, R1 and R2
      fcb.r0 =  fRec        & 0xFF;
      fcb.r1 = (fRec >>  8) & 0xFF;
      fcb.r2 = (fRec >> 16) & 0xFF;
      // Write the FCB back into RAM
      writeFCB();
      break;

    case 0x25:  // LOGOFF
      // This allows a program to log off any drives.
      break;

    case 0x26:  // DRVACCESS
      // Locks one or more disc drives (not implemented MP/M only).
      result = 0x0000;
      break;

    case 0x27:  // DRVFREE
      // Releases locks on disc drives (not implemented MP/M only).
      result = 0x0000;
      break;

    case 0x28:  // WTSPECL
      // For the case where we are writing to unused disk space, this
      // space will be zeroed out first.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Compute the file record and seek position
      fRec = fcb.r2 * 0x010000UL + fcb.r1 * 0x0100UL + fcb.r0;
      fPos = fRec * sizBK;
      // Select the drive
      if (selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Get the filename
          fcb2cname(fcb, fName);
          // Write one block
          result = drv->write(ramDMA, fName, fPos);
          // Check the result
          if (!result) {
            // Adjust FCB
            fcb.s2 = (fRec / recS2) & 0x7F;   // extent, high byte, reset unmodified flag
            fcb.ex = (fRec % recS2) / recEX;  // extent, low byte
            fcb.cr =  fRec % recEX;           // cr (current record)
          }
        }
        else
          // Return error 4 if write protected
          bdosError(4);
      }
      // Write the FCB back into RAM
      writeFCB();
      break;

    default:
#ifdef DEBUG_BDOS_CALLS
      // Show unimplemented BDOS calls only when debugging
      Serial.print("\r\nUnimplemented BDOS call 0x");
      Serial.print(func, HEX);
      Serial.print("\r\n");
      cpu->trace();
#endif
      break;
  }

  // Get return status
  cpu->regHL(result);
  // Force version 1.4 compatibility
  cpu->regB((uint8_t)cpu->regH());
  cpu->regA((uint8_t)cpu->regL());

  // Return
  return cpu->regA();
}


void BDOS::bdosError(uint8_t err) {
  Serial.print("\r\nBdos Err On ");
  Serial.print((char)(cDrive + 'A'));
  Serial.print(" : ");
  switch (err) {
    case 1:
      Serial.print("Bad Sector");
      break;
    case 2:
      Serial.print("Select");
      break;
    case 3:
      Serial.print("File R/O");
      break;
    case 4:
      Serial.print("R/O");
      break;
  }
  Serial.print("\r\n");
  // Wait for a keypress
  bios->conin();
  // Restore the TDRIVE byte
  cDrive = tDrive;
  ram->setByte(TDRIVE, cUser << 4 | cDrive);
  // Always reboot on these errors.
  bios->wboot();
}

// Read the FCB from RAM, register DE has the address.
// Store the address in ramFCB and the FCB in fcb
void BDOS::readFCB() {
  // Get the FCB address
  ramFCB = cpu->regDE();
  // Create the FCB object
  ram->read(ramFCB, fcb.buf, 36);
  // Uppercase file name and type
  for (uint8_t i = 0; i < 11; i++)
    *(fcb.fn + i) = toupper(*(fcb.fn + i) & 0x7F);
#ifdef DEBUG_FCB_READ
  // Show FCB
  showFCB(BDOS_CALLS[func]);
#endif
}

// Write the FCB back into RAM.
// The address is already in ramFCB and the FCB in fcb
void BDOS::writeFCB() {
  // Write the FCB back into RAM
  ram->write(ramFCB, fcb.buf, 36);
#ifdef DEBUG_FCB_WRITE
  // Show FCB
  showFCB(BDOS_CALLS[func]);
#endif
}

// Debug FCB
void BDOS::showFCB(const char* comment) {
  char buf[80];
  // Start with a new line
  Serial.print("\r\n");
  // Print the comment
  if (comment[0]) {
    Serial.print("\r\n; ");
    Serial.print(comment);
  }
  Serial.print("\r\nDR FN       TP   EX S1 S2 RC CR R0 R1 R2");
  Serial.print("\r\n ");
  Serial.print((char)(fcb.dr == 0 ? '*' : ((fcb.dr == '?') ? '?' : (fcb.dr + 'A' - 1))));
  Serial.print(' ');
  for (uint8_t i = 0; i < 8; i++) Serial.print((char)fcb.fn[i]);
  Serial.print(' ');
  for (uint8_t i = 0; i < 3; i++) Serial.print((char)fcb.tp[i]);
  sprintf_P(buf, PSTR("  %02X %02X %02X %02X %02X %02X %02X %02X "),
            fcb.ex, fcb.s1, fcb.s2, fcb.rc, fcb.cr, fcb.r0, fcb.r1, fcb.r2);
  Serial.print(buf);
  //for (uint8_t i = 0; i < 16; i++) {
  //  sprintf_P(buf, PSTR(" %02X"), fcb.al[i]);
  //  Serial.print(buf);
  //}
  Serial.print("\r\n");
}

// Create a directory entry into RAM
void BDOS::dirEntry(char *cname, uint8_t uid, uint32_t fsize) {
  uint8_t blocks, i;
  // Create a new directory entry object
  DIR_t de;
  // Zero it out
  memset(de.buf, 0, 32);
  // Store the user id
  de.uu = uid;
  // Copy the file name and type from cname (8+3 bytes)
  memcpy(de.fn, cname, 11);
  // Check if it's a new file and we should report all extents
  if (fRecs == 0) {
    // Compute the number of records
    fRecs = (fsize + sizBK - 1) / sizBK;
    // Compute the number of extents
    fExts = (fRecs + recEX - 1) / recEX;
    // Reset the used extents (start with zero)
    fExtU = 0;
  }
  // Extents per directory entry
  uint8_t expde = bios->dpb.exm + 1;
  // Check if the file fits in a single directory entry
  if (fExts <= expde) { // for now, 2 extents
    // Yes, compute the ex, s2 and rc fields
    if (fExts > 0) {
      // Maximum value of 'ex' is 31 (mskEX)
      // Since this might be the tail of multi-extents file, we need
      // to add the count of already used extents
      uint8_t extents = fExts + fExtU - 1;
      de.ex = extents % (mskEX + 1);
      de.s2 = extents / (mskEX + 1);
      // The 'rc' is the number of the records not already allocated
      // in previous extents
      de.rc = fRecs - (recEX * (fExts - 1));
    }
    // Get the number of required allocation blocks
    // (for now, 32 recors in one block)
    blocks = ((fRecs + bios->dpb.blm) / (bios->dpb.blm + 1));
    // Reset the records and extents counters
    fRecs = 0;
    fExts = 0;
    fExtU = 0;
  }
  else {
    // No, we need more extents, max out this directory entry
    // Since this might be in the middle of multi-extents file,
    // we need to add the count of already used extents
    uint8_t extents = expde + fExtU - 1;
    de.ex = extents % (mskEX + 1);
    de.s2 = extents / (mskEX + 1);
    // The 'rc' will be the number of the records in an extent
    de.rc = recEX;
    // Required allocation blocks may be 8 or 16, according to DSM
    // (for now, the disk is 8MB, so 8 blocks of 16 bits)
    blocks = bios->dpb.dsm <= 255 ? 16 : 8;
    // Update the records and extents counts for next call, using
    // the value of extents per directory entry
    fRecs -= expde * recEX;
    fExts -= expde;
    fExtU += expde;
  }

  // Fill up the appropriate number of allocation blocks
  if (bios->dpb.dsm <= 255)
    // Maximum of 16 blocks of 8 bits
    for (i = 0; i < blocks; i++)
      de.al[i] = (uint8_t)fAllB++;
  else
    // Maximum of 8 blocks of 16 bits
    for (i = 0; i < 2 * blocks; i += 2) {
      de.al[i]     = fAllB & 0xFF;
      de.al[i + 1] = fAllB >> 8;
      fAllB++;
    }

#ifdef DEBUG_DIRENTRY
  char buf[80];
  Serial.print("\r\nUU FN       TP   EX S1 S2 RC");
  Serial.print("\r\n ");
  Serial.print((char)(toHEX(de.uu)));
  Serial.print(' ');
  for (uint8_t i = 0; i < 8; i++) Serial.print((char)de.fn[i]);
  Serial.print(' ');
  for (uint8_t i = 0; i < 3; i++) Serial.print((char)de.tp[i]);
  sprintf_P(buf, PSTR("  %02X %02X %02X %02X "), de.ex, de.s1, de.s2, de.rc);
  Serial.print(buf);
  for (uint8_t i = 0; i < 16; i++) {
    sprintf_P(buf, PSTR(" %02X"), de.al[i]);
    Serial.print(buf);
  }
  Serial.print("\r\n");
#endif
  // Write the directory entry into RAM (at the DMA address)
  ram->write(ramDMA, de.buf, 32);
}

// Autoselect the drive.
// 0 or ?: use the current drive
// 1-16  : substract 1
bool BDOS::selDrive(uint8_t drive) {
  uint8_t result = false;
  // Check if the drive is specified
  if (!drive || drive == '?')
    // Use the current drive
    drive = cDrive;
  else
    // Use A=0, B=1, ...
    drive--;
  // Check if the drive exists (as directory on SD)
  result = drv->selDrive(drive);
  if (not result)
    // Return and display error
    bdosError(2);
  // Return the status
  return result;
}

// Convert FCB to host file name
bool BDOS::fcb2fname(FCB_t fcb, char* fname) {
  bool unique = true;
  // TODO Use custom base paths
  // Start with the drive letter
  if (fcb.dr && fcb.dr != '?')
    // The drive is specified and non ambiguous
    *(fname++) = (fcb.dr - 1) + 'A';
  else
    // Use the current drive
    *(fname++) = cDrive + 'A';
  // Path separator
  *(fname++) = '/';
  // User number, converted to HEX
  *(fname++) = toHEX(cUser);
  // Path separator
  *(fname++) = '/';
  // Check if the file name is ambiguous or not
  if (fcb.dr != '?') {
    // File name
    for (uint8_t i = 0; i < 8; i++) {
      char c = fcb.fn[i] & 0x7F;
      if (c > ' ')
        *(fname++) = toupper(c);
      if (c == '?')
        unique = false;
    }
    // File type
    for (uint8_t i = 0; i < 3; i++) {
      char c = fcb.tp[i] & 0x7F;
      if (c > ' ') {
        // Only add the dot if there's an extension
        if (i == 0)
          *(fname++) = '.';
        *(fname++) = toupper(c);
      }
      if (c == '?')
        unique = false;
    }
  }
  else {
    // The file name is ambiguous, fill with '?'
    for (uint8_t i = 0; i < 8; i++)
      *(fname++) = '?';
    *(fname++) = '.';
    for (uint8_t i = 0; i < 3; i++)
      *(fname++) = '?';
    unique = false;
  }
  // End with zero
  *(fname++) = '\0';
  // Return if it is ambiguous or not
  return unique;
}

// Convert FCB to CP/M file name (A0FILE    TXT)
bool BDOS::fcb2cname(FCB_t fcb, char* fname) {
  bool unique = true;
  // Start with the drive letter
  if (fcb.dr && fcb.dr != '?')
    // The drive is specified and non ambiguous
    *(fname++) = (fcb.dr - 1) + 'A';
  else
    // Use the current drive
    *(fname++) = cDrive + 'A';
  // User number, converted to HEX
  *(fname++) = toHEX(cUser);
  // Check if the file name is ambiguous or not
  if (fcb.dr != '?') {
    // File name
    for (uint8_t i = 0; i < 8 + 3; i++) {
      char c = *(fcb.fn + i) & 0x7F;
      *(fname++) = toupper(c);
      if (c == '?')
        unique = false;
    }
  }
  else {
    // The file name is ambiguous, fill with '?'
    for (uint8_t i = 0; i < 8 + 3; i++)
      *(fname++) = '?';
  }
  // End with zero
  *(fname++) = '\0';
  // Return if it is ambiguous or not
  return unique;
}
