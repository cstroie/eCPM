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

BDOS::BDOS(I8080 cpu, RAM ram) {
  this->cpu = cpu;
  this->ram = ram;
}

BDOS::~BDOS() {
}

void BDOS::init() {
  // Serial number, 6 bytes
  uint8_t cpmSerialNo[] = {0, 22, 0, 0, 0, 0};
  ram.write(BDOSCODE, cpmSerialNo, 6);

  // BDOS entry (FBASE)
  ram.writeByte(BDOSCODE + 6, 0xC3);          // JP
  ram.writeWord(BDOSCODE + 7, BDOSCODE + 0x0100);

  // BDOS dispatcher (FBASE1)
  ram.writeByte(BDOSCODE + 0x0100,     0xDB); // IN A
  ram.writeByte(BDOSCODE + 0x0100 + 1, 0x00);
  ram.writeByte(BDOSCODE + 0x0100 + 2, 0xC9); // RET

#ifdef DEBUG
  ram.hexdump(BDOSCODE, 16);
  ram.hexdump(BDOSCODE + 0x0100, 16);
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
  while (!Serial.available()) { }
  // Always reboot on these errors.
  cpu.jump(WBOOT);
}


void BDOS::call(uint16_t port) {
  uint16_t  w;
  uint8_t   b, count;
  char      c;

  // HL is reset by the BDOS
  cpu.regHL(0x0000);

  switch (cpu.regC()) {
    case 0x00:  // WBOOT
      // System reset
      cpu.jump(WBOOT);
      break;

    case 0x01:  // GETCON
      // Function to get a character from the console device.
      //cpu.jump(CONIN);
      while (!Serial.available()) { }
      c = Serial.read();
      cpu.regHL(c);
      if (c == 0x0A or c == 0x0D or c == 0x09 or c == 0x08 or c >= ' ')
        cpu.regE(c);
      else
        // A regular control char so don't echo
        break;

    case 0x02:  // OUTCON
      // Function to output (E) to the console device and expand tabs if necessary.
      //cpu.jump(CONOUT);
      Serial.write((char)(cpu.regE() & 0x7F));
      break;

    case 0x03:  // GETRDR
      // Function to get a character from the tape reader device.
      //cpu.jump(READER);
      cpu.regHL(0x1A);
      break;

    case 0x04:  // PUNCH
      // Auxiliary (Punch) output
      cpu.jump(PUNCH);
      break;

    case 0x05:  // LIST
      // Printer output
      cpu.jump(LIST);
      break;

    case 0x06:  // DIRCIO
      // Function to perform direct console i/o. If (C) contains (FF)
      // then this is an input request. Otherwise we are to output (C).
      if (cpu.regE() == 0xFF)
        cpu.regHL(Serial.available() ? Serial.read() : 0x00);
      else
        Serial.write((char)(cpu.regE() & 0x7F));
      break;

    case 0x07:  // GETIOB
      // Function to return the i/o byte.
      cpu.regHL(ram.readByte(IOBYTE));
      break;

    case 0x08:  // SETIOB
      // Function to set the i/o byte.
      ram.writeByte(IOBYTE, cpu.regE());
      break;

    case 0x09:  // PRTSTR
      // Function to print the character string pointed to by (DE)
      // on the console device. The string ends with a '$'.
      w = cpu.regDE();
      b = ram.readByte(w++);
      while (b != '$') {
        Serial.print((char)(b & 0x7F));
        b = ram.readByte(w++);
      }
      cpu.regDE(w);
      break;

    case 0x0A:  // RDBUFF
      // Function to execute a buffered read.
      // DE = Address of buffer
      // Read (DE) bytes from the console
      // Return: A = Number of chars read
      // (DE) = First char
      // Get the number of characters to read
      w = cpu.regDE();
      b = ram.readByte(w);
      w++;
      count = 0;
      // Very simple line input
      while (b) {
        while (!Serial.available()) {}
        c = Serial.read();
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
          Serial.write("#\r\n  ");
          for (uint8_t j = 1; j <= count; ++j)
            Serial.write((char)ram.readByte(w + j));
        }
        else if (c == 21) {
          // ^U
          Serial.write("#\r\n  ");
          w = cpu.regDE();
          b = ram.readByte(w);
          w++;
          count = 0;
        }
        else if (c == 24) {
          // ^X
          for (uint8_t j = 0; j < count; ++j)
            Serial.write("\b \b");
          w = cpu.regDE();
          b = ram.readByte(w);
          w++;
          count = 0;
        }
        else if (c < 0x20 || c > 0x7E)
          // Invalid character
          continue;
        Serial.write((char)(c & 0x7F));
        ++count;
        ram.writeByte(w + count, c);
        // Reached the expected count
        if (count == b)
          break;
      }
      // Save the number of characters read
      ram.writeByte(w, count);
      // Gives a visual feedback that read ended
      Serial.write('\r');
      break;

    case 0x0B:  // GETCSTS
      // Function to interigate the console device.
      cpu.regHL(Serial.available() ? 0xFF : 0x00);
      break;

    case 0x0C:  // GETVER
      // Function to return the current cp/m version number.
      // Return: B=H=system type, A=L=version number
      cpu.regHL(0x0022);
      break;

    case 0x0D:  // RSTDSK
      // Function to reset the disk system.
      cDrive = 0;           // Select drive 'A'
      rwoVector = 0x0000;   // Clear write protect vector
      logVector = 0x0001;   // Reset log in vector
      ramDMA = TBUFF;      // Setup default DMA address
      //HL = _CheckSUB(); // TODO Checks if there's a $$$.SUB on the boot disk
      cpu.regHL(0x0000);
      break;

    case 0x0E:  // SETDSK
      // Function to set the active disk number.
      tDrive = cpu.regE() & 0x0F;
      if (cDrive != tDrive) {
        if (sdSelect(tDrive)) {
          cDrive = tDrive;
          logVector = logVector | (1 << cDrive);
        }
      }
      break;

    case 0x0F:  // OPENFIL
      // Function to open a specified file.
      result = 0xFF;
      // Get the FCB address
      ramFCB = cpu.regDE();
      // Create the FCB object
      ram.read(ramFCB, fcb.buf, 36);
      // Clear S2
      fcb.s2 = 0x00;
      // Select the drive
      if (sdSelect(fcb.dr)) {
        // TODO Find the file
        // Get the filename on SD card
        fcb2fname(fcb, fName);
        //Serial.println(fName);
        // Get the file size (in blocks)
        fSize = sdFileSize(fName) / BlkSZ;
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
      // Return the result in HL
      cpu.regHL(result);
      break;



    case 0x14:  // READSEQ
      // Read a file sequentially
      ramFCB = cpu.regDE();
      // Create the FCB object
      ram.read(ramFCB, fcb.buf, 36);
      // Compute the file seek position
      fPos = ((fcb.s2 & MaxS2) * BlkS2 * BlkSZ) +
             (fcb.ex * BlkEX * BlkSZ) +
             (fcb.cr * BlkSZ);
      // Select the drive
      if (sdSelect(fcb.dr)) {
        // Get the filename on SD card
        fcb2fname(fcb, fName);
        // Read one block
        result = sdSeqRead(fName, fPos);
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
      ram.write(ramFCB, fcb.buf, 36);
      // Return the result in HL
      cpu.regHL(result);
      break;



    case 0x18:  // GETLOG
      // Function to return the login vector.
      cpu.regHL(logVector);
      break;

    case 0x19:  // GETCRNT
      // Function to return the current disk assignment.
      cpu.regHL(0x00);
      break;

    case 0x1A:  // PUTDMA
      // Function to set the dma address.
      ramDMA = cpu.regDE();
      cpu.regBC(ramDMA);
      cpu.jump(SETDMA);
      break;



    case 0x1C:  // WRTPRTD
      // Function to write protect the current disk.
      logVector = logVector | (1 << cDrive);
      break;

    case 0x1D:  // GETROV
      // Function to return the read-only status vector.
      cpu.regHL(rwoVector);
      break;

    case 0x1E:  // SETATTR
      // Function to set the file attributes (read-only, system).
      break;

    case 0x1F:  // GETPARM
      // Function to return the address of the disk parameter block for the current drive.
      cpu.regHL(DPBADDR);
      break;

    case 0x20:  // GETUSER
      // Function to get or set the user number. If (E) was (FF)
      // then this is a request to return the current user number.
      // Else set the user number from (E).
      if (cpu.regE() == 0xFF)
        cpu.regHL(cUser & 0x000F);
      else
        // TODO Make user dir
        cUser = cpu.regE() & 0x0F;
      break;


    default:
      /*
        Unimplemented calls get listed
      */
#ifdef DEBUG
      // Show unimplemented BDOS calls only when debugging
      Serial.print("\r\nUnimplemented BDOS call 0x");
      Serial.print(cpu.regC(), 16);
      Serial.print("\r\n");
      cpu.trace();
#endif
      break;
  }

  // CP/M BDOS does this before returning
  cpu.regB((uint8_t)cpu.regH());
  cpu.regA((uint8_t)cpu.regL());
  // C ends up equal to E
  cpu.regC((uint8_t)cpu.regE());
}


uint8_t BDOS::selDrive(uint8_t drive) {
  uint8_t result = 0xFF;
  // Check if the drive is specified
  if (!drive || drive == '?')
    // Use the current drive
    drive = cDrive;
  // Check if the drive exists (as directory on SD)
  if (sdSelect(drive)) {
    // Set the drive log in vector
    logVector = logVector | (1 << drive);
    result = 0x00;
  } else
    // Return and display error
    bdosError(2);
  // Return the status
  return (result);
}

bool BDOS::fcb2fname(FCB_t fcb, char* fname) {
  bool unique = true;
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
  *(fname++) = toupper('0' + cUser + (cUser > 9 ? 7 : 0));
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

bool BDOS::sdSelect(uint8_t drive) {
  bool result = false;
  char disk[] = "A";
  // Adjust the drive letter
  disk[0] += drive;
  // Check for directory existence
  digitalWrite(BUILTIN_LED, HIGH ^ LEDinv);
  if (File f = SD.open((char*)disk, FILE_READ)) {
    result = f.isDirectory();
    f.close();
  }
  digitalWrite(BUILTIN_LED, LOW ^ LEDinv);
  return result;
}

uint32_t BDOS::sdFileSize(char* fname) {
  uint32_t len = -1;
  // Open the file and get the size
  digitalWrite(BUILTIN_LED, HIGH ^ LEDinv);
  if (File f = SD.open((char*)fname, FILE_READ)) {
    len = f.size();
    f.close();
  }
  digitalWrite(BUILTIN_LED, LOW ^ LEDinv);
  return len;
}

uint8_t BDOS::sdSeqRead(char* fname, uint32_t fpos) {
  uint8_t result = 0xFF;
  uint8_t buf[BlkSZ];

  digitalWrite(BUILTIN_LED, HIGH ^ LEDinv);
  File f = SD.open((char*)fname, FILE_READ);
  if (f) {
    if (f.seek(fpos)) {
      // Clear the buffer
      memset(buf, 0x1a, BlkSZ);
      // Read from file
      if (f.read(&buf[0], BlkSZ)) {
        // Write into RAM
        ram.write(ramDMA, buf, BlkSZ);
        result = 0x00;
      }
      else
        // Read error
        result = 0x01;
    } else
      // Seek error
      result = 0x01;
    // Close the file
    // TODO Optimize
    f.close();
  } else
    // Open error
    result = 0x10;

  digitalWrite(BUILTIN_LED, LOW ^ LEDinv);
  return result;
}
