/**
  Embedded CP/M Machine

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

// Global parameters
#include "global.h"
// Configuration
#include "config.h"

#include "ram.h"
#include "i8080.h"

#include "tst8080_com.h"
#include "8080exm_com.h"
#include "demo_com.h"

// Some declarations
void callBIOS(int port, int value);
void callBDOS(int port);


// Persistent configuration
RAM ram(D0);

int  I8080::read_word(int addr) {
  return ram.readWord(addr);
}

void I8080::write_word(int addr, int word) {
  return ram.writeWord(addr, word);
}

int  I8080::read_byte(int addr) {
  return ram.readByte(addr);
}

void I8080::write_byte(int addr, int byte) {
  ram.writeByte(addr, byte);
}

int  I8080::io_input(int port) {
  callBDOS(port);
  return regA();
}
void I8080::io_output(int port, int value) {
  callBIOS(port, value);
}

void I8080::iff(int on) {
  state = 0;
  //ram.writeByte(addr, byte);
}


I8080 cpu;

void patchCPM() {
  uint16_t offset;

  // Patch in a JP to WBOOT at location 0x0000
  ram.writeByte(0x0000, JP);
  ram.writeWord(0x0001, WBOOT);
  //  Patch in a JP to the BDOS entry at location 0x0005
  ram.writeByte(ENTRY, JP);
  ram.writeWord(ENTRY + 1, BDOSCODE + 0x06);

  // IOBYTE - Points to Console */
  ram.writeByte(IOBYTE, 0x3D);
  // Current drive/user - A:/0 */
  ram.writeByte(TDRIVE, 0x00);

  // Serial number, 6 bytes
  uint8_t cpmSerialNo[] = {0, 22, 0, 0, 0, 0};
  ram.write(BDOSCODE, cpmSerialNo, 6);

  // BDOS entry (FBASE)
  ram.writeByte(BDOSCODE + 6, JP);
  ram.writeWord(BDOSCODE + 7, BDOSCODE + 0x0100);

  // BDOS dispatcher (FBASE1)
  ram.writeByte(BDOSCODE + 0x0100, INa);
  ram.writeByte(BDOSCODE + 0x0100 + 1, 0x00);
  ram.writeByte(BDOSCODE + 0x0100 + 2, RET);

  // BIOS jump vectors (17 functions)
  for (uint8_t i = 0; i < 17; i++) {
    offset = i * 3;
    // BIOS jump vectors
    ram.writeByte(BIOSCODE + offset, JP);
    ram.writeWord(BIOSCODE + offset + 1, BIOSCODE + 0x0100 + offset);
    // BIOS routines
    ram.writeByte(BIOSCODE + 0x0100 + offset, OUTa);
    ram.writeByte(BIOSCODE + 0x0100 + offset + 1, i);
    ram.writeByte(BIOSCODE + 0x0100 + offset + 2, RET);
  }
}

void callBIOS(int port, int value) {
  /*
    Serial.print("OUT ");
    Serial.print(port, 16);
    Serial.print(" ");
    Serial.println(value, 16);
  */

  switch (port) {
    case 0x00:
      // BOOT: Ends CPM
      cpu.state = 0;
      break;
    case 0x01:
      // WBOOT: Back to CCP
      break;
    case 0x02:
      // CONST: Console status
      cpu.regA(Serial.available() ? 0xff : 0x00);
      break;
    case 0x03:
      // CONIN: Console input
      while (!Serial.available()) { }
      cpu.regA(Serial.read());
      break;
    case 0x04:
      // CONOUT Console output
      Serial.write((char)(cpu.regC() & 0x7F));
      break;
    case 0x05:
      // LIST List output
      break;
    case 0x06:
      // PUNCH/AUXOUT Punch output
      break;
    case 0x07:
      // READER Reader input (0x1A = device not implemented)
      cpu.regA(0x1A);
      break;
    case 0x08:
      // HOME Home disk head
      break;
    case 0x09:
      // SELDSK Select disk drive
      cpu.regHL(0x0000);
      break;
    case 0x0A:
      // SETTRK Set track number
      break;
    case 0x0B:
      // SETSEC Set sector number
      break;
    case 0x0C:
      // SETDMA Set DMA address
      cpu.regHL(cpu.regBC());
      //dmaAddr = cpu.regBC();
      break;
    case 0x0D:
      // READ Read selected sector
      cpu.regA(0x00);
      break;
    case 0x0E:
      // WRITE Write selected sector
      cpu.regA(0x00);
      break;
    case 0x0F:
      // LISTST Get list device status
      cpu.regA(0xFF);
      break;
    case 0x10:
      // SECTRAN Sector translate
      cpu.regHL(cpu.regBC());  // HL=BC=No translation (1:1)
      break;
    default:
#ifdef DEBUG
      // Show unimplemented BIOS calls only when debugging
      Serial.print("\r\nUnimplemented BIOS call: 0x");
      Serial.print(port, 16);
      Serial.print("\r\n");
      cpu.trace();
#endif
      break;
  }
}

