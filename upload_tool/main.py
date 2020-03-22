#!/usr/bin/python


#    Serial Upload utility for Audioflynt
#    Interfaces with an Audioflynt board programmed with the "serial_debug" firmware
#    Copyright (C) 2015 Tim Hobbs
#
#    Audioflynt is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Audioflynt is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with Audioflynt.  If not, see <http://www.gnu.org/licenses/>.


import serial
import sys
import crcmod

def waitfordevice():
    ser.write(b"stat\r")
    busy = int(ser.readline().strip())
    while busy & 1:
        ser.write(b"stat\r")
        busy = int(ser.readline().strip())

print('Opening serial device ' + sys.argv[1] + "...")

ser = serial.Serial(sys.argv[1], baudrate=4000000, timeout=1, bytesize=8, parity='N', stopbits=1, xonxoff=0, rtscts=0)

if ser.isOpen():
    print("Opened serial port")

ser.write(b"toplel\r")

line = ser.readline().strip()

if line == "Haha!":
    print("Opened Audioflynt successfully")
    
# Read Flash device size
ser.write(b"geom\r")

geom = ser.readline().strip()
geomsplit = geom.split(b':')

if (len(geomsplit) != 2):
    print("Bad geometry values")
    sys.exit(1)

devsize = 2**int(geomsplit[0])
blocksize = 2**int(geomsplit[1])

print("Dev Size: " + str(devsize))
print("Block Size: " + str(blocksize))

if (len(sys.argv) == 3):
    infile = open(sys.argv[2], "rb")
    data = infile.read()
    infile.close()
    
    ser.write(b"wren\r")
    
    waitfordevice()
        
    if len(data) == devsize:
        print("Data length matches device size")
        
        blockcount = int(devsize/blocksize)
        index = 0
        
        #Matches the CRC16 built into ATXMEGA
        crc16 = crcmod.predefined.Crc('xmodem')
            
        for i in range (0, blockcount):
            
            block = data[index:index+blocksize]
            index = index + blocksize
                        
            #print("Writing block", i, "with CRC32", crc, ", len:", len(block))
            
            setblockcmd = "block " + str(i) + "\r"
            ser.write(setblockcmd.encode('ascii'))
            blockset = int(ser.readline().strip())
            
            if blockset != i:
                print("Block set fail:", blockset)
                sys.exit(1)
            
            
            ser.write(b"data\r")
            ser.write(block)
            response = ser.readline().strip()

            ser.write(b"wren\r")

            waitfordevice()

            ser.write(b"prog\r")
        
            waitfordevice()
        
            ser.write(b"fcrc\r")

            crc16.update(block)
            crc = crc16.crcValue
            crc16 = crc16.new()
            
            response = int(ser.readline().strip())
            
            if response != crc:
                print("CRC Fail at", i, ", expected", crc, "got", response)
                sys.exit(1)
            
            if i % 128 == 0:
                print(str(float(100*i)/blockcount) + "% complete")
            

    print("Done")
ser.close()
