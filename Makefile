MCU=atxmega16a4u
CC=avr-gcc
CFLAGS=-Os -lm -Wall -mmcu=$(MCU) -std=c99
OBJ2HEX=avr-objcopy 
UISP=/usr/local/bin/uisp 
TARGET=main
AVRDUDE_BASE=avrdude -p x16a4u -c dragon_pdi

OBJS = main.o audioflynt.o SpiFlash.o FAT.o AudioFile.o RawCodec.o MsAdpcmCodec.o ToneGen.o 

#program : $(TARGET).hex
#	$(UISP) -dprog=stk500 -dserial=/dev/ttyS1 --erase -dpart=atmega32
#	$(UISP) -dprog=stk500 -dserial=/dev/ttyS1 --upload -dpart=atmega32 \
#		if=$(TARGET).hex -v=2


default: $(TARGET).hex

##main.o : main.c
##	$(CC) $(CFLAGS) main.c -o main.o

main.obj : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o main.obj
	avr-size  --format=avr --mcu=$(MCU) main.obj

main.hex : main.obj
	$(OBJ2HEX) -R .eeprom -O ihex $< $@
	
clean :
	rm -f *.hex *.obj *.o

program: main.hex
	$(AVRDUDE_BASE) -U flash:w:main.hex -B 1

reset:
	$(AVRDUDE_BASE)