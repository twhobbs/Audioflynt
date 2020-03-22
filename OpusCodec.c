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

#include "OpusCodec.h"
#include <stdio.h>

int OpusCodec_sample(struct AudioFileSource* source, struct CodecData* codecData, uint16_t* buff, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        buff[i] = 32768;
    }
    return count;
}

void OpusCodec_printFrameData(uint8_t* data, int len, FILE* debugStream)
{
    uint8_t b0 = data[0];
    uint32_t rng = 128;
    uint32_t val = (127 - (b0>>1));
}
