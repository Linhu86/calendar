#
#  calendar Makefile
#  Created by Linhu
#  30/01/2016
#

TARGET=calendar

CC=gcc
CFLAGS=
LDFLAGS=
SOURCES=
OBJECT=
INC=

#flag to control gdb debugging feature
GDB_DEBUG_ON=-g

#flag to control debugging info print or not.
DEBUG_ON=DEBUG_INFO_ON

#flag to control the os platform.
OS_INFO=OS_LINUX

#flag to control the os platform wrapper.
OS_WRAPPER=linux

#Add source file.
SOURCES += src/calendar_main.c \
		  src/api/wrapper/rtos_wrapper_$(OS_WRAPPER).c

#Include header file
INC += -Iinc \
	   -Isrc/api/wrapper

$(INFO $(OBJECT))

#Add CFLAGS
CFLAGS += $(GDB_DEBUG_ON) \
		  -D$(DEBUG_ON)   \
		  -D$(OS_INFO)

#Add LDFLAGS
LDFLAGS += -lpthread -lrt

#build
all:$(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(INC) $(CFLAGS) $(LDFLAGS) $^ -o $@

#clean workspace
clean:
	rm -rf src/*.o $(TARGET)






