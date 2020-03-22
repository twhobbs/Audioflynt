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

#ifndef FAT_INCLUDED
#define FAT_INCLUDED

#include <stdint.h>
#include <stdio.h>

typedef struct FAT_Header
{
    uint8_t jump[3];
    char oemName[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t numReservedSectors;
    uint8_t numFatCopies;
    uint16_t numRootDirectoryEntries;
    uint16_t numTotalSectors;
    uint8_t mediaDescriptorType;
    uint16_t numSectorsPerFat;
    uint16_t numSectorsPerTrack;
    uint16_t numHeads;
    uint32_t numHiddenSectors;
    uint32_t numSectorsInFilesystem;
    uint8_t logicalDriveNumber;
    uint8_t reserved;
    uint8_t extendedSignature;
    uint32_t serialNumber;
    char volumeLabel[11];
    char fsType[8];

} __attribute__((packed)) FAT_Header;

typedef struct FAT_DirEntry
{
    char filename[11];
    uint8_t attributes;
    uint8_t reserved[10];
    uint16_t time;
    uint16_t date;
    uint16_t startingCluster;
    uint32_t filesize;
} __attribute__((packed)) FAT_DirEntry;

#define ATTR_READ_ONLY 1
#define ATTR_HIDDEN 2
#define ATTR_SYSTEM_FILE 4
#define ATTR_LABEL 8
#define ATTR_SUBDIR 16
#define ATTR_ARCHIVE 32

typedef struct FATFile
{
    uint32_t size;
    uint32_t offset;
    
    uint32_t fatOffset;
    uint16_t startingCluster;
    uint16_t terminator;
    
    uint16_t currentCluster;
    uint32_t bytesPerCluster;
    
    uint32_t devicePositionBase;
    uint32_t currentDevicePosition;
    uint32_t clusterEndDevicePosition;
} FATFile;

int fatFindFile(const char* filename, FAT_Header* fatHeader, FAT_DirEntry* target);

FATFile fatOpenFile(FAT_Header* fatHeader, FAT_DirEntry* target);

size_t fatRead(void* dest, size_t len, FATFile* fatFile);

uint8_t fatGetC(FATFile* fatFile);

int32_t fatSeek(FATFile* fatFile, int32_t pos, int mode);

uint16_t fatNextCluster(FATFile* file, uint16_t cluster);

#endif
