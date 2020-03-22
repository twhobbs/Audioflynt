#ifndef COMMAND_HANDLER_INCLUDED
#define COMMAND_HANDLER_INCLUDED

#include "VirtualSerial.h"
#include "CommandHandler.h"
#include "../SpiFlash.h"
#include "../FAT.h"

#define FULL_COMMANDS

void handleCommand(FILE* stream, uint8_t* buff, int buffLength, int* readMode, uint8_t* data, FAT_Header* fatHeader);

#endif