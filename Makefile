CC=gcc
CFLAGS=-g -DDEBUG_INFO_ON
LDFLAGS=-lpthread -lrt
SOURCE= src/calendar_main.c
OBJECT= src/calendar_main.o
TARGET = calendar

all:$(TARGET)

$(OBJECT):$(SOURCE)
	$(CC) $(CFLAGS) -o $@ -c $< 

$(TARGET):$(OBJECT)
	$(CC) $(LDFLAGS) -o $@ $< 

clean:
	rm src/*.o $(TARGET)






