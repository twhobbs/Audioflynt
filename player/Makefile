TARGET=player

SRCS = main.c ../AudioFile.c ../ToneGen.c ../RawCodec.c ../MsAdpcmCodec.c
CFLAGS += -g -Wall

default: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -lao -lm -o $(TARGET)
	
clean :
	rm -f $(TARGET) $(OBJS)
