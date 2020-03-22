#
#             LUFA Library
#     Copyright (C) Dean Camera, 2014.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

MCU          = atxmega16a4u
ARCH         = XMEGA
BOARD        = NONE
F_CPU        = 32000000
F_USB        = 48000000
OPTIMIZATION = s
TARGET       = VirtualSerial
SRC          = $(TARGET).c ../SpiFlash.c Descriptors.c CommandHandler.c ../FAT.c ../AudioFile.c ../RawCodec.c ../MsAdpcmCodec.c ../ToneGen.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = ../../../other/LUFA-140928/LUFA/
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/ -std=gnu99 -Wall -Wno-unused-variable -Werror -mcall-prologues
LD_FLAGS     =
AVRDUDE_PROGRAMMER = dragon_pdi
# Default target
all:

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk
