/*
 *    This file is part of Audioflynt
 *    Copyright (C) 2015 Tim Hobbs
 *
 *    Audioflynt is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Audioflynt is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with Audioflynt.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPIFLASH_INCLUDED
#define SPIFLASH_INCLUDED

#include <stddef.h>
#include <stdint.h>

typedef struct cfi_header {
    uint8_t manufacturer;
    uint8_t memInterfaceType;
    uint8_t memDensity;
    uint8_t cfiLength;
    uint8_t sectorArchitecture;
    uint8_t familyId;
    uint8_t model[2];
} __attribute__((packed)) cfi_header;

typedef struct cfi_ident {
  uint8_t  qry[3];
  uint16_t P_ID;
  uint16_t P_ADR;
  uint16_t A_ID;
  uint16_t A_ADR;
  uint8_t  VccMin;
  uint8_t  VccMax;
  uint8_t  VppMin;
  uint8_t  VppMax;
  uint8_t  WordWriteTimeoutTyp;
  uint8_t  BufWriteTimeoutTyp;
  uint8_t  BlockEraseTimeoutTyp;
  uint8_t  ChipEraseTimeoutTyp;
  uint8_t  WordWriteTimeoutMax;
  uint8_t  BufWriteTimeoutMax;
  uint8_t  BlockEraseTimeoutMax;
  uint8_t  ChipEraseTimeoutMax;
  uint8_t  DevSize;
  uint16_t InterfaceDesc;
  uint16_t MaxBufWriteSize;
} __attribute__((packed)) cfi_ident;

void spiInit(void);

void spiflashGetDeviceInfo(cfi_header* header, cfi_ident* ident);

void spiflashWriteBlock(uint32_t address, uint8_t* data);

void spiflashReadData(uint32_t address, uint8_t* data, int len);

uint8_t spiflashGetStatusRegister(void);

void spiflashSetWriteEnabled(int writeEnabled);

void spiflashEraseDevice(void);
#endif
