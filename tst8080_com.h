/**
  TST8080_COM.h - 8080/8085 CPU Diagnostic, version 1.0, by
                  Microcosm Associates (Kelly Smith test)

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

#ifndef TST8080_COM_H
#define TST8080_COM_H

uint8_t TST8080_COM[] = {
  0xc3, 0xb2, 0x01, 0x4d, 0x49, 0x43, 0x52, 0x4f, 0x43, 0x4f, 0x53, 0x4d,
  0x20, 0x41, 0x53, 0x53, 0x4f, 0x43, 0x49, 0x41, 0x54, 0x45, 0x53, 0x20,
  0x38, 0x30, 0x38, 0x30, 0x2f, 0x38, 0x30, 0x38, 0x35, 0x20, 0x43, 0x50,
  0x55, 0x20, 0x44, 0x49, 0x41, 0x47, 0x4e, 0x4f, 0x53, 0x54, 0x49, 0x43,
  0x0d, 0x0a, 0x20, 0x56, 0x45, 0x52, 0x53, 0x49, 0x4f, 0x4e, 0x20, 0x31,
  0x2e, 0x30, 0x20, 0x20, 0x28, 0x43, 0x29, 0x20, 0x31, 0x39, 0x38, 0x30,
  0x0d, 0x0a, 0x24, 0xd5, 0xeb, 0x0e, 0x09, 0xcd, 0x05, 0x00, 0xd1, 0xc9,
  0x0e, 0x02, 0xcd, 0x05, 0x00, 0xc9, 0xf5, 0xcd, 0x6a, 0x01, 0x5f, 0xcd,
  0x54, 0x01, 0xf1, 0xcd, 0x6e, 0x01, 0x5f, 0xc3, 0x54, 0x01, 0x0f, 0x0f,
  0x0f, 0x0f, 0xe6, 0x0f, 0xfe, 0x0a, 0xfa, 0x77, 0x01, 0xc6, 0x07, 0xc6,
  0x30, 0xc9, 0x0d, 0x0a, 0x20, 0x43, 0x50, 0x55, 0x20, 0x49, 0x53, 0x20,
  0x4f, 0x50, 0x45, 0x52, 0x41, 0x54, 0x49, 0x4f, 0x4e, 0x41, 0x4c, 0x24,
  0x0d, 0x0a, 0x20, 0x43, 0x50, 0x55, 0x20, 0x48, 0x41, 0x53, 0x20, 0x46,
  0x41, 0x49, 0x4c, 0x45, 0x44, 0x21, 0x20, 0x20, 0x20, 0x20, 0x45, 0x52,
  0x52, 0x4f, 0x52, 0x20, 0x45, 0x58, 0x49, 0x54, 0x3d, 0x24, 0x31, 0xbd,
  0x07, 0x21, 0x03, 0x01, 0xcd, 0x4b, 0x01, 0xe6, 0x00, 0xca, 0xc3, 0x01,
  0xcd, 0xa0, 0x06, 0xd2, 0xc9, 0x01, 0xcd, 0xa0, 0x06, 0xea, 0xcf, 0x01,
  0xcd, 0xa0, 0x06, 0xf2, 0xd5, 0x01, 0xcd, 0xa0, 0x06, 0xc2, 0xe4, 0x01,
  0xda, 0xe4, 0x01, 0xe2, 0xe4, 0x01, 0xfa, 0xe4, 0x01, 0xc3, 0xe7, 0x01,
  0xcd, 0xa0, 0x06, 0xc6, 0x06, 0xc2, 0xef, 0x01, 0xcd, 0xa0, 0x06, 0xda,
  0xf8, 0x01, 0xe2, 0xf8, 0x01, 0xf2, 0xfb, 0x01, 0xcd, 0xa0, 0x06, 0xc6,
  0x70, 0xe2, 0x03, 0x02, 0xcd, 0xa0, 0x06, 0xfa, 0x0c, 0x02, 0xca, 0x0c,
  0x02, 0xd2, 0x0f, 0x02, 0xcd, 0xa0, 0x06, 0xc6, 0x81, 0xfa, 0x17, 0x02,
  0xcd, 0xa0, 0x06, 0xca, 0x20, 0x02, 0xda, 0x20, 0x02, 0xe2, 0x23, 0x02,
  0xcd, 0xa0, 0x06, 0xc6, 0xfe, 0xda, 0x2b, 0x02, 0xcd, 0xa0, 0x06, 0xca,
  0x34, 0x02, 0xe2, 0x34, 0x02, 0xfa, 0x37, 0x02, 0xcd, 0xa0, 0x06, 0xfe,
  0x00, 0xda, 0x4f, 0x02, 0xca, 0x4f, 0x02, 0xfe, 0xf5, 0xda, 0x4f, 0x02,
  0xc2, 0x4f, 0x02, 0xfe, 0xff, 0xca, 0x4f, 0x02, 0xda, 0x52, 0x02, 0xcd,
  0xa0, 0x06, 0xce, 0x0a, 0xce, 0x0a, 0xfe, 0x0b, 0xca, 0x5e, 0x02, 0xcd,
  0xa0, 0x06, 0xd6, 0x0c, 0xd6, 0x0f, 0xfe, 0xf0, 0xca, 0x6a, 0x02, 0xcd,
  0xa0, 0x06, 0xde, 0xf1, 0xde, 0x0e, 0xfe, 0xf0, 0xca, 0x76, 0x02, 0xcd,
  0xa0, 0x06, 0xe6, 0x55, 0xfe, 0x50, 0xca, 0x80, 0x02, 0xcd, 0xa0, 0x06,
  0xf6, 0x3a, 0xfe, 0x7a, 0xca, 0x8a, 0x02, 0xcd, 0xa0, 0x06, 0xee, 0x0f,
  0xfe, 0x75, 0xca, 0x94, 0x02, 0xcd, 0xa0, 0x06, 0xe6, 0x00, 0xdc, 0xa0,
  0x06, 0xe4, 0xa0, 0x06, 0xfc, 0xa0, 0x06, 0xc4, 0xa0, 0x06, 0xfe, 0x00,
  0xca, 0xaa, 0x02, 0xcd, 0xa0, 0x06, 0xd6, 0x77, 0xd4, 0xa0, 0x06, 0xec,
  0xa0, 0x06, 0xf4, 0xa0, 0x06, 0xcc, 0xa0, 0x06, 0xfe, 0x89, 0xca, 0xc0,
  0x02, 0xcd, 0xa0, 0x06, 0xe6, 0xff, 0xe4, 0xcd, 0x02, 0xfe, 0xd9, 0xca,
  0x2a, 0x03, 0xcd, 0xa0, 0x06, 0xe8, 0xc6, 0x10, 0xec, 0xd9, 0x02, 0xc6,
  0x02, 0xe0, 0xcd, 0xa0, 0x06, 0xe0, 0xc6, 0x20, 0xfc, 0xe5, 0x02, 0xc6,
  0x04, 0xe8, 0xcd, 0xa0, 0x06, 0xf0, 0xc6, 0x80, 0xf4, 0xf1, 0x02, 0xc6,
  0x80, 0xf8, 0xcd, 0xa0, 0x06, 0xf8, 0xc6, 0x40, 0xd4, 0xfd, 0x02, 0xc6,
  0x40, 0xf0, 0xcd, 0xa0, 0x06, 0xd8, 0xc6, 0x8f, 0xdc, 0x09, 0x03, 0xd6,
  0x02, 0xd0, 0xcd, 0xa0, 0x06, 0xd0, 0xc6, 0xf7, 0xc4, 0x15, 0x03, 0xc6,
  0xfe, 0xd8, 0xcd, 0xa0, 0x06, 0xc8, 0xc6, 0x01, 0xcc, 0x21, 0x03, 0xc6,
  0xd0, 0xc0, 0xcd, 0xa0, 0x06, 0xc0, 0xc6, 0x47, 0xfe, 0x47, 0xc8, 0xcd,
  0xa0, 0x06, 0x3e, 0x77, 0x3c, 0x47, 0x04, 0x48, 0x0d, 0x51, 0x5a, 0x63,
  0x6c, 0x7d, 0x3d, 0x4f, 0x59, 0x6b, 0x45, 0x50, 0x62, 0x7c, 0x57, 0x14,
  0x6a, 0x4d, 0x0c, 0x61, 0x44, 0x05, 0x58, 0x7b, 0x5f, 0x1c, 0x43, 0x60,
  0x24, 0x4c, 0x69, 0x55, 0x15, 0x7a, 0x67, 0x25, 0x54, 0x42, 0x68, 0x2c,
  0x5d, 0x1d, 0x4b, 0x79, 0x6f, 0x2d, 0x65, 0x5c, 0x53, 0x4a, 0x41, 0x78,
  0xfe, 0x77, 0xc4, 0xa0, 0x06, 0xaf, 0x06, 0x01, 0x0e, 0x03, 0x16, 0x07,
  0x1e, 0x0f, 0x26, 0x1f, 0x2e, 0x3f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85,
  0x87, 0xfe, 0xf0, 0xc4, 0xa0, 0x06, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95,
  0xfe, 0x78, 0xc4, 0xa0, 0x06, 0x97, 0xc4, 0xa0, 0x06, 0x3e, 0x80, 0x87,
  0x06, 0x01, 0x0e, 0x02, 0x16, 0x03, 0x1e, 0x04, 0x26, 0x05, 0x2e, 0x06,
  0x88, 0x06, 0x80, 0x80, 0x80, 0x89, 0x80, 0x80, 0x8a, 0x80, 0x80, 0x8b,
  0x80, 0x80, 0x8c, 0x80, 0x80, 0x8d, 0x80, 0x80, 0x8f, 0xfe, 0x37, 0xc4,
  0xa0, 0x06, 0x3e, 0x80, 0x87, 0x06, 0x01, 0x98, 0x06, 0xff, 0x80, 0x99,
  0x80, 0x9a, 0x80, 0x9b, 0x80, 0x9c, 0x80, 0x9d, 0xfe, 0xe0, 0xc4, 0xa0,
  0x06, 0x3e, 0x80, 0x87, 0x9f, 0xfe, 0xff, 0xc4, 0xa0, 0x06, 0x3e, 0xff,
  0x06, 0xfe, 0x0e, 0xfc, 0x16, 0xef, 0x1e, 0x7f, 0x26, 0xf4, 0x2e, 0xbf,
  0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa7, 0xfe, 0x24, 0xc4, 0xa0, 0x06,
  0xaf, 0x06, 0x01, 0x0e, 0x02, 0x16, 0x04, 0x1e, 0x08, 0x26, 0x10, 0x2e,
  0x20, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb7, 0xfe, 0x3f, 0xc4, 0xa0,
  0x06, 0x3e, 0x00, 0x26, 0x8f, 0x2e, 0x4f, 0xa8, 0xa9, 0xaa, 0xab, 0xac,
  0xad, 0xfe, 0xcf, 0xc4, 0xa0, 0x06, 0xaf, 0xc4, 0xa0, 0x06, 0x06, 0x44,
  0x0e, 0x45, 0x16, 0x46, 0x1e, 0x47, 0x26, 0x06, 0x2e, 0xbf, 0x70, 0x06,
  0x00, 0x46, 0x3e, 0x44, 0xb8, 0xc4, 0xa0, 0x06, 0x71, 0x0e, 0x00, 0x4e,
  0x3e, 0x45, 0xb9, 0xc4, 0xa0, 0x06, 0x72, 0x16, 0x00, 0x56, 0x3e, 0x46,
  0xba, 0xc4, 0xa0, 0x06, 0x73, 0x1e, 0x00, 0x5e, 0x3e, 0x47, 0xbb, 0xc4,
  0xa0, 0x06, 0x74, 0x26, 0x06, 0x2e, 0xbf, 0x66, 0x3e, 0x06, 0xbc, 0xc4,
  0xa0, 0x06, 0x75, 0x26, 0x06, 0x2e, 0xbf, 0x6e, 0x3e, 0xbf, 0xbd, 0xc4,
  0xa0, 0x06, 0x26, 0x06, 0x2e, 0xbf, 0x3e, 0x32, 0x77, 0xbe, 0xc4, 0xa0,
  0x06, 0x86, 0xfe, 0x64, 0xc4, 0xa0, 0x06, 0xaf, 0x7e, 0xfe, 0x32, 0xc4,
  0xa0, 0x06, 0x26, 0x06, 0x2e, 0xbf, 0x7e, 0x96, 0xc4, 0xa0, 0x06, 0x3e,
  0x80, 0x87, 0x8e, 0xfe, 0x33, 0xc4, 0xa0, 0x06, 0x3e, 0x80, 0x87, 0x9e,
  0xfe, 0xcd, 0xc4, 0xa0, 0x06, 0xa6, 0xc4, 0xa0, 0x06, 0x3e, 0x25, 0xb6,
  0xfe, 0x37, 0xc4, 0xa0, 0x06, 0xae, 0xfe, 0x05, 0xc4, 0xa0, 0x06, 0x36,
  0x55, 0x34, 0x35, 0x86, 0xfe, 0x5a, 0xc4, 0xa0, 0x06, 0x01, 0xff, 0x12,
  0x11, 0xff, 0x12, 0x21, 0xff, 0x12, 0x03, 0x13, 0x23, 0x3e, 0x13, 0xb8,
  0xc4, 0xa0, 0x06, 0xba, 0xc4, 0xa0, 0x06, 0xbc, 0xc4, 0xa0, 0x06, 0x3e,
  0x00, 0xb9, 0xc4, 0xa0, 0x06, 0xbb, 0xc4, 0xa0, 0x06, 0xbd, 0xc4, 0xa0,
  0x06, 0x0b, 0x1b, 0x2b, 0x3e, 0x12, 0xb8, 0xc4, 0xa0, 0x06, 0xba, 0xc4,
  0xa0, 0x06, 0xbc, 0xc4, 0xa0, 0x06, 0x3e, 0xff, 0xb9, 0xc4, 0xa0, 0x06,
  0xbb, 0xc4, 0xa0, 0x06, 0xbd, 0xc4, 0xa0, 0x06, 0x32, 0xbf, 0x06, 0xaf,
  0x3a, 0xbf, 0x06, 0xfe, 0xff, 0xc4, 0xa0, 0x06, 0x2a, 0xbd, 0x06, 0x22,
  0xbf, 0x06, 0x3a, 0xbd, 0x06, 0x47, 0x3a, 0xbf, 0x06, 0xb8, 0xc4, 0xa0,
  0x06, 0x3a, 0xbe, 0x06, 0x47, 0x3a, 0xc0, 0x06, 0xb8, 0xc4, 0xa0, 0x06,
  0x3e, 0xaa, 0x32, 0xbf, 0x06, 0x44, 0x4d, 0xaf, 0x0a, 0xfe, 0xaa, 0xc4,
  0xa0, 0x06, 0x3c, 0x02, 0x3a, 0xbf, 0x06, 0xfe, 0xab, 0xc4, 0xa0, 0x06,
  0x3e, 0x77, 0x32, 0xbf, 0x06, 0x2a, 0xbd, 0x06, 0x11, 0x00, 0x00, 0xeb,
  0xaf, 0x1a, 0xfe, 0x77, 0xc4, 0xa0, 0x06, 0xaf, 0x84, 0x85, 0xc4, 0xa0,
  0x06, 0x3e, 0xcc, 0x12, 0x3a, 0xbf, 0x06, 0xfe, 0xcc, 0x12, 0x3a, 0xbf,
  0x06, 0xfe, 0xcc, 0xc4, 0xa0, 0x06, 0x21, 0x77, 0x77, 0x29, 0x3e, 0xee,
  0xbc, 0xc4, 0xa0, 0x06, 0xbd, 0xc4, 0xa0, 0x06, 0x21, 0x55, 0x55, 0x01,
  0xff, 0xff, 0x09, 0x3e, 0x55, 0xd4, 0xa0, 0x06, 0xbc, 0xc4, 0xa0, 0x06,
  0x3e, 0x54, 0xbd, 0xc4, 0xa0, 0x06, 0x21, 0xaa, 0xaa, 0x11, 0x33, 0x33,
  0x19, 0x3e, 0xdd, 0xbc, 0xc4, 0xa0, 0x06, 0xbd, 0xc4, 0xa0, 0x06, 0x37,
  0xd4, 0xa0, 0x06, 0x3f, 0xdc, 0xa0, 0x06, 0x3e, 0xaa, 0x2f, 0xfe, 0x55,
  0xc4, 0xa0, 0x06, 0xb7, 0x27, 0xfe, 0x55, 0xc4, 0xa0, 0x06, 0x3e, 0x88,
  0x87, 0x27, 0xfe, 0x76, 0xc4, 0xa0, 0x06, 0xaf, 0x3e, 0xaa, 0x27, 0xd4,
  0xa0, 0x06, 0xfe, 0x10, 0xc4, 0xa0, 0x06, 0xaf, 0x3e, 0x9a, 0x27, 0xd4,
  0xa0, 0x06, 0xc4, 0xa0, 0x06, 0x37, 0x3e, 0x42, 0x07, 0xdc, 0xa0, 0x06,
  0x07, 0xd4, 0xa0, 0x06, 0xfe, 0x09, 0xc4, 0xa0, 0x06, 0x0f, 0xd4, 0xa0,
  0x06, 0x0f, 0xfe, 0x42, 0xc4, 0xa0, 0x06, 0x17, 0x17, 0xd4, 0xa0, 0x06,
  0xfe, 0x08, 0xc4, 0xa0, 0x06, 0x1f, 0x1f, 0xdc, 0xa0, 0x06, 0xfe, 0x02,
  0xc4, 0xa0, 0x06, 0x01, 0x34, 0x12, 0x11, 0xaa, 0xaa, 0x21, 0x55, 0x55,
  0xaf, 0xc5, 0xd5, 0xe5, 0xf5, 0x01, 0x00, 0x00, 0x11, 0x00, 0x00, 0x21,
  0x00, 0x00, 0x3e, 0xc0, 0xc6, 0xf0, 0xf1, 0xe1, 0xd1, 0xc1, 0xdc, 0xa0,
  0x06, 0xc4, 0xa0, 0x06, 0xe4, 0xa0, 0x06, 0xfc, 0xa0, 0x06, 0x3e, 0x12,
  0xb8, 0xc4, 0xa0, 0x06, 0x3e, 0x34, 0xb9, 0xc4, 0xa0, 0x06, 0x3e, 0xaa,
  0xba, 0xc4, 0xa0, 0x06, 0xbb, 0xc4, 0xa0, 0x06, 0x3e, 0x55, 0xbc, 0xc4,
  0xa0, 0x06, 0xbd, 0xc4, 0xa0, 0x06, 0x21, 0x00, 0x00, 0x39, 0x22, 0xc4,
  0x06, 0x31, 0xc3, 0x06, 0x3b, 0x3b, 0x33, 0x3b, 0x3e, 0x55, 0x32, 0xc1,
  0x06, 0x2f, 0x32, 0xc2, 0x06, 0xc1, 0xb8, 0xc4, 0xa0, 0x06, 0x2f, 0xb9,
  0xc4, 0xa0, 0x06, 0x21, 0xc3, 0x06, 0xf9, 0x21, 0x33, 0x77, 0x3b, 0x3b,
  0xe3, 0x3a, 0xc2, 0x06, 0xfe, 0x77, 0xc4, 0xa0, 0x06, 0x3a, 0xc1, 0x06,
  0xfe, 0x33, 0xc4, 0xa0, 0x06, 0x3e, 0x55, 0xbd, 0xc4, 0xa0, 0x06, 0x2f,
  0xbc, 0xc4, 0xa0, 0x06, 0x2a, 0xc4, 0x06, 0xf9, 0x21, 0xb4, 0x06, 0xe9,
  0x21, 0x90, 0x01, 0xcd, 0x4b, 0x01, 0xe1, 0xe5, 0x7c, 0xcd, 0x5a, 0x01,
  0xe1, 0x7d, 0xcd, 0x5a, 0x01, 0xc3, 0x00, 0x00, 0x21, 0x7a, 0x01, 0xcd,
  0x4b, 0x01, 0xc3, 0x00, 0x00, 0xbf, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
uint16_t TST8080_COM_len = 1536;

#endif /* TST8080_COM_H */
