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

#ifndef AUDIOFILE_INCLUDED
#define AUDIOFILE_INCLUDED

#include <stdio.h>
#include "ToneGen.h"
#include "MsAdpcmCodec.h"
#include "OpusCodec.h"

#define AFORMAT_WAVE 1
#define AFORMAT_OGG 2
#define AFORMAT_MIDI 3

typedef struct CodecData
{
    union
    {
        ToneGenData tone;
        MsAdpcmCodecData msadpcm;
        OpusCodecData opus;
    };
    int rewindSeek;
} CodecData;

typedef struct AudioFileSource
{
    void* source;
    size_t (*readfunc)(void *dest, size_t size, void *source);
    int32_t (*seekfunc)(void *source, int32_t offset, int origin);
    int32_t (*tellfunc)(void *source);
    
} AudioFileSource;

typedef struct AudioFileHeader
{
    AudioFileSource source;
    
    int sampleRate;
    
    CodecData codecData;
        
    int (*samplefunc)(AudioFileSource* source, CodecData* codecData, uint16_t* buff, int count);
    
    void (*rewindfunc)(AudioFileSource* source, CodecData* codecData);
    
} AudioFileHeader;

void AudioFile_initialise(AudioFileHeader* header, FILE* debugStream, int format);

int AudioFile_sample(AudioFileHeader* header, uint16_t* buff, int count);

void AudioFile_rewind(AudioFileHeader* header);

#endif
