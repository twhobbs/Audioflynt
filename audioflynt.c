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


#include "audioflynt.h"

void boardSetup()
{
    OSC.XOSCCTRL = 0x4B;			// 2-9 MHz crystal; 0.4-16 MHz XTAL w/16K CLK startup
    OSC.CTRL = 0x08;       			// enable external crystal oscillator 
    while(!(OSC.STATUS & OSC_XOSCRDY_bm));	// wait for osc ready
    OSC.PLLCTRL = 0xC8;				// PLL->XOSC, 8x mult
    OSC.CTRL = 0x18;        			// Enable PLL & External Oscillator 
    while(!(OSC.STATUS & OSC_PLLRDY_bm));	// wait for PLL ready
    _PROTECTED_WRITE(CLK.CTRL, CLK_SCLKSEL_PLL_gc);// switch to PLL clock
    OSC.CTRL &= ~OSC_RC2MEN_bm;	

    AUDIO_PORT.DIR = GAIN0 | GAIN1;
    AUDIO_PORT.OUT = 0;
    
    DACB.CTRLB = DAC_CHSEL_SINGLE_gc; // Only channel 0 operates
    DACB.CTRLC = DAC_REFSEL_INT1V_gc; // AVcc is the DAC reference voltage
    DACB.CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm; // Enable DAC and channel 0
    DACB.CH0DATA = 0; // Output 0 Volts (apart from gain and offset error)
    
    PORTC.DIR = LED0 | LED1 | LED2;
    PORTC.OUT = 0;

    PORTA.DIR = POWER_LIGHT;
    
    PORTA.PIN1CTRL |= PORT_OPC_PULLUP_gc;
    
    PORTA.PIN2CTRL = PORT_OPC_PULLUP_gc;
    PORTA.PIN3CTRL = PORT_OPC_PULLUP_gc;
    PORTA.PIN4CTRL = PORT_OPC_PULLUP_gc;
    PORTA.PIN5CTRL = PORT_OPC_PULLUP_gc;
    PORTA.PIN6CTRL = PORT_OPC_PULLUP_gc;
    PORTA.PIN7CTRL = PORT_OPC_PULLUP_gc;
            
    
    
    TCC0.CNT = 0;// Zeroise count
    TCC0.PER = F_CPU/22050; //Period      
    TCC0.CTRLA = TC_CLKSEL_DIV1_gc; //Divider 
    TCC0.INTCTRLA = TC_OVFINTLVL_HI_gc; //Liow level interrupt
    TCC0.INTFLAGS = 0x01; // clear any initial interrupt flags 
    TCC0.CTRLB = TC_WGMODE_NORMAL_gc; // Normal operation
    
    PMIC.CTRL |= PMIC_HILVLEN_bm;
}
