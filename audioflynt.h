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

#ifndef AUDIOFLYNT_INCLUDED
#define AUDIOFLYNT_INCLUDED

#define F_CPU 32E6

#include <avr/io.h>

#define AUDIO_PORT PORTB
#define GAIN0 _BV(0)
#define GAIN1 _BV(1)
#define AUDIO _BV(2)

#define LED0	_BV(0)
#define LED1	_BV(1)
#define LED2	_BV(2)

#define POWER_LIGHT _BV(0)
#define KEY_SWITCH _BV(1)

#define SONG_BUTTON_0 _BV(2)
#define SONG_BUTTON_1 _BV(3)
#define SONG_BUTTON_2 _BV(4)
#define SONG_BUTTON_3 _BV(5)
#define SONG_BUTTON_4 _BV(6)
#define SONG_BUTTON_5 _BV(7)

void boardSetup();

#endif
