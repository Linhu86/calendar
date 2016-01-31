#
#  calendar Makefile
#  Created by Linhu
#  30/01/2016
#

TARGET=calendar
TEST_TARGET=unittest/testapp

CC=gcc
CFLAGS=
LDFLAGS=
SOURCES=
TEST_SOURCES=
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
		       src/api/src/user_input_parse.c \
		       src/api/src/calendar_manager.c \
		       src/api/src/calendar_database.c \
		       src/api/wrapper/rtos_wrapper_$(OS_WRAPPER).c \

TEST_SOURCES += unittest/src/test.c \
		                src/api/src/user_input_parse.c \
		                src/api/src/calendar_manager.c \
		                src/api/src/calendar_database.c \
				   src/api/wrapper/rtos_wrapper_$(OS_WRAPPER).c \

#Include header file
INC += -Iinc \
	     -Isrc/api/wrapper \
	     -Isrc/api/inc

$(INFO $(OBJECT))

#Add CFLAGS
CFLAGS += $(GDB_DEBUG_ON) \
		    -D$(DEBUG_ON)   \
		    -D$(OS_INFO)

#Add LDFLAGS
LDFLAGS += -lpthread -lrt

#build
all:$(TARGET) $(TEST_TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(INC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(TEST_TARGET): $(TEST_SOURCES)
	$(CC) $(INC) $(CFLAGS) $(LDFLAGS) -lcunit $^ -o $@

#clean workspace
clean:
	rm -rf src/*.o $(TARGET)  $(TEST_TARGET)






