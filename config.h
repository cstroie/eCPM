/**
  config.h - Common configuration

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

#ifndef CONFIG_H
#define CONFIG_H


// Debug mode
//#define DEBUG 1
//#define DEBUG_BDOS_CALLS
//#define DEBUG_FCB_READ
//#define DEBUG_FCB_WRITE
//#define DEBUG_DIRENTRY

// File system block size
#define BLS_2048

// RAM cache size (bytes)
#define RAM_BUFFER_SIZE  (8)

// Serial port speed
#define SERIAL_SPEED  (115200)

// SPI speed
#define SPI_SPEED (20000000)

// LED configuration
#define LEDinv  (true)

#endif /* CONFIG_H */
