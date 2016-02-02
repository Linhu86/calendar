#
#  calendar Makefile for linux platform
#  Created by Linhu
#  31/01/2016
#

BUILD_OPT=HOST_LINUX

#uncomment the following 2 lines to cross-compile calendar for ARM LINARO platform.
#BUILD_OPT=ARM_LINARO
#ARM_LINARO_PATH=../gcc-linaro-arm-linux-gnueabihf-4.8-2014.03_linux

TARGET=calendar

#unit test to be executed only in host environment unless you have cross-compiled libcunit1 in target environment.
ifeq ($(BUILD_OPT), HOST_LINUX)
	TEST_TARGET=unittest/testapp
endif

#compiler settings
ifeq ($(BUILD_OPT), HOST_LINUX)
	CC=gcc
else ifeq ($(BUILD_OPT), ARM_LINARO)
	CC=$(ARM_LINARO_PATH)/bin/arm-linux-gnueabihf-gcc
else
	CC=gcc
endif

CFLAGS=
INC=
LDFLAGS=
SOURCES=
TEST_SOURCES=
OBJECT=

#Cross compile include and library path settings.
ifeq ($(BUILD_OPT), ARM_LINARO)
INC += -I$(ARM_LINARO_PATH)/arm-linux-gnueabihf/libc/usr/include/ \
       -I$(ARM_LINARO_PATH)/arm-linux-gnueabihf/include/

LDFLAGS += -L$(ARM_LINARO_PATH)/arm-linux-gnueabihf/libc/usr/lib \
           -L$(ARM_LINARO_PATH)/arm-linux-gnueabihf/libc/lib \
           -L$(ARM_LINARO_PATH)/arm-linux-gnueabihf/lib
endif

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
           src/api/src/helper_func.c       \
           src/api/wrapper/rtos_wrapper_$(OS_WRAPPER).c \

TEST_SOURCES += unittest/src/test.c \
                src/api/src/user_input_parse.c \
                src/api/src/calendar_manager.c \
                src/api/src/calendar_database.c \
		        src/api/src/helper_func.c       \
                src/api/wrapper/rtos_wrapper_$(OS_WRAPPER).c \

#Include header file
INC += -Iinc \
	   -Isrc/api/wrapper \
	   -Isrc/api/inc

$(INFO $(OBJECT))

#Add CFLAGS
CFLAGS += $(GDB_DEBUG_ON)
CFLAGS +=-D$(DEBUG_ON)
CFLAGS +=-D$(OS_INFO)

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






