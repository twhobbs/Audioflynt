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

#include "MsAdpcmCodec.h"
#include "AudioFile.h"
#include <stdio.h>

#define READBUFFLEN 50

const int16_t adaptationTable [] = { 
    230, 230, 230, 230, 307, 409, 512, 614, 
    768, 614, 512, 409, 307, 230, 230, 230 
    } ;

const int16_t adaptCoeff1 [] = { 256, 512, 0, 192, 240, 460, 392 } ;

const int16_t adaptCoeff2 [] = { 0, -256, 0, 64, 0, -208, -232 } ;

void msadpcmProcessNibble(uint8_t unibble, MsAdpcmCodecData* state, uint16_t* buff, int* countRead)
{
    int8_t nibble = (unibble & 0x8 ? 0xf0 | unibble : unibble);
 
    int32_t delta = state->delta;
    int32_t sample = state->sample1;
    sample *= adaptCoeff1[state->predictor];
    
    int32_t sampleb = state->sample2;
    sampleb *= adaptCoeff2[state->predictor];
    
    sample += sampleb;
    
    sample = sample >> 8;
    sample += nibble * delta;
    
    sample = sample > INT16_MAX ? INT16_MAX : (sample < INT16_MIN ? INT16_MIN : sample);
    
    buff[*countRead] = sample + 32768;
    *countRead = (*countRead) + 1;
    
    state->sample2 = state->sample1;
    state->sample1 = sample;
    
    delta = (adaptationTable[unibble] * delta);
    delta = delta >> 8;
    
    if (delta < 16)
    {
	delta = 16;
    }
    
    state->delta = delta;
    
    //printf("Delta: %d\n", *delta);
}

void msadpcmProcessByte(uint8_t byte, MsAdpcmCodecData* state, uint16_t* buff, int* countRead)
{
    msadpcmProcessNibble((byte >> 4) & 0xf, state, buff, countRead);
    msadpcmProcessNibble((byte & 0xf), state, buff, countRead);
}

void msadpcmReadBlock(struct AudioFileSource* source, MsAdpcmCodecData* state, uint16_t* buff, int* countRead)
{
    state->nextBlock += state->blocklen;
    
    source->readfunc(&state->predictor, 1, source->source);
    
    int16_t blockParams[3];
    source->readfunc(blockParams, 6, source->source);
    
    state->delta = blockParams[0];
    state->sample1 = blockParams[1];
    state->sample2 = blockParams[2];
    
    //fprintf(msadpcm_debugStream, "BlockParams: %ld %d %d\r\n", state->delta, state->sample1, state->sample2);
    
    buff[*countRead] = state->sample2 + 32768;
    *countRead = (*countRead) + 1;
    buff[*countRead] = state->sample1 + 32768;
    *countRead = (*countRead) + 1;
    
    state->curByte += 7;
   
}

int MsAdpcmCodec_sample(struct AudioFileSource* source, struct CodecData* codecData, uint16_t* buff, int count)
{
    int countRead = 0;
    if (codecData->msadpcm.curByte == 0)
    {
        codecData->msadpcm.nextBlock = 0;
    }
    
    if (count >= 2)
    {
        int done = 0;
        while (countRead < count && !done)
        {
            if (codecData->msadpcm.curByte == codecData->msadpcm.nextBlock)
            {
                msadpcmReadBlock(source, &codecData->msadpcm, buff, &countRead);
            }
            
            int len = codecData->msadpcm.nextBlock - codecData->msadpcm.curByte;
	    if (len > READBUFFLEN)
	    {
		len = READBUFFLEN;
	    }
	    
	    int nibblesLeft = (count - countRead) >> 1;
	    if (len > nibblesLeft)
	    {
		len = nibblesLeft;
	    }
            
            uint8_t b[READBUFFLEN];
            int byteRead = source->readfunc(b, len, source->source);
            codecData->msadpcm.curByte += byteRead;
	    
	    uint8_t i;
            for (i = 0; i < len; i++)
            {
                msadpcmProcessByte(b[i], &codecData->msadpcm, buff, &countRead);
            }

            done = byteRead == 0;
        }
        
        
    }
    
    return countRead;
}

void MsAdpcmCodec_rewind(struct AudioFileSource* source, struct CodecData* codecData)
{
    source->seekfunc(source->source, codecData->rewindSeek, SEEK_SET);
    codecData->msadpcm.curByte = 0;
    codecData->msadpcm.delta = 16;
}
