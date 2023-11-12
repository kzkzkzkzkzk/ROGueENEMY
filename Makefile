CFLAGS= -g -O0 -D _DEFAULT_SOURCE -D_POSIX_C_SOURCE=200112L -std=c11 -fPIE -pedantic -Wall # -Werror
LDFLAGS=-lpthread -levdev -lrt -lm
CC=gcc
OBJECTS=main.o input_dev.o dev_iio.o output_dev.o queue.o platform.o
TARGET=rogue_enemy

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

include depends

depends:
	$(CC) -MM $(OBJECTS:.o=.c) > depends

clean:
	rm -f ./$(TARGET) *.o depends