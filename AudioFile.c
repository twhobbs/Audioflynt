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

#include "AudioFile.h"
#include "RawCodec.h"
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

void AudioFile_initialiseWav(AudioFileHeader* header, FILE* debugStream)
{
    char headerdat[36];
    
    AudioFileSource source = header->source;
    
    (*source.readfunc)(headerdat, 36, source.source);
    
    uint32_t* headerSize = (uint32_t*)&headerdat[16];
    uint16_t* format = (uint16_t*)&headerdat[20];
    uint16_t* channels = (uint16_t*)&headerdat[22];
    uint32_t* sampleRate = (uint32_t*)&headerdat[24];
    uint16_t* blockAlign = (uint16_t*)&headerdat[32];
    uint16_t* bitsPerSample = (uint16_t*)&headerdat[34];

    (*source.seekfunc)(source.source, *headerSize + 20, SEEK_SET);

    char dataHeader[8];
    size_t read = (*source.readfunc)(dataHeader, 8, source.source);
    uint32_t* dataSize = (uint32_t*)&dataHeader[4];

    while (strncmp(dataHeader, "data", 4) != 0 && read > 0)
    {
        (*source.seekfunc)(source.source, *dataSize, SEEK_CUR);
        read = (*source.readfunc)(dataHeader, 8, source.source);
    }
    
    header->sampleRate = *sampleRate;
    header->codecData.rewindSeek = (*source.tellfunc)(source.source);
    
    if (*format == 1 && *channels == 1 && *bitsPerSample == 16) 
    {
        header->samplefunc = RawCodec_sample;
        header->rewindfunc = RawCodec_rewind;
    }    
    else if (*format == 2 && *channels == 1)
    {
        header->samplefunc = MsAdpcmCodec_sample;
        header->codecData.msadpcm.blocklen = *blockAlign;
        header->rewindfunc = MsAdpcmCodec_rewind;
        (*header->rewindfunc)(&header->source, &header->codecData);
    }
    else
    {
        header->sampleRate = 22050;
        header->codecData.tone.i = 0;
        header->codecData.tone.spd = (2 * M_PI * 440.0f) / header->sampleRate;
        header->codecData.tone.count = 3*header->sampleRate;
        header->samplefunc = ToneGen_sample;
    }
    
    
}

#ifdef ENABLE_OPUS
int AudioFile_readOggPage(AudioFileHeader* header, FILE* debugStream)
{
    AudioFileSource source = header->source;
    
    uint8_t version, type, segments;
    uint32_t pageSequence;
    
    {
        char headerdat[27];
        (*source.readfunc)(headerdat, 27, source.source);
        
        version = ((uint8_t*)&headerdat)[4];
        type = ((uint8_t*)&headerdat)[5];
        segments = ((uint8_t*)&headerdat)[26];
        pageSequence = *((uint32_t*)&headerdat[18]);
    }
    
    if (version != 0 || type & 4) //check for end of stream
    {
        return 1;
    }
    
    int i;
    uint16_t amountToSkip = 0;
    //uint8_t maxSegLength = 0;
    uint8_t segLengths[256];
    (*source.readfunc)(segLengths, segments, source.source);
    for (i = 0; i < segments; i++)
    {
        amountToSkip += segLengths[i];
    }
    
    if (pageSequence == 0 && amountToSkip >= 18)
    {
        header->codecData.opus.version = 0;
        
        char opushead[18];
        (*source.readfunc)(opushead, 18, source.source);
        
        if (strncmp(opushead, "OpusHead", 8) == 0)
        {
            header->codecData.opus.version = *((uint8_t*)&opushead[8]);
            header->codecData.opus.channels = *((uint8_t*)&opushead[9]);
            header->codecData.opus.preskip = *((uint16_t*)&opushead[10]);
            header->codecData.opus.originalRate = *((uint32_t*)&opushead[12]);
            header->codecData.opus.outputGainDb = *((uint16_t*)&opushead[16]);
            
            fprintf(debugStream, "Version: %u\r\n", header->codecData.opus.version);
            fprintf(debugStream, "Channels: %u\r\n", header->codecData.opus.channels);
            fprintf(debugStream, "Preskip: %u\r\n", header->codecData.opus.preskip);
            fprintf(debugStream, "Original Rate: %lu\r\n", (unsigned long)header->codecData.opus.originalRate);
            fprintf(debugStream, "Output Gain Db: %u\r\n", header->codecData.opus.outputGainDb);
            
        }   
        
        (*source.seekfunc)(source.source, amountToSkip-18, SEEK_CUR);
    }
    else if (pageSequence > 1 && header->codecData.opus.version == 1)
    {
        uint8_t buff[30];
        int i;
        for (i = 0; i < segments; i++)
        {
            (*source.readfunc)(buff, 30, source.source);
            
            uint8_t config = buff[0] >> 3;
            uint8_t s = (buff[0] >> 2) & 1;
            uint8_t c = buff[0] & 3;
            
            fprintf(debugStream, "Packet %u config: %u, ", i, config);
            fprintf(debugStream, s ? "stereo" : "mono");
            fprintf(debugStream, ", code: %u\r\n", c);
            
            if (s == 0 && c == 0 && config == 28)
            {
                OpusCodec_printFrameData(buff + 1, 29, debugStream);
            }
            else
            {
                fprintf(debugStream, "Unsupported frame type\r\n");    
            }
        }
        
    }
    else
    {
        (*source.seekfunc)(source.source, amountToSkip, SEEK_CUR);
    }
    
    return 0;
}

void AudioFile_initialiseOgg(AudioFileHeader* header, FILE* debugStream)
{
    AudioFile_readOggPage(header, debugStream);
    int eos = AudioFile_readOggPage(header, debugStream);
    
    while (!eos)
    {
        eos = AudioFile_readOggPage(header, debugStream);
    }
    
    if (header->codecData.opus.version == 1)
    {
        header->sampleRate = 22050;
        header->codecData.tone.i = 0;
        header->codecData.tone.spd = (2 * M_PI * 440.0f) / header->sampleRate;
        header->codecData.tone.count = header->sampleRate / 4;
        header->samplefunc = ToneGen_sample;
    }
    else
    {
        header->sampleRate = 22050;
        header->codecData.tone.i = 0;
        header->codecData.tone.spd = (2 * M_PI * 440.0f) / header->sampleRate;
        header->codecData.tone.count = header->sampleRate;
        header->samplefunc = ToneGen_sample;
    }
}
#endif

void AudioFile_initialise(AudioFileHeader* header, FILE* debugStream, int format)
{
    //printf("AudioFile Initialise: %d\n", format);
    header->sampleRate = 0;
    if (format == AFORMAT_WAVE)
    {
        AudioFile_initialiseWav(header, debugStream);
    }
#ifdef ENABLE_OPUS
    else if (format == AFORMAT_OGG)
    {
        AudioFile_initialiseOgg(header, debugStream);
    }
#endif
#ifdef ENABLE_MIDI
    else if (format == AFORMAT_MIDI)
    {
        //printf("preInit MIDI\n");
        Midi_initialiseMidi(header, debugStream);
    }
#endif
}

int AudioFile_sample(AudioFileHeader* header, uint16_t* buff, int count)
{
    return header->samplefunc(&header->source, &header->codecData, buff, count);
}

void AudioFile_rewind(AudioFileHeader* header)
{
    header->rewindfunc(&header->source, &header->codecData);   
}
