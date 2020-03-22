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

#include "ToneGen.h"
#include "AudioFile.h"
#include <math.h>

int ToneGen_sample(struct AudioFileSource* source, struct CodecData* codecData, uint16_t* buff, int count)
{
    uint32_t countLeft = codecData->tone.count - codecData->tone.i;
    count = count < countLeft ? count : countLeft;
    
    int i;
    for (i = 0; i < count; i++)
    {
        float x = ((codecData->tone.i + i) * codecData->tone.spd);
        buff[i] = (sinf(x) * 10000) + 32768;
    }
    
    codecData->tone.i += count;
    
    return count;
}
