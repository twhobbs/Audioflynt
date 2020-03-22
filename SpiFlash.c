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

#include "SpiFlash.h"

#include <avr/cpufunc.h>
#include <avr/io.h>

#define FLASH_HOLD _BV(3)
#define FLASH_CS _BV(4)

#define SPI_MOSI _BV(5)
#define SPI_MISO _BV(6)
#define SPI_CLK _BV(7)

#define LED0 _BV(0)
#define LED1 _BV(1)

uint8_t spiTransceiveByte(uint8_t byte)
{
    SPIC.DATA = byte;
    while(!(SPIC.STATUS & SPI_IF_bm)/* && counter++ < 50000*/); // wait for transmit complete
    uint8_t result =  SPIC.DATA;
    
    return result;
}

void spiInit()
{
    PORTC.PIN4CTRL = PORT_OPC_WIREDANDPULL_gc;
    PORTC.OUTSET = FLASH_CS;
    
    PORTC.DIRSET = FLASH_HOLD | FLASH_CS | SPI_MOSI | SPI_CLK | LED0 | LED1;
    PORTC.OUTSET = FLASH_HOLD;
    
    
    //SPIC.CTRL = 0xD0;    // SPI master, clock idle low, data setup on trailing edge, data sampled on leading edge, double speed mode enabled
    SPIC.CTRL = SPI_PRESCALER_DIV4_gc | SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc;
    SPIC.INTCTRL = SPI_INTLVL_OFF_gc;
    
       
    SPIC.STATUS;    //both these are to take IF down
    SPIC.DATA;  //when I don't use interrupts
    
    PORTC.OUTCLR = FLASH_CS;
    spiTransceiveByte(0xF0); //Reset
    PORTC.OUTSET = FLASH_CS;
}

void spiflashGetDeviceInfo(cfi_header* header, cfi_ident* cfi)
{
    
    PORTC.OUTCLR = FLASH_CS;
    spiTransceiveByte(0x9F); //Read Identification
    
    uint8_t* data = (uint8_t*)header;
    
    for (uint8_t i = 0; i < sizeof(cfi_header); i++)
    {
	data[i] = spiTransceiveByte(0);
    }
    
    for (uint8_t i = 0; i < (0x10 - sizeof(cfi_header)); i++)
    {
	spiTransceiveByte(0);
    }
    
    data = (uint8_t*)cfi;
    
    for (uint8_t i = 0; i < sizeof(cfi_ident); i++)
    {
	data[i] = spiTransceiveByte(0);
    }
    
    
    PORTC.OUTSET = FLASH_CS;

}

void spiflashWriteBlock(uint32_t address, uint8_t* data)
{
    PORTC.OUTCLR = FLASH_CS;
    
    spiTransceiveByte(0x12); //Page program
    
    spiTransceiveByte((address >> 24) & 0xff);
    spiTransceiveByte((address >> 16) & 0xff);
    spiTransceiveByte((address >> 8 ) & 0xff);
    spiTransceiveByte((address      ) & 0xff);
    
    
    for (int i = 0; i < 256; i++)
    {
	spiTransceiveByte(data[i]);
    }
    
	
    PORTC.OUTSET = FLASH_CS;
}

void spiflashReadData(uint32_t address, uint8_t* data, int len)
{
    PORTC.OUTCLR = FLASH_CS;
    
    spiTransceiveByte(0x13); //Read
    
    spiTransceiveByte((address >> 24) & 0xff);
    spiTransceiveByte((address >> 16) & 0xff);
    spiTransceiveByte((address >> 8 ) & 0xff);
    spiTransceiveByte((address      ) & 0xff);
    
    for (int i = 0; i < len; i++)
    {
	data[i] = spiTransceiveByte(0);
    }
    
    PORTC.OUTSET = FLASH_CS;
}

void spiflashEraseDevice()
{
    PORTC.OUTCLR = FLASH_CS;          // assert SS pin (active low)
    spiTransceiveByte(0x60);          //Bulk erase command
    PORTC.OUTSET = FLASH_CS;          // de-assert SS pin
}

uint8_t spiflashGetStatusRegister()
{
    PORTC.OUTCLR = FLASH_CS;
    spiTransceiveByte(0x05);
    uint8_t statusRegister = spiTransceiveByte(0x0);
    PORTC.OUTSET = FLASH_CS;
    return statusRegister;
}

void spiflashSetWriteEnabled(int writeEnabled)
{
    PORTC.OUTCLR = FLASH_CS;
    
    if (writeEnabled)
    {
	spiTransceiveByte(0x06); //Write enable
    }
    else
    {
	spiTransceiveByte(0x04); //Write disable
    }
    
    PORTC.OUTSET = FLASH_CS;
}
