# Audioflynt

ATXMEGA16A4U based audio playback board + software.

Two board sizes are provided, Large (audioflynt) and Small (audioflynt2).

## Features

- WAV file playback with PCM and MS-ADPCM codecs
- FAT16 filesystem support
- Onboard flash memory for storing audio files
- USB interface for configuring flash memory

Large board additional features:

- TPA3111D1PWP Class-D amplifier
- Switching power regulator
- Space for two MOSFETs

## Requirements

### Packages

Audioflynt has been tested on Ubuntu 17.10. The required packages to build the firmwares, player test, and upload tool are:

`make gcc-avr avr-libc avrdude libao-dev python python-pip`

### Upload Tool requirements

The upload tool is a Python application which connects to the serial_debug firmware. The required pip packages can be installed
by running `pip install -r requirements.txt` in the `software/upload_tool` directory.

### LUFA

The serial_debug firmware requires [LUFA](http://www.fourwalledcubicle.com/LUFA.php), a USB library for AVR by Dean Camera.
Download the LUFA 140928 package, extract it, and change the `LUFA_PATH` variable in `software/serial_debug/makefile` to 
point to the `LUFA` directory within.

### Board Schematics

The Audioflynt circuits and PCBs were designed in Eagle.


## Building

To build the player test utility, run `make` in the `software/player` directory.

To build the player firmware, run `make` in the `software/` directory.

To build the serial_debug firmware, run `make` in the `software/serial_debug` directory.

## Steps to get audio on the board

Assuming you have a working Audioflynt board:

1. Create a FAT16 image, exactly the size of the onboard flash chip.
2. If you are using the large version of the board, provide a power source (small version uses USB power).
3. Connect the board to your computer using a USB cable.
4. Burn the serial_debug firmware to the board using a compatible programmer connected to the PDI port.
5. Wait until the Audioflynt's serial device becomes available on your computer.
6. Connect to the board using a serial terminal. Entering an invalid command should result in "Haha" being returned. Entering `stat` should result in '0' being returned.
7. In the serial terminal, enter the `eras` command to erase the flash memory. This should change the result of the `stat` command to be '2', and then eventually '0' once the erase is complete.
8. Close your serial terminal.
9. Run the serial_upload script, providing it with your FAT16 image and the serial device. This begins the flash write process.
10. Wait until the upload completes.
11. Burn the player firmware to the board.
12. Push some buttons.

## Quirks, Errata, and Notes

- The upload utility does not erase the flash, it only writes. Writes will fail verification if the chip has not been erased. The erase steps must be taken before every  is changed.
- In some circumstances, board programming fails unless a finger is held against the bottom of the PDI port (and even then is unreliable). 
- Incomplete exploratory code for Opus and Midi Codecs is present in the codebase
