#
#  calendar Makefile
#  Created by Linhu
#  30/01/2016
#

TARGET=calendar

CC=gcc
CFLAGS=
LDFLAGS=
SOURCE=
OBJECT=
INC=

#flag to control gdb debugging feature
GDB_DEBUG_ON=-g

#flag to control debugging info print or not.
DEBUG_ON=DEBUG_INFO_ON

#flag to control the os platform.
OS_INFO=OS_LINUX

#Add source file.
SOURCE += src/calendar_main.c

#Include header file
INC += -Iinc

#Convert obj file name from source file name.
OBJECT += $(SOURCE:.cpp=.o)

$(INFO $(OBJECT))

#Add CFLAGS
CFLAGS += $(GDB_DEBUG_ON) \
		  -D$(DEBUG_ON)   \
		  -D$(OS_INFO)    \

CFLAGS += -Iinc

#Add LDFLAGS
LDFLAGS += -lpthread -lrt

#build
all:$(TARGET)

#Compile source file and create object file.
$(OBJECT):$(SOURCE)
	$(CC) $(CFLAGS) -o $@ -c $< 

#Link obj file to binary file.
$(TARGET):$(OBJECT)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

#clean workspace
clean:
	rm -rf src/*.o $(TARGET)






