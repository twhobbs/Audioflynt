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

#ifndef TONEGEN_INCLUDED
#define TONEGEN_INCLUDED

#include <stdint.h>

typedef struct ToneGenData
{
    float spd;
    int32_t i;
    int32_t count;
} ToneGenData;

struct AudioFileSource;
struct CodecData;

int ToneGen_sample(struct AudioFileSource* source, struct CodecData* codecData, uint16_t* buff, int count);

#endif
