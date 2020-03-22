/*
 *    Player Test utility for Audioflynt
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

#include <ao/ao.h>
#include "../AudioFile.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#define BUFFLEN 32

size_t stdfile_readfunc(void *dest, size_t size, void *source)
{
    return fread(dest, 1, size, (FILE*)source);
}

int stdfile_seekfunc(void *source, int offset, int origin)
{
    return fseek((FILE*)source, offset, origin);
}

int32_t stdfile_tellfunc(void* source)
{
    return ftell((FILE*)source);
}

void convertBuffer(uint16_t* in, int16_t* out, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        out[i] = in[i] - 32768;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s FILE\n", argv[0]);
        return 1;
    }
    
    FILE* inputFile = fopen(argv[1], "r");
    if (inputFile == NULL)
    {
        printf("Bad file provided\n");
        return 1;
    }
    
    char* ext = strchr(argv[1], '.');
    
    int aformat = 0;
    
    if (ext != NULL)
    {
        if (strncmp(ext+1, "wav", 3) == 0)
        {
            aformat = AFORMAT_WAVE;
            printf("WAVE Format\n");
        }
        else if (strncmp(ext+1, "ogg", 3) == 0 || strncmp(ext+1, "opus", 4) == 0)
        {
            aformat = AFORMAT_OGG;
            printf("OGG Format\n");
        }
        else if (strncmp(ext+1, "mid", 3) == 0)
        {
            aformat = AFORMAT_MIDI;
            printf("MIDI Format\n");
        }
        
    }
    
    if (aformat == 0)
    {
        printf("Couldn't determine audio format\n");
        return 1;
    }
    
    AudioFileHeader afh;
    afh.source.source = inputFile;
    afh.source.readfunc = stdfile_readfunc;
    afh.source.seekfunc = stdfile_seekfunc;
    afh.source.tellfunc = stdfile_tellfunc;
    
    AudioFile_initialise(&afh, stdout, aformat);
    
    ao_initialize();
    
    //int driver = ao_default_driver_id();
    int driver = ao_driver_id("alsa");
    
    if (afh.sampleRate == 0)
    {
        printf("Bad audio file\n");
        return 1;
    }
    else if (driver == -1)
    {
        printf("Failed to find default audio device\n");
    }
    else
    {
        ao_sample_format format;
        format.bits = 16;
        format.rate = afh.sampleRate;
        format.channels = 1;
        format.byte_format = AO_FMT_LITTLE;
        format.matrix = "M";
        
        printf("Trying to open format with rate: %d\n", format.rate);
        
        uint16_t buff[BUFFLEN];
        
        int16_t sbuff[BUFFLEN];
        
        ao_device* aodev = ao_open_live(driver, &format, NULL);
        
        int printCount = 0;
        
        FILE* outfile = fopen("out.raw", "w");
        
        if (aodev == NULL)
        {
            printf("Failed to open audio device\n");
        }
        else
        {
            
            int count;
            do
            {
                count = AudioFile_sample(&afh, buff, BUFFLEN);

                fwrite((char*)&buff, 2, count, outfile);
                
                convertBuffer(buff, sbuff, BUFFLEN);
                
                
                
                ao_play(aodev, (char*)sbuff, count*sizeof(int16_t));
            }
            while (count == BUFFLEN);
            
            
            ao_close(aodev);
        }
        
        fclose(outfile);
    }
    
    
    
    ao_shutdown();
    
    fclose(inputFile);
    
    return 0;
}
