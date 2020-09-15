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

BDOS::BDOS(I8080 *cpu, RAM *ram, DRIVE *drv, BIOS *bios): cpu(cpu), ram(ram), drv(drv), bios(bios) {
}

BDOS::~BDOS() {
}

void BDOS::init() {
  // Serial number, 6 bytes
  uint8_t cpmSerialNo[] = {0, 22, 0, 0, 0, 0};
  ram->write(BDOSCODE, cpmSerialNo, 6);

  // BDOS entry (FBASE)
  ram->writeByte(BDOSCODE + 6, 0xC3);          // JP
  ram->writeWord(BDOSCODE + 7, BDOSCODE + 0x0100);

  // BDOS dispatcher (FBASE1)
  ram->writeByte(BDOSCODE + 0x0100,     0xDB); // IN A
  ram->writeByte(BDOSCODE + 0x0100 + 1, 0x00);
  ram->writeByte(BDOSCODE + 0x0100 + 2, 0xC9); // RET

#ifdef DEBUG
  ram->hexdump(BDOSCODE, 16);
  ram->hexdump(BDOSCODE + 0x0100, 16);
#endif
}

void BDOS::bdosError(uint8_t err) {
  Serial.print("Bdos Err On ");
  Serial.print(cDrive + 'A');
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
  // Always reboot on these errors.
  bios->wboot();
}


