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

#include "RawCodec.h"

#include "AudioFile.h"

int RawCodec_sample(struct AudioFileSource* source, struct CodecData* codecData, uint16_t* buff, int count)
{
    int readBytes = source->readfunc(buff, count * 2, source->source);
    
    //16bit PCM in wave is always signed, so convert to unsigned
    int i;
    for (i = 0; i < readBytes; i++)
    {
        int16_t* sb = (int16_t*)&buff[i];
        buff[i] = *sb + 32768;
    }
    
    return readBytes / 2;
}

void RawCodec_rewind(struct AudioFileSource* source, struct CodecData* codecData)
{
    source->seekfunc(source->source, codecData->rewindSeek, SEEK_SET);
}
