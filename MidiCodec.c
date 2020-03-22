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

#include "MidiCodec.h"

#include "AudioFile.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

uint32_t Midi_ReadVariableLength(uint8_t* track, size_t* pos)
{
    uint32_t val = track[*pos] & 0x7f;
    
    while (track[*pos] & 0x80)
    {
        *pos = *pos + 1;
    
        val = (val << 7) | (track[*pos] & 0x7f);
    }
    
    *pos = *pos + 1;
    
    return val;    
}

int Midi_ReadEvent(uint8_t* track, size_t* pos, uint8_t* lastStatus)
{
    uint8_t event = track[*pos];
    *pos = *pos + 1;
    
    if ((event & 0xf0) == 0xf0)
    {
        
        uint8_t type = 0;
        if (event == 0xff)
        {
            type = track[*pos];
            *pos = *pos + 1;
        }
        uint32_t len = Midi_ReadVariableLength(track, pos);
        *pos += len;
        
        printf("SysEx Len: %lu\n", (unsigned long)len);
    }
    else if ((event & 0x80) || (*lastStatus & 0x80)) // MIDI event
    {
        if (!(event & 0x80))
        {
            event = *lastStatus;
        }
        
        *lastStatus = event;
        
        uint8_t status = event & 0xf0;
        uint8_t channel = event & 0x0f;
        
        if (status == 0x80)
        {
            //printf("Note off\n");
            *pos = *pos + 2;
        }
        else if (status == 0x90)
        {
            //printf("Note on\n");
            *pos = *pos + 2;
        }
        else if (status == 0xA0)
        {
            printf("Aftertouch\n");
            *pos = *pos + 2;
        }
        else if (status == 0xB0)
        {
            //printf("Control change\n");
            *pos = *pos + 2;
        }
        else if (status == 0xC0)
        {
            printf("Program change\n");
            *pos = *pos + 1;
        }
        else if (status == 0xD0)
        {
            printf("Channel pressure\n");
            *pos = *pos + 1;
        }
        
        else if (status == 0xE0)
        {
            //printf("Pitch Wheel\n");
            *pos = *pos + 2;
        }
        else
        {
            return status;
        }
    }
    else
    {
        return event;
    }
    
    return 0;
}

void Midi_dumpMidiTrack(uint8_t* track, size_t len, FILE* debugStream)
{
    uint8_t lastStatus = 0;
    
    size_t pos = 0;
    while (pos < len)
    {
        uint32_t val = Midi_ReadVariableLength(track, &pos);
        //fprintf(debugStream, "Var Length: %lu\r\n", (unsigned long)val);
        
        uint8_t event = Midi_ReadEvent(track, &pos, &lastStatus);
        if (event != 0)
        {
            fprintf(debugStream, "Unhandled MIDI event: %x\r\n", event);
            return;
        }
    }
}

void Midi_initialiseMidi(AudioFileHeader* header, FILE* debugStream)
{
    printf("Init MIDI\n");
    
    char chunkHeader[4];
    uint32_t chunklen = 0;
    AudioFileSource source = header->source;
    
    size_t read = 1;
    
    while (read)
    {
        (*source.readfunc)(chunkHeader, 4, source.source);
        read = (*source.readfunc)(((uint8_t*)&chunklen), 4, source.source);
        chunklen = ntohl(chunklen);
    
        if (read)
        {
            fprintf(debugStream, "MIDI chunk name: %.4s, length: %lu\r\n", chunkHeader, (unsigned long)chunklen);
            if (strncmp(chunkHeader, "MThd", 4) == 0)
            {
                uint16_t format;
                uint16_t numTracks;
                uint16_t division;
                
                (*source.readfunc)((&format), 2, source.source);
                (*source.readfunc)((&numTracks), 2, source.source);
                (*source.readfunc)((&division), 2, source.source);
                
                format = ntohs(format);
                numTracks = ntohs(numTracks);
                division = ntohs(division);
                
                fprintf(debugStream, "MIDI header: Format: %u, NumTracks: %u, Division: %u\r\n", format, numTracks, division);
            }
            else if (strncmp(chunkHeader, "MTrk", 4) == 0)
            {
                uint8_t* track = malloc(chunklen);
                (*source.readfunc)(track, chunklen, source.source);
                
                int i;
                for (i = 0; i < 50; i++)
                {
                    printf("%x ", track[i]);
                }
                printf("\r\n");
                
                Midi_dumpMidiTrack(track, chunklen, debugStream);
                
                free(track);
            }
            else
            {
                (source.seekfunc)(source.source, chunklen, SEEK_CUR);    
            }
            
            
            
        }
    }
}
