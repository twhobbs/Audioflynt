#include "audioflynt.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h> 
#include <avr/cpufunc.h> 

#include <math.h>

#include "AudioFile.h"
#include "SpiFlash.h"
#include "FAT.h"

#include <string.h>

//#include "sine.h"

const uint8_t songButtons[] = {SONG_BUTTON_0, SONG_BUTTON_1, SONG_BUTTON_2, SONG_BUTTON_3, SONG_BUTTON_4};
const uint8_t buttonMask = SONG_BUTTON_0 | SONG_BUTTON_1 | SONG_BUTTON_2 | SONG_BUTTON_3 | SONG_BUTTON_4;

AudioFileHeader audioFileHeaders[11];
FATFile audioFiles[11];
const char* audioNames[11] = {"00.wav", "01.wav", "02.wav", "03.wav", "04.wav", "10.wav", "11.wav", "12.wav", "13.wav", "14.wav", "15.wav"};

#define BUFFLEN 100

volatile uint16_t outbuf1[BUFFLEN];
volatile uint16_t outbuf2[BUFFLEN];
volatile uint16_t* curbuf = 0;
volatile uint16_t* writebuf = outbuf1;
volatile uint16_t bufpos = 0;
volatile uint8_t bufready = 0;
volatile uint8_t songIndex = 0;
volatile uint16_t sinepos = 0;

void invertLED()
{
    PORTA.OUTTGL = POWER_LIGHT;
}

size_t fatReadFunc(void *dest, size_t size, void *source)
{
    return fatRead(dest, size, (FATFile*) source);
}

int32_t fatSeekFunc(void *source, int32_t offset, int origin)
{
    return fatSeek((FATFile*) source, offset, origin);
}

int32_t fatTellFunc(void* source)
{
    return ((FATFile*) source)->offset;
}

int openAudioFile(const char* filename, FAT_Header* fatHeader, FATFile* fatFile, AudioFileHeader* audioHeader)
{
    FAT_DirEntry dirEntry;
    
    int found = fatFindFile(filename, fatHeader, &dirEntry);
    if (found)
    {
        *fatFile = fatOpenFile(fatHeader, &dirEntry);
        
        audioHeader->source.source = fatFile;
        audioHeader->source.readfunc = fatReadFunc;
        audioHeader->source.seekfunc = fatSeekFunc;
        audioHeader->source.tellfunc = fatTellFunc;
        
        AudioFile_initialise(audioHeader, 0, AFORMAT_WAVE);
    }
    return found;
}

void playpart(uint8_t* done, int audioIndex);

int main() 
{
    cli();
    
    boardSetup();
    
    spiInit();
    
    
    
    _delay_us(10);
    
    FAT_Header fatHeader;
    spiflashReadData(0, (uint8_t*)&fatHeader, sizeof(FAT_Header));

    int found = 0;
    
    for (int i = 0; i < 11; i++)
    {
        found += openAudioFile(audioNames[i], &fatHeader, &audioFiles[i], &audioFileHeaders[i]);
    }

    //uint8_t on = 0;
    //uint16_t value = 0;
    //uint8_t lastPortValue = PORTA.IN;
    
    //uint8_t sineIndex = 0;
       
    if (found == 11) 
    {
        PORTA.OUTSET = POWER_LIGHT;
    }
    
    sei();
    
    uint8_t done = 0;
    uint8_t playing = 0;
    uint8_t selection = 0;
    
    uint8_t toggleNoPlay = 0;
    
    uint16_t toggleCounter = 0;
    
    uint8_t toggleOn = 0;
        
    while (1)
    {
        uint8_t portValue = PORTA.IN;
        AUDIO_PORT.OUT = ((portValue & KEY_SWITCH) == 0) ? GAIN0 | GAIN1 : 0;
        
        uint8_t playWhip = 0;
        
	//while (!(DMA.INTFLAGS & DMA_CH0TRNIF_bm));
        //DMA.INTFLAGS = DMA_CH0TRNIF_bm;

        
        
        if ((portValue & TOGGLE_BUTTON) == 0)
        {
            if (toggleCounter < 50000)
            {
                toggleCounter++;
            }
            else
            {
                toggleOn = 1;
            }
        }
        else
        {
            toggleOn = 0;
            toggleCounter = 0;
        }
        
        if ((portValue & buttonMask) != buttonMask || (playing && toggleOn)) 
        {
            if (!playing)
            {
                for (uint8_t i = 0; i < 5; i++)
                {
                    if ((portValue & songButtons[i]) == 0)
                    {
                        selection = i;
                    }
                }
                
                if (toggleOn)
                {
                    selection += 5;
                }
                
                playing = 1;
                toggleNoPlay = 0;
            }
            
            if (!done) 
            {
            
                playpart(&done, selection);
            }
        }
        else if (toggleOn)
        {
            toggleNoPlay = 1;
        }
        else 
        {
            done = 0;
            
            if (playing) 
            {
                playing = 0;
                bufpos = 0;
                //PORTC.OUTCLR = LED1;
                AudioFile_rewind(&audioFileHeaders[selection]);
                //PORTC.OUTSET = LED1;
                
                //PORTC.OUTTGL = LED0;
            }
            else if (toggleNoPlay) 
            {
                toggleNoPlay = 0;
                playWhip = 1;
            }
            //sineIndex = 0;
        }
        
        if (playWhip)
        {
            playWhip = 0;
            uint8_t whipDone = 0;
            
            bufpos = 0;
            
            while (!whipDone)
            {
                playpart(&whipDone, 10);
            }
            
            AudioFile_rewind(&audioFileHeaders[10]);
            
        }
        
        _NOP();
    }
}

void playpart(uint8_t* done, int audioIndex)
{
    while (bufready)
    {
        _NOP();
    }

    uint16_t len = AudioFile_sample(&audioFileHeaders[audioIndex], writebuf, BUFFLEN);
    *done = len < BUFFLEN;
    bufready = 1;
}

ISR(TCC0_OVF_vect)
{
    if (curbuf != 0)
    {
        DACB.CH0DATA = curbuf[bufpos] >> 4;
        ++bufpos;
        
        if (bufpos >= BUFFLEN) 
        {
            curbuf = 0;
            bufpos = 0;
        }
    }
    else 
    {
        DACB.CH0DATA = 2048;
        if ((PORTA.IN & buttonMask) != buttonMask)
        {
            //PORTA.OUTTGL = POWER_LIGHT;
        }
    }
    
    
    if (curbuf == 0 && bufready == 1)
    {
        curbuf = writebuf;
        if (writebuf == outbuf1)
        {
            writebuf = outbuf2;
        }
        else 
        {
            writebuf = outbuf1;
        }
        bufready = 0;
    }

}