void BDOS::call(uint16_t port) {
  uint16_t  w;
  uint8_t   b, count;
  char      c;

  // HL is reset by the BDOS
  cpu->regHL(0x0000);
  //cpu->trace();

  switch (cpu->regC()) {
    case 0x00:  // WBOOT
      // System reset
      bios->wboot();
      break;

    case 0x01:  // GETCON
      // Function to get a character from the console device.
      c = bios->conin();
      cpu->regHL(c);
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
      cpu->regHL(bios->reader());
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
          cpu->regHL(bios->conin());
        else
          cpu->regHL(0x00);
      }
      else {
        // TODO outcon()
        bios->conout(cpu->regE());
      }
      break;

    case 0x07:  // GETIOB
      // Function to return the i/o byte.
      cpu->regHL(ram->readByte(IOBYTE));
      break;

    case 0x08:  // SETIOB
      // Function to set the i/o byte.
      ram->writeByte(IOBYTE, cpu->regE());
      break;

    case 0x09:  // PRTSTR
      // Function to print the character string pointed to by (DE)
      // on the console device. The string ends with a '$'.
      w = cpu->regDE();
      b = ram->readByte(w++);
      while (b != '$') {
        bios->conout(b);
        b = ram->readByte(w++);
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
      b = ram->readByte(w);
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
            bios->conout(ram->readByte(w + j));
        }
        else if (c == 21) {
          // ^U
          Serial.write("#\r\n   ");
          w = cpu->regDE();
          b = ram->readByte(w);
          w++;
          count = 0;
        }
        else if (c == 24) {
          // ^X
          for (uint8_t j = 0; j < count; ++j)
            Serial.write("\b \b");
          w = cpu->regDE();
          b = ram->readByte(w);
          w++;
          count = 0;
        }
        else if (c < 0x20 || c > 0x7E)
          // Invalid character
          continue;
        bios->conout(c);
        ++count;
        ram->writeByte(w + count, c);
        // Reached the expected count
        if (count == b)
          break;
      }
      // Save the number of characters read
      ram->writeByte(w, count);
      // Gives a visual feedback that read ended
      bios->conout('\r');
      break;

    case 0x0B:  // GETCSTS
      // Function to interigate the console device.
      cpu->regHL(bios->consts());
      break;

    case 0x0C:  // GETVER
      // Function to return the current cp/m version number.
      // Return: B=H=system type, A=L=version number
      cpu->regHL(0x0022);
      break;

    case 0x0D:  // RSTDSK
      // Function to reset the disk system.
      cDrive = 0;           // Select drive 'A'
      rwoVector = 0x0000;   // Clear write protect vector
      logVector = 0x0001;   // Reset log in vector
      ramDMA = TBUFF;       // Setup default DMA address
      //HL = _CheckSUB();   // TODO Check if there's a $$$.SUB on the boot disk
      cpu->regHL(0x0000);
      break;

    case 0x0E:  // SETDSK
      // Function to set the active disk number.
      tDrive = cpu->regE() & 0x0F;
      if (cDrive != tDrive)
        if (drv->selDrive(tDrive)) {
          cDrive = tDrive;
          logVector = logVector | (1 << cDrive);
        }
      break;

    case 0x0F:  // OPENFIL
      // Function to open a specified file.
      result = 0xFF;
      // Read the FCB from RAM
      readFCB();
      // Clear S2
      fcb.s2 = 0x00;
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Get the filename on SD card
        fcb2fname(fcb, fName);
        // Try to open it
        if (drv->open(fName)) {
          // Get the file size (in blocks)
          fSize = drv->fileSize(fName) / BlkSZ;
          // Reset S1 and S2
          fcb.s1 = 0x00;
          fcb.s2 = 0x80; // set unmodified flag
          // Set the record count
          fcb.rc = fSize > MaxRC ? MaxRC : (uint8_t)fSize;
          // Clean up allocation
          for (uint8_t i = 0; i < 16; i++)
            fcb.al[i] = 0x00;
          // Success
          result = 0x00;
        }
      }
      // Write the FCB back into RAM
      writeFCB();
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x10:  // CLOSEFIL
      // Function to close a specified file.
      result = 0xFF;
      // Read the FCB from RAM
      readFCB();
      // Clear S2
      fcb.s2 = 0x00;
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Check if the file has been modifed
        if (!(fcb.s2 & 0x80)) {
          // Check if the drive is write protected
          if (!(rwoVector & (1 << fcb.dr))) {
            // Get the filename on SD card
            fcb2fname(fcb, fName);
            // Check if this is '$$$.SUB'
            if (ramFCB == BatchFCB)
              // Truncate it to fcb.rc CP/M records so SUBMIT.COM can work
              drv->truncate(fName, fcb.rc);
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
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x11:  // GETFST
      // Function to return the first occurence of a specified file name.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Keep the drive letter, user hexcode and CP/M filename
        // (pattern, actually) in fName: 'A0???????????'
        fName[FNDRIVE] = fcb.dr + 'A';
        fName[FNUSER] = toHEX(cUser);
        memcpy(fName + FNFILE, fcb.fn, 11);
        fName[FNZERO] = '\0';
        // Check if we need to find for all users
        fAllUsers = fcb.dr == '?';
        if (fAllUsers)
          result = drv->findFirst(fName, fSize);
        else
          result = drv->findFirst(fName, fSize);
        if (result == 0x00)
          // On return, fName contains the drive letter,
          // hex user code and the CP/M name.
          // Create a directory entry
          dirEntry(fName + FNFILE, frHEX(fName[1]), fSize);
      }
      // Reset direntry file size counters
      fRecs = 0;
      fExts = 0;
      fExtU = 0;
      // Write the FCB back into RAM
      writeFCB();
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x12:  // GETNXT
      // Function to return the next occurence of a file name.
      result = 0xFF;
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        if (fAllUsers)
          result = drv->findNext(fName, fSize);
        else
          result = drv->findNext(fName, fSize);
        if (result == 0x00)
          // On return, fName contains the drive letter,
          // hex user code and CP/M name
          // Create a directory entry
          dirEntry(fName + FNFILE, frHEX(fName[1]), fSize);
      }
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x13:  // DELFILE
      // Function to delete a file by name.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Keep the drive letter, user hexcode and CP/M filename
          // (pattern, actually) in fName: 'A0???????????'
          fName[FNDRIVE] = fcb.dr + 'A';
          fName[FNUSER] = toHEX(cUser);
          memcpy(fName + FNFILE, fcb.fn, 11);
          fName[FNZERO] = '\0';
          result = drv->findFirst(fName, fSize);
          // Now fName will contain individual matching files
          while (result != 0xFF) {
            // Delete it, the full file name starts at FNHOST
            drv->remove(fName + FNHOST);
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
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x14:  // READSEQ
      // Function to execute a sequential read of the specified record number.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Compute the file seek position
      fPos = ((fcb.s2 & MaxS2) * BlkS2 * BlkSZ) +
             (fcb.ex * BlkEX * BlkSZ) +
             (fcb.cr * BlkSZ);
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Get the filename on SD card
        fcb2fname(fcb, fName);
        // Read one block
        result = drv->read(ramDMA, fName, fPos);
        // Check the result
        if (!result) {
          // Read succeeded, adjust FCB
          fcb.cr++;
          if (fcb.cr > MaxCR) {
            fcb.cr = 1;
            fcb.ex++;
          }
          if (fcb.ex > MaxEX) {
            fcb.ex = 0;
            fcb.s2++;
          }
          if ((fcb.s2 & 0x7F) > MaxS2)
            // TODO Not sure what to do
            result = 0xfe;
        }
      }
      // Write the FCB back into RAM
      writeFCB();
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x15:  // WRTSEQ
      // Function to write the net sequential record.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Compute the file seek position
      fPos = ((fcb.s2 & MaxS2) * BlkS2 * BlkSZ) +
             (fcb.ex * BlkEX * BlkSZ) +
             (fcb.cr * BlkSZ);
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Get the filename on SD card
          fcb2fname(fcb, fName);
          // Read one block
          result = drv->write(ramDMA, fName, fPos);
          // Check the result
          if (!result) {
            // Reset unmodified flag
            fcb.s2 &= 0x7F;
            // Read succeeded, adjust FCB
            fcb.cr++;
            if (fcb.cr > MaxCR) {
              fcb.cr = 1;
              fcb.ex++;
            }
            if (fcb.ex > MaxEX) {
              fcb.ex = 0;
              fcb.s2++;
            }
            if ((fcb.s2 & 0x7F) > MaxS2)
              // TODO Not sure what to do
              result = 0xfe;
          }
        }
        else
          // Return error 4 if write protected
          bdosError(4);
      }
      // Write the FCB back into RAM
      writeFCB();
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x16:  // FCREATE
      // Create a file function.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Get the filename on SD card
          fcb2fname(fcb, fName);
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
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x17:  // RENFILE
      // Function to rename a file.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          uint16_t ramNewFCB = ramFCB + 16;
          // Prevents rename from moving files among drives
          ram->writeByte(ramNewFCB, ram->readByte(ramFCB));
          // Create the new FCB object
          FCB_t newfcb;
          ram->read(ramNewFCB, newfcb.buf, 36);
          // Get the filename on SD card
          fcb2fname(fcb, fName);
          // Get the new filename on SD card
          char newName[128];
          fcb2fname(newfcb, newName);
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
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x18:  // GETLOG
      // Function to return the login vector.
      cpu->regHL(logVector);
      break;

    case 0x19:  // GETCRNT
      // Function to return the current disk assignment.
      cpu->regHL(cDrive);
      break;

    case 0x1A:  // PUTDMA
      // Function to set the dma address.
      ramDMA = cpu->regDE();
      cpu->regBC(ramDMA);
      bios->setdma();
      break;

    case 0x1B:  // GETALOC
      // Function to return the allocation vector.
      cpu->regHL(alcVector);
      break;

    case 0x1C:  // WRTPRTD
      // Function to write protect the current disk.
      logVector = logVector | (1 << cDrive);
      break;

    case 0x1D:  // GETROV
      // Function to return the read-only status vector.
      cpu->regHL(rwoVector);
      break;

    case 0x1E:  // SETATTR
      // Function to set the file attributes (read-only, system).
      break;

    case 0x1F:  // GETPARM
      // Function to return the address of the disk parameter block for the current drive.
      cpu->regHL(DPBADDR);
      break;

    case 0x20:  // GETUSER
      // Function to get or set the user number. If (E) was (FF)
      // then this is a request to return the current user number.
      // Else set the user number from (E).
      if (cpu->regE() == 0xFF)
        cpu->regHL(cUser & 0x000F);
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
      fPos = fRec * BlkSZ;
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Get the filename on SD card
        fcb2fname(fcb, fName);
        // Read one block
        result = drv->read(ramDMA, fName, fPos);
        // Check the result
        if (result == 0 or result == 1 or result == 4) {
          // Adjust FCB unless error #6 (seek past 8MB)
          fcb.cr = fRec & 0x7F;
          fcb.ex = (fRec >> 7) & 0x1f;
          fcb.s2 = ((fRec >> 12) & MaxS2) | (fcb.s2 & 0x80);
        }
      }
      // Write the FCB back into RAM
      writeFCB();
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x22:  // WTRANDOM
      // Function to write a random record to a file.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Compute the file record and seek position
      fRec = fcb.r2 * 0x010000UL + fcb.r1 * 0x0100UL + fcb.r0;
      fPos = fRec * BlkSZ;
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Get the filename on SD card
          fcb2fname(fcb, fName);
          // Write one block
          result = drv->write(ramDMA, fName, fPos);
          // Check the result
          if (!result) {
            // Reset unmodified flag
            fcb.s2 &= 0x7F;
            // Read succeeded, adjust FCB
            fcb.cr = fRec & 0x7F;
            fcb.ex = (fRec >> 7) & 0x1f;
            fcb.s2 = (fRec >> 12) & MaxS2;  // resets unmodified flag
          }
        }
        else
          // Return error 4 if write protected
          bdosError(4);
      }
      // Write the FCB back into RAM
      writeFCB();
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x23:  // FILESIZE
      // Function to compute the size of a random file.
      result = 0xFF;
      // Read the FCB from RAM
      readFCB();
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // TODO Find the file
        // Get the filename on SD card
        fcb2fname(fcb, fName);
        // Get the file size (in blocks)
        fSize = drv->fileSize(fName) / BlkSZ;
        if (fSize != -1) {
          fcb.r0 = count & 0xFF;
          fcb.r1 = (count >>  8) & 0xFF;
          fcb.r2 = (count >> 16) & 0xFF;
          // Success
          result = 0x00;
        }
      }
      // Write the FCB back into RAM
      writeFCB();
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x24:  // SETRAN
      // Function to return the random record position of a given
      // file which has been read in sequential mode up to now.
      result = 0x00;
      // Read the FCB from RAM
      readFCB();
      // Get the last used record
      fSize  = fcb.cr & 0x7F;
      fSize += (fcb.ex & 0x1F) << 7;
      fSize += (fcb.s2 & MaxS2) << 12;
      // Compute random position and store it into fcb
      fcb.r0 = fSize & 0xFF;
      fcb.r1 = (fSize >>  8) & 0xFF;
      fcb.r2 = (fSize >> 16) & 0xFF;
      // Write the FCB back into RAM
      writeFCB();
      // Return the result in HL
      cpu->regHL(result);
      break;

    case 0x25:  // LOGOFF
      // This allows a program to log off any drives.
      break;

    case 0x26:  // DRVACCESS
      // Locks one or more disc drives.
      break;

    case 0x27:  // DRVFREE
      // Releases locks on disc drives.
      break;

    case 0x28:  // WTSPECL
      // For the case where we are writing to unused disk space, this
      // space will be zeroed out first.
      result = 0xFF;
      // Read the FCB from RAM, address in DE
      readFCB();
      // Compute the file record and seek position
      fRec = fcb.r2 * 0x010000UL + fcb.r1 * 0x0100UL + fcb.r0;
      fPos = fRec * BlkSZ;
      // Select the drive
      if (drv->selDrive(fcb.dr)) {
        // Check if the drive is write protected
        if (!(rwoVector & (1 << fcb.dr))) {
          // Get the filename on SD card
          fcb2fname(fcb, fName);
          // Write one block
          result = drv->write(ramDMA, fName, fPos);
          // Check the result
          if (!result) {
            // Reset unmodified flag
            fcb.s2 &= 0x7F;
            // Read succeeded, adjust FCB
            fcb.cr = fRec & 0x7F;
            fcb.ex = (fRec >> 7) & 0x1f;
            fcb.s2 = (fRec >> 12) & MaxS2;  // resets unmodified flag
          }
        }
        else
          // Return error 4 if write protected
          bdosError(4);
      }
      // Write the FCB back into RAM
      writeFCB();
      // Return the result in HL
      cpu->regHL(result);
      break;

    default:
