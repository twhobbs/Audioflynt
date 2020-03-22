/*
 *    Serial Interface command handler of Audioflynt
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


#include "CommandHandler.h"
#include "../AudioFile.h"

#define BUFFLEN 32

uint32_t blockdest = 0;

size_t fatReadFunc(void *dest, size_t size, void *source)
{
    return fatRead(dest, size, (FATFile*) source);
}

int32_t fatSeekFunc(void *source, int32_t offset, int origin)
{
    return fatSeek((FATFile*) source, offset, origin);
}

int32_t fatTellFunc(void* source)
{
    return ((FATFile*) source)->offset;
}


    int (*seekfunc)(void *source, int offset, int origin);

uint16_t crc16(uint8_t* data, int len)
{
    CRC.CTRL = CRC_RESET_RESET0_gc;
    //CRC.CTRL |= CRC_CRC32_bm;
    CRC.CTRL |= CRC_SOURCE_IO_gc;
    for (int i = 0; i < len; i++)
    {
	CRC.DATAIN = data[i];
    }
    
    CRC.STATUS |= CRC_BUSY_bm; // finish
    loop_until_bit_is_clear(CRC.STATUS,CRC_BUSY_bp); // wait for done
    
    uint16_t checksum = ((uint16_t)CRC.CHECKSUM0 & 0x00FF);
    checksum |= (((uint16_t)CRC.CHECKSUM1 << 8) & 0xFF00);
    
    return checksum;
}

/*void play(FILE* stream, FAT_Header* fatHeader, char* name)
{
        msadpcm_debugStream = stream;
    
    	FAT_DirEntry dirEntry;
	
	int found = fatFindFile(name, fatHeader, &dirEntry);
	if (found)
	{
	    FATFile playFile = fatOpenFile(fatHeader, &dirEntry);
	    
	    fprintf(stream, "File found, filesize: %lu\r\n", playFile.size);
	    fprintf(stream, "First cluster: %u\r\n", playFile.startingCluster);
	    fprintf(stream, "Device pos: %lu\r\n", playFile.currentDevicePosition);
	    fprintf(stream, "Terminator: %u\r\n", playFile.terminator);
	    
	    AudioFileHeader afh;
	    afh.source.source = &playFile;
	    afh.source.readfunc = fatReadFunc;
	    afh.source.seekfunc = fatSeekFunc;
            afh.source.tellfunc = fatTellFunc;
	    
	    AudioFile_initialise(&afh, stream, AFORMAT_WAVE);
	    
	    fprintf(stream, "Sample Rate: %u\r\n", afh.sampleRate);
            fprintf(stream, "Rewind Seek: %u\r\n", afh.codecData.rewindSeek);
	    
            uint16_t buff[BUFFLEN];
            
            int16_t read;
            uint16_t i;
            for (i = 0; i < 5000; i++)
            {
                read = AudioFile_sample(&afh, buff, BUFFLEN);
                fprintf(stream, ".");
            }
            fprintf(stream, "\r\n");
            
            
            for (i = 0; i < read; i++)
            {
                fprintf(stream, "%u ", buff[i]);
            }
            fprintf(stream, "\r\n");
	}
}*/

