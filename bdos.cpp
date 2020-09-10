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
      addrDMA = TBUFF;      // Setup default DMA address
      //HL = _CheckSUB(); // TODO Checks if there's a $$$.SUB on the boot disk
      cpu.regHL(0x0000);
      break;

    case 0x0E:  // SETDSK
      // Function to set the active disk number.
      if (cDrive != cpu.regE())
        logVector = logVector | (1 << cpu.regE());
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
      addrDMA = cpu.regDE();
      cpu.regBC(addrDMA);
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
