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

#ifndef MSADPCMCODEC_INCLUDED
#define MSADPCMCODEC_INCLUDED

#include <stdint.h>
#include <stdio.h>

typedef struct MsAdpcmCodecData
{
    uint8_t predictor;
    int16_t sample1;
    int16_t sample2;
    int32_t delta;
    
    int16_t blocklen;
    uint32_t curByte;
    uint32_t nextBlock;
    uint32_t decodedSamples;
} MsAdpcmCodecData;

extern const int16_t adaptationTable [];

extern const int16_t adaptCoeff1 [];

extern const int16_t adaptCoeff2 [];

void adpcmProcessByte(uint8_t byte, MsAdpcmCodecData* state, void (*sampleCallback)(int16_t));

struct AudioFileSource;
struct CodecData;

int MsAdpcmCodec_sample(struct AudioFileSource* source, struct CodecData* codecData, uint16_t* buff, int count);

void MsAdpcmCodec_rewind(struct AudioFileSource* source, struct CodecData* codecData);

FILE* msadpcm_debugStream;

#endif
