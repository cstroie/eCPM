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
  ram.writeByte(BDOSCODE + 6, JP);
  ram.writeWord(BDOSCODE + 7, BDOSCODE + 0x0100);

  //  Patch in a JP to the BDOS entry at location 0x0005
  ram.writeByte(ENTRY, JP);
  ram.writeWord(ENTRY + 1, BDOSCODE + 0x06);

  // IOBYTE - Points to Console */
  ram.writeByte(IOBYTE, 0x3D);
  // Current drive/user - A:/0 */
  ram.writeByte(TDRIVE, 0x00);

  // BDOS dispatcher (FBASE1)
  ram.writeByte(BDOSCODE + 0x0100, INa);
  ram.writeByte(BDOSCODE + 0x0100 + 1, 0x00);
  ram.writeByte(BDOSCODE + 0x0100 + 2, RET);

#ifdef DEBUG
  ram.hexdump(BDOSCODE, 16);
  ram.hexdump(BDOSCODE + 0x0100, 16);
#endif
}

void BDOS::call(uint16_t port) {
  uint8_t  b, count;
  char c;
  uint16_t w;

  // HL is reset by the BDOS
  cpu.regHL(0x0000);

  switch (cpu.regC()) {
    case BDOS_WBOOT:
      // WBOOT: System reset
      // Doesn't return. Reloads CP/M.
      // Same as call to "BOOT"
      cpu.state = 0;
      break;

    case BDOS_GETCON:
      // GETCON: Console input
      // Get a char from the console
      // Return: A=Char
      while (!Serial.available()) {}
      c = Serial.read();
      Serial.write(c);
      cpu.regHL(c);
      break;

    case BDOS_OUTCON:
      // OUTCON: Console output
      // E = Char
      // Send the char in E to the console
      Serial.write((char)(cpu.regE() & 0x7F));
      break;

    case BDOS_GETRDR:
      // GETRDR: Auxiliary (Reader) input
      // Returns: A=Char
      cpu.regHL(0x1A);
      break;

    case BDOS_PUNCH:
      // PUNCH: Auxiliary (Punch) output
      break;

    case BDOS_LIST:
      // LIST: Printer output
      break;

    case BDOS_DIRCIO:
      // DIRCIO: Direct console IO
      // E = 0xFF : Checks for char available and returns it, or 0x00 if none (read)
      // E = char : Outputs char (write)
      // Return: A=Char or 0x00 (on read)
      if (cpu.regE() == 0xFF)
        cpu.regHL(Serial.available() ? Serial.read() : 0x00);
      else
        Serial.write((char)(cpu.regE() & 0x7F));
      break;

    case BDOS_GETIOB:
      // GETIOB: Get IOBYTE
      // Get the system IOBYTE
      // Return: A = IOBYTE
      cpu.regHL(ram.readByte(IOBYTE));
      break;

    case BDOS_SETIOB: // TODO not C ?
      // SETIOB: Set IOBYTE
      // E = IOBYTE
      // Set the system IOBYTE to E
      ram.writeByte(IOBYTE, cpu.regE());
      break;

    case BDOS_PRTSTR:
      // PRTSTR: Output string
      // DE = Address of string
      // Send the $ terminated string pointed by (DE) to the screen
      w = cpu.regDE();
      b = ram.readByte(w++);
      while (b != '$') {
        Serial.print((char)(b & 0x7F));
        b = ram.readByte(w++);
      }
      cpu.regDE(w);
      break;

    case BDOS_RDBUFF:
      // RDBUFF: Buffered input
      // DE = Address of buffer
      // Read (DE) bytes from the console
      // Return: A = Number os chars read
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

    case BDOS_GETCSTS:
      // GETCSTS: Get console status
      // Return: A=0x00 or 0xFF
      cpu.regHL(Serial.available() ? 0xff : 0x00);
      break;

    case BDOS_GETVER:
      // GETVER: Get version number
      // Return: B=H=system type, A=L=version number
      cpu.regHL(0x22);
      break;

    case BDOS_GETUSER:
      // GETUSER: Get/Set user code
      // Function to get or set the user number. If (E) was (FF)
      // then this is a request to return the current user number.
      // Else set the user number from (E).
      if (cpu.regE() == 0xFF)
        cpu.regHL(ram.readByte(TDRIVE) & 0x0F);
      else
        // _SetUser(DE);
        // Make user dir
        ram.writeByte(TDRIVE, (ram.readByte(TDRIVE) & 0xF0) | (cpu.regE() & 0x0F));
      break;


    default:
      /*
        Unimplemented calls get listed
      */
#ifdef DEBUG
      // Show unimplemented BDOS calls only when debugging
      Serial.print("\r\nUnimplemented BDOS call.\r\n");
      Serial.print("C = 0x");
      Serial.print(port, 16);
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
