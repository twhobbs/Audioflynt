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

#include "SpiFlash.h"
#include "FAT.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

int fatFindFile(const char* filename, FAT_Header* fatHeader, FAT_DirEntry* dirEntry)
{
    char match[11];
    for (int i = 0; i < 11; i++)
    {
	match[i] = 0x20;
    }
    
    int i = 0;
    while (i < 8 && filename[i] != '.' && filename[i] != 0)
    {
	match[i] = toupper(filename[i]);
	i++;
    }
    
    if (filename[i] == '.')
    {
	i++;
    }
    
    int extI = 8;
    while (extI < 11 && filename[i] != '.' && filename[i] != 0)
    {
	match[extI] = toupper(filename[i]);
	i++;
	extI++;
    }

    
    uint32_t dirEntryOffset = (fatHeader->numReservedSectors + fatHeader->numFatCopies * fatHeader->numSectorsPerFat) * fatHeader->bytesPerSector;
    
    int count = 0;
    int found = 0;
    
    spiflashReadData(dirEntryOffset, (uint8_t*)dirEntry, sizeof(FAT_DirEntry));
    
//     fprintf(stream, "Offset: %lu\r\n", dirEntryOffset);
//     fprintf(stream, "Match %.11s to %.11s\r\n", match, dirEntry->filename);
//     fprintf(stream, "Comp count %u to %u\r\n", count, fatHeader->numRootDirectoryEntries);
    
    while (found == 0 && dirEntry->filename[0] != 0 && count < fatHeader->numRootDirectoryEntries)
    {
	if ((dirEntry->attributes & ATTR_HIDDEN) == 0 && strncmp(dirEntry->filename, match, 11) == 0)
	{
	    found = 1;
	}
	else
	{
	    dirEntryOffset += sizeof(FAT_DirEntry);
	    count++;
	    
	    spiflashReadData(dirEntryOffset, (uint8_t*)dirEntry, sizeof(FAT_DirEntry));
// 	    fprintf(stream, "Offset: %lu\r\n", dirEntryOffset);
// 	    fprintf(stream, "Match %.11s to %.11s\r\n", match, dirEntry->filename);
// 	    fprintf(stream, "Comp count %u to %u\r\n", count, fatHeader->numRootDirectoryEntries);
    
	}
    }
    
    return found;
}

FATFile fatOpenFile(FAT_Header* fatHeader, FAT_DirEntry* target)
{
    FATFile ret;
    ret.fatOffset = fatHeader->numReservedSectors*fatHeader->bytesPerSector;
    ret.startingCluster = ret.currentCluster = target->startingCluster;
    ret.size = target->filesize;
    ret.offset = 0;
    spiflashReadData(ret.fatOffset+2, (uint8_t*)&ret.terminator, 2);
    
    ret.bytesPerCluster = fatHeader->bytesPerSector * fatHeader->sectorsPerCluster;
    
    ret.devicePositionBase = (fatHeader->numReservedSectors + fatHeader->numFatCopies * fatHeader->numSectorsPerFat) * fatHeader->bytesPerSector - 2 * ret.bytesPerCluster + fatHeader->numRootDirectoryEntries * 32;
    
    ret.currentDevicePosition = ret.devicePositionBase + ret.currentCluster * ret.bytesPerCluster;

    ret.clusterEndDevicePosition = ret.currentDevicePosition + ret.bytesPerCluster;
    return ret;
}

size_t fatRead(void* dest, size_t reqlen, FATFile* fatFile)
{
    uint32_t remaining = fatFile->size - fatFile->offset;
    size_t len = reqlen < remaining ? reqlen : remaining;
    
    spiflashReadData(fatFile->currentDevicePosition, dest, len);
    
    fatSeek(fatFile, len, SEEK_CUR);
    
    return len;
}

uint8_t fatGetC(FATFile* fatFile)
{
    uint8_t ret;
    fatRead(&ret, 1, fatFile);
    return ret;
}

int32_t fatSeek(FATFile* fatFile, int32_t pos, int mode)
{
    uint32_t target = 0;
    
    if (mode == SEEK_SET)
    {
	//fatFile->currentDevicePosition = fatFile->devicePositionBase + fatFile->currentCluster * fatFile->bytesPerCluster + pos;
	target = pos;
    }
    else if (mode == SEEK_CUR)
    {
	target = fatFile->offset + pos;
    }
    else if (mode == SEEK_END)
    {
	target = fatFile->size + pos;
    }
    
    if (target > fatFile->size)
    {
	target = fatFile->size;
    }
    
    if (target < fatFile->offset) //cbf going back through blocks, just go to start and then go forward
    {
	fatFile->offset = 0;
	fatFile->currentDevicePosition = fatFile->devicePositionBase;
	fatFile->currentCluster = fatFile->startingCluster;
    }
    
    uint16_t targetClusterCount = target/fatFile->bytesPerCluster;
    uint16_t clustersToSkip = (targetClusterCount) - (fatFile->offset/fatFile->bytesPerCluster);
    
    uint16_t i;
    for (i = 0; i < clustersToSkip; i++)
    {
	fatFile->currentCluster = fatNextCluster(fatFile, fatFile->currentCluster);
	
	fatFile->offset += fatFile->bytesPerCluster;
    }
    
    fatFile->offset = targetClusterCount * fatFile->bytesPerCluster;
    
    uint16_t diff = target - fatFile->offset;
    
    fatFile->offset += diff;
    fatFile->currentDevicePosition = fatFile->devicePositionBase + fatFile->currentCluster * fatFile->bytesPerCluster + diff;
    
    return 0;
}

uint16_t fatNextCluster(FATFile* file, uint16_t cluster)
{
    uint16_t ret;
    
    spiflashReadData(file->fatOffset + (cluster * 2), (uint8_t*)&ret, 2);
    
    return ret;
}
