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

#ifndef OPUSCODEC_INCLUDED
#define OPUSCODEC_INCLUDED

#include <stdint.h>
#include <stdio.h>

typedef struct OpusCodecData
{
    uint8_t version;
    uint8_t channels;
    uint16_t preskip;
    uint32_t originalRate;
    uint16_t outputGainDb;
} OpusCodecData;

struct AudioFileSource;
struct CodecData;

int OpusCodec_sample(struct AudioFileSource* source, struct CodecData* codecData, uint16_t* buff, int count);

void OpusCodec_printFrameData(uint8_t* data, int len, FILE* debugStream);

#endif
