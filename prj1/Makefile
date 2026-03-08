CC = gcc
OBJS = bitmap.o debug.o hash.o hex_dump.o list.o main.o
TARGET = testlib


$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)


bitmap.o: bitmap.c
debug.o: debug.c 
hash.o: hash.c 
hex_dump.o: hex_dump.c
list.o: list.c
main.o: main.c

.PHONY: clean
clean:
		rm -f *.o
		rm -f $(TARGET)
