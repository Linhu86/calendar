CC=gcc
CFLAGS=-g
LDFLAGS=
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