void handleCommand(FILE* stream, uint8_t* buff, int buffLength, int* readMode, uint8_t* data, FAT_Header* fatHeader)
{
#ifdef FULL_COMMANDS
    if (strncmp((char*)buff, "geom", 4) == 0)
    {
	cfi_ident cfi;
	cfi_header header;
	
	spiflashGetDeviceInfo(&header, &cfi);

	fprintf(stream, "%u:%u\r\n", cfi.DevSize, cfi.MaxBufWriteSize);
    }
    else if (strncmp((char*)buff, "data", 4) == 0)
    {
	*readMode = true;
    }
    else if (strncmp((char*)buff, "block", 5) == 0)
    {
	sscanf((char*)buff+6, "%lu", &blockdest);
	fprintf(stream, "%lu\r\n", blockdest);
    }
    else if (strncmp((char*)buff, "prog", 4) == 0)
    {
	spiflashWriteBlock(blockdest << 8, data);
    }
    else if (strncmp((char*)buff, "eras", 4) == 0)
    {
	spiflashEraseDevice();
    }
    else if (strncmp((char*)buff, "stat", 4) == 0)
    {
	fprintf(stream, "%u\r\n", spiflashGetStatusRegister());
    }
    else if (strncmp((char*)buff, "wren", 4) == 0)
    {
	spiflashSetWriteEnabled(1);
    }
    else if (strncmp((char*)buff, "wrdi", 4) == 0)
    {
	spiflashSetWriteEnabled(0);
    }
    else if (strncmp((char*)buff, "bcrc", 4) == 0)
    {
	
	uint16_t checksum = crc16(data, 256);
	
	fprintf(stream, "%u\r\n", checksum);
    }
    else if (strncmp((char*)buff, "fcrc", 4) == 0)
    {
	uint8_t page[256];
	
	spiflashReadData(blockdest << 8, page, 256);
	
	uint16_t checksum = crc16(page, 256);
	
	fprintf(stream, "%u\r\n", checksum);
    }
    else if (strncmp((char*)buff, "fati", 4) == 0)
    {
	fprintf(stream, "oemName: %.8s\r\n", fatHeader->oemName);
	fprintf(stream, "bytesPerSector: %u\r\n", fatHeader->bytesPerSector);
	fprintf(stream, "sectorsPerCluster: %u\r\n", fatHeader->sectorsPerCluster);
	fprintf(stream, "numReservedSectors: %u\r\n", fatHeader->numReservedSectors);
	fprintf(stream, "numFatCopies: %u\r\n", fatHeader->numFatCopies);
	fprintf(stream, "numRootDirectoryEntries: %u\r\n", fatHeader->numRootDirectoryEntries);
	fprintf(stream, "numTotalSectors: %u\r\n", fatHeader->numTotalSectors);
	fprintf(stream, "mediaDescriptorType: %x\r\n", fatHeader->mediaDescriptorType);
	fprintf(stream, "numSectorsPerFat: %u\r\n", fatHeader->numSectorsPerFat);
	fprintf(stream, "numSectorsPerTrack: %u\r\n", fatHeader->numSectorsPerTrack);
	fprintf(stream, "numHeads: %u\r\n", fatHeader->numHeads);
	fprintf(stream, "numHiddenSectors: %lu\r\n", fatHeader->numHiddenSectors);
	
	fprintf(stream, "volumeLabel: %.8s\r\n", fatHeader->volumeLabel);
	fprintf(stream, "fsType: %.8s\r\n", fatHeader->fsType);
    }
    else if (strncmp((char*)buff, "walk", 4) == 0)
    {
	char* filename = (char*)buff+5;
	
	FAT_DirEntry dirEntry;
	
	int found = fatFindFile(filename, fatHeader, &dirEntry);
	
	if (found)
	{
	    FATFile file = fatOpenFile(fatHeader, &dirEntry);
	    
	    uint16_t cluster = dirEntry.startingCluster;
	    while (cluster != 0 && cluster != file.terminator)
	    {
		fprintf(stream, "%u\r\n", cluster);
		cluster = fatNextCluster(&file, cluster);
	    }
	}
	else
	{
	    fprintf(stream, "File not found\r\n");
	}
    }
    else if (strncmp((char*)buff, "file", 4) == 0)
    {
	char* filename = (char*)buff+5;
	
	FAT_DirEntry dirEntry;
	
	int found = fatFindFile(filename, fatHeader, &dirEntry);
	
	if (found)
	{
	    fprintf(stream, "File found, filesize: %lu\r\n", dirEntry.filesize);
	    fprintf(stream, "First cluster: %u\r\n", dirEntry.startingCluster);
	}
	else
	{
	    fprintf(stream, "File not found\r\n");
	}
	
    }
    else if (strncmp((char*)buff, "cat", 3) == 0)
    {
	char* filename = (char*)buff+4;
	
	FAT_DirEntry dirEntry;
	
	int found = fatFindFile(filename, fatHeader, &dirEntry);
	
	if (found)
	{
	    FATFile file = fatOpenFile(fatHeader, &dirEntry);
	    char buff[32];
	    
	    int read = fatRead(buff, 31, &file);
	    
	    while (read > 0)
	    {
		buff[read] = 0;
		char *p = buff;
		while (*p != 0)
		{
		    if (*p == '\n')
		    {
			fputs("\r\n", stream);
		    }
		    else
		    {
			fputc(*p, stream);
		    }
		    p++;
		}
		read = fatRead(buff, 31, &file);
	    }
	}
	else
	{
	    fprintf(stream, "File not found\r\n");
	}
	
    }
    else if (strncmp((char*)buff, "crc", 3) == 0)
    {
	char* filename = (char*)buff+4;
	
	FAT_DirEntry dirEntry;
	
	int found = fatFindFile(filename, fatHeader, &dirEntry);
	
	if (found)
	{
	    
	    CRC.CTRL = CRC_RESET_RESET0_gc;
	    //CRC.CTRL |= CRC_CRC32_bm;
	    CRC.CTRL |= CRC_SOURCE_IO_gc;
	    
	    FATFile file = fatOpenFile(fatHeader, &dirEntry);
	    char buff[128];
	    size_t read = fatRead(buff, 128, &file);
	    while (read > 0)
	    {
		for (int i = 0; i < read; i++)
		{
		    CRC.DATAIN = buff[i];
		}
		read = fatRead(buff, 128, &file);
	    }
	    
	    CRC.STATUS |= CRC_BUSY_bm; // finish
	    loop_until_bit_is_clear(CRC.STATUS,CRC_BUSY_bp); // wait for done
	    
	    uint16_t checksum = ((uint16_t)CRC.CHECKSUM0 & 0x00FF);
	    checksum |= (((uint16_t)CRC.CHECKSUM1 << 8) & 0xFF00);
	    
	    fprintf(stream, "Checksum: %u\r\n", checksum);
	}
	else
	{
	    fprintf(stream, "File not found\r\n");
	}
	
    }
    //else 
#endif
    /*if (strncmp((char*)buff, "play", 4) == 0)
    {
	play(stream, fatHeader, (char*)buff + 5);
    }*/
    else
    {
	fputs("Haha!\r\n", stream);
    }
}
