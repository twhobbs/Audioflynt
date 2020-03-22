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

#ifndef MIDICODEC_INCLUDED
#define MIDICODEC_INCLUDED

#include "AudioFile.h"
#include <stdint.h>
#include <stdio.h>


struct AudioFileSource;
struct CodecData;

int MidiCodec_sample(struct AudioFileSource* source, struct CodecData* codecData, uint16_t* buff, int count);



uint32_t Midi_ReadVariableLength(uint8_t* track, size_t* pos);

int Midi_ReadEvent(uint8_t* track, size_t* pos, uint8_t* lastStatus);

void Midi_dumpMidiTrack(uint8_t* track, size_t len, FILE* debugStream);

void Midi_initialiseMidi(AudioFileHeader* header, FILE* debugStream);


#endif
