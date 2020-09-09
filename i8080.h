/**
  i8080.h - Intel 8080 (KR580VM80A) microprocessor core model

  Copyright (C) 2012 Alexander Demin <alexander@demin.ws>

  Credits

  Viacheslav Slavinsky, Vector-06C FPGA Replica
  http://code.google.com/p/vector06cc/

  Dmitry Tselikov, Bashrikia-2M and Radio-86RK on Altera DE1
  http://bashkiria-2m.narod.ru/fpga.html

  Ian Bartholomew, 8080/8085 CPU Exerciser
  http://www.idb.me.uk/sunhillow/8080.html

  Frank Cringle, The origianal exerciser for the Z80.

  Thanks to zx.pk.ru and nedopc.org/forum communities.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef I8080_H
#define I8080_H

#include <Arduino.h>


typedef unsigned char           uns8;
typedef unsigned short          uns16;
typedef unsigned long int       uns32;
typedef signed char             sgn8;
typedef signed short            sgn16;
typedef signed long int         sgn32;

typedef union {
  struct {
    uns8 l, h;
  } b;
  uns16 w;
} reg_pair;

typedef struct {
  uns8 carry_flag;
  uns8 unused1;
  uns8 parity_flag;
  uns8 unused3;
  uns8 half_carry_flag;
  uns8 unused5;
  uns8 zero_flag;
  uns8 sign_flag;
} flag_reg;

struct registers {
  flag_reg f;
  reg_pair af, bc, de, hl;
  reg_pair sp, pc;
  uns16 iff;
  uns16 last_pc;
};

static struct registers regs;


class I8080 {
  public:
    I8080();
    ~I8080();
    void init(void);
    int  instruction(void);
    void jump(int addr);
    int  pc(void);

    int  regBC(void);
    int  regDE(void);
    int  regHL(void);
    int  regSP(void);
    void regBC(int value);
    void regDE(int value);
    void regHL(int value);
    void regSP(int value);

    int  regA(void);
    int  regB(void);
    int  regC(void);
    int  regD(void);
    int  regE(void);
    int  regH(void);
    int  regL(void);
    void regA(uns8 value);
    void regB(uns8 value);
    void regC(uns8 value);
    void regD(uns8 value);
    void regE(uns8 value);
    void regH(uns8 value);
    void regL(uns8 value);

    int  read_word(int addr);
    void write_word(int addr, int word);
    int  read_byte(int addr);
    void write_byte(int addr, int byte);
    int  io_input(int port);
    void io_output(int port, int value);
    void iff(int on);

    void ret();
    void trace(bool newline = false);

    int state = 1;


  private:
    void store_flags(void);
    void retrieve_flags(void);
    int  execute(int opcode);

    uns32 work32;
    uns16 work16;
    uns8  work8;
    int   index;
    uns8  carry, add;

};




#endif