void callBDOS(int port) {
  uint8_t  b, count;
  char c;
  uint16_t w;

  // HL is reset by the BDOS
  cpu.regHL(0x0000);

  switch (cpu.regC()) {
    case 0:
      /*
        C = 0 : System reset
        Doesn't return. Reloads CP/M
        Same as call to "BOOT"
      */
      cpu.state = 0;
      break;
    case 1:
      /*
        C = 1 : Console input
        Gets a char from the console
        Returns: A=Char
      */
      while (!Serial.available()) { }
      c = Serial.read();
      Serial.write(c);
      cpu.regHL(c);
      break;
    case 2:
      /*
        C = 2 : Console output
        E = Char
        Sends the char in E to the console
      */
      Serial.write((char)(cpu.regE() & 0x7F));
      break;
    case 3:
      /*
        C = 3 : Auxiliary (Reader) input
        Returns: A=Char
      */
      cpu.regHL(0x1A);
      break;
    case 4:
      /*
        C = 4 : Auxiliary (Punch) output
      */
      break;
    case 5:
      /*
        C = 5 : Printer output
      */
      break;
    case 6:
      /*
        C = 6 : Direct console IO
        E = 0xFF : Checks for char available and returns it, or 0x00 if none (read)
        E = char : Outputs char (write)
        Returns: A=Char or 0x00 (on read)
      */
      if (cpu.regE() == 0xFF)
        cpu.regHL(Serial.available() ? Serial.read() : 0x00);
      else
        Serial.write((char)(cpu.regE() & 0x7F));
      break;
    case 7:
      /*
        C = 7 : Get IOBYTE
        Gets the system IOBYTE
        Returns: A = IOBYTE
      */
      cpu.regHL(ram.readByte(IOBYTE));
      break;
    case 8:
      /*
        C = 8 : Set IOBYTE
        E = IOBYTE
        Sets the system IOBYTE to E
      */
      ram.writeByte(IOBYTE, cpu.regE());
      break;
    case 9:
      /*
        C = 9 : Output string
        DE = Address of string
        Sends the $ terminated string pointed by (DE) to the screen
      */
      w = cpu.regDE();
      while (true) {
        b = ram.readByte(w++);
        if (b == '$') break;
        Serial.print((char)(b & 0x7F));
      }
      cpu.regDE(w);
      break;
    case 10:
      /*
        C = 10 (0Ah) : Buffered input
        DE = Address of buffer
        Reads (DE) bytes from the console
        Returns: A = Number os chars read
        DE) = First char
      */
      // Buffer
      w = cpu.regDE();
      // Gets the number of characters to read
      b = ram.readByte(w);
      // Points to the number read
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
        if (c == 5)
          // ^E
          Serial.write("\r\n");
        if ((c == 0x08 || c == 0x7F) && count > 0) {
          // ^H and DEL
          Serial.write("\b \b");
          count--;
          continue;
        }
        if (c == 0x0A || c == 0x0D) {
          // ^J and ^M
          break;
        }
        if (c == 18) {
          // ^R
          Serial.write("#\r\n  ");
          for (uint8_t j = 1; j <= count; ++j)
            Serial.write((char)ram.readByte(w + j));
        }
        if (c == 21) {
          // ^U
          Serial.write("#\r\n  ");
          w = cpu.regDE();
          b = ram.readByte(w);
          w++;
          count = 0;
        }
        if (c == 24) {
          // ^X
          for (uint8_t j = 0; j < count; ++j)
            Serial.write("\b \b");
          w = cpu.regDE();
          b = ram.readByte(w);
          w++;
          count = 0;
        }
        // Invalid character
        if (c < 0x20 || c > 0x7E)
          continue;
        Serial.write((char)(b & 0x7F));
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
    case 11:
      /*
        C = 11 (0Bh) : Get console status
        Returns: A=0x00 or 0xFF
      */
      cpu.regHL(Serial.available() ? 0xff : 0x00);
      break;
    case 12:
      /*
        C = 12 (0Ch) : Get version number
        Returns: B=H=system type, A=L=version number
      */
      cpu.regHL(0x22);
      break;

    default:
      /*
        Unimplemented calls get listed
      */
#ifdef DEBUG
      // Show unimplemented BDOS calls only when debugging
      Serial.print("\r\nUnimplemented BDOS call.\r\n");
      Serial.print("C = 0x");
      Serial.print(ch, 16);
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

/**
  Main Arduino setup function
*/
void setup() {
  // Serial port configuration
  Serial.begin(115200);

  // SPI
  SPI.begin();

  // Init the SPI RAM
  // FIXME This breaks the SPI
  //ram.init();

  patchCPM();

  // Load some data
  ram.write(0x0100, TST8080_COM, TST8080_COM_len);
  //ram.write(0x0100, DEMO_COM, DEMO_COM_len);
  //ram.write(0x0100, __8080EXM_COM, __8080EXM_COM_len);

  // RAM hex dump
  ram.hexdump(0x0000, 0x0200);


  cpu.jump(0x0100);
  //Serial.println(cpu.instruction());
  //cpu.trace();
}

/**
  Main Arduino loop
*/
void loop() {
  if (cpu.state) {
    cpu.instruction();
    //cpu.trace();
  }

  //delay(10);
}