#ifdef DEBUG
      // Show unimplemented BDOS calls only when debugging
      Serial.print("\r\nUnimplemented BDOS call 0x");
      Serial.print(cpu->regC(), HEX);
      Serial.print("\r\n");
      cpu->trace();
#endif
      break;
  }

  // CP/M BDOS does this before returning
  cpu->regB((uint8_t)cpu->regH());
  cpu->regA((uint8_t)cpu->regL());
  // C ends up equal to E
  cpu->regC((uint8_t)cpu->regE());
}




// Read the FCB from RAM, register DE has the address.
// Store the address in ramFCB and the FCB in fcb
void BDOS::readFCB() {
  // Get the FCB address
  ramFCB = cpu->regDE();
  // Create the FCB object
  ram->read(ramFCB, fcb.buf, 36);
  // Show FCB
  //showFCB();
}

// Write the FCB back into RAM.
// The address is already in ramFCB and the FCB in fcb
void BDOS::writeFCB() {
  // Write the FCB back into RAM
  ram->write(ramFCB, fcb.buf, 36);
}

// Debug FCB
void BDOS::showFCB() {
  char buf[80];
  Serial.print("\r\nDR FN       TP   EX S1 S2 RC CR R0 R1 R2");
  Serial.print("\r\n ");
  Serial.print((char)(fcb.dr + 'A'));
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
  // Create the directory entry object
  DIR_t de;
  // Zero it out
  memset(de.buf, 0, 32);
  // Store the user id
  if (fAllUsers)
    // FIXME use the proper user id currFindUser; // set user code for return
    de.uu = uid;
  else
    de.uu = uid;
  // Copy the file name and type from cname (8+3 bytes)
  memcpy(de.fn, cname, 11);

  // Round the file size to the next multiple of blocks
  if (fsize & (BlkSZ - 1))
    fsize = fsize | (BlkSZ - 1);
  // File records
  fRecs = fsize / BlkSZ;
  // File extents
  fExts = fRecs / BlkEX + ((fRecs & (BlkEX - 1)) ? 1 : 0);
  // File extents used (start with zero)
  fExtU = 0;

  uint8_t firstFreeAllocBlock = 25;

  // Check if the file fits in a single directory entry
  if (fExts <= bios->dpb.exm + 1) {
    // Yes, compute the data
    if (fExts > 0) {
      de.ex = (fExts - 1 + fExtU) % (MaxEX + 1);
      de.s2 = (fExts - 1 + fExtU) / (MaxEX + 1);
      de.rc = fRecs - (BlkEX * (fExts - 1));
    }
    blocks = (fRecs >> bios->dpb.bsh) + ((fRecs & bios->dpb.blm) ? 1 : 0);
    fRecs = 0;
    fExts = 0;
    fExtU = 0;
  }
  else {
    // No, max out the directory entry
    de.ex = (bios->dpb.exm + fExtU) % (MaxEX + 1);
    de.s2 = (bios->dpb.exm + fExtU) / (MaxEX + 1);
    de.rc = BlkEX;
    blocks = (bios->dpb.dsm + 1) < 256 ? 16 : 8;
    // Update remaining records and extents for next call
    fRecs -= (bios->dpb.exm + 1) * BlkEX;
    fExts -= (bios->dpb.exm + 1);
    fExtU += (bios->dpb.exm + 1);
  }
  // Phoney up an appropriate number of allocation blocks
  if (bios->dpb.dsm <= 255)
    for (i = 0; i < blocks; i++)
      de.al[i] = (uint8_t)firstFreeAllocBlock++;
  else
    for (i = 0; i < 2 * blocks; i += 2) {
      de.al[i] = firstFreeAllocBlock & 0xFF;
      de.al[i + 1] = firstFreeAllocBlock >> 8;
      firstFreeAllocBlock++;
    }
  // Write the directory entry into RAM (at the DMA address)
  ram->write(ramDMA, de.buf, 32);
}

uint8_t BDOS::selDrive(uint8_t drive) {
  uint8_t result = 0xFF;
  // Check if the drive is specified
  if (!drive || drive == '?')
    // Use the current drive
    drive = cDrive;
  // Check if the drive exists (as directory on SD)
  if (drv->selDrive(drive)) {
    // Set the drive log in vector
    logVector = logVector | (1 << drive);
    result = 0x00;
  } else
    // Return and display error
    bdosError(2);
  // Return the status
  return (result);
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
  } else {
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
