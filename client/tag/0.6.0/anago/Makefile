all: anago.exe
LIBUSB = d:/dev/LibUSB-Win32
SQUIRREL = ../SQUIRREL2
KAZZO = ../../kazzo/firmware
VPATH = ..
#CFLAGS = -g -O0
CFLAGS = -O2
CFLAGS += -Wall -Werror -I.. -I$(LIBUSB)/include -I$(SQUIRREL)/include -I$(KAZZO) -DDEBUG=1 -DANAGO=1
LDFLAG = -L. -L$(LIBUSB)/lib/gcc -L$(SQUIRREL)/lib
CC = gcc
OBJ = anago.o header.o crc32.o file.o \
	script_flash.o script_dump.o script_common.o \
	progress.o flash_device.o \
	reader_dummy.o reader_kazzo.o usb_device.o squirrel_wrap.o memory_manage.o
clean:
	rm -f $(OBJ)
anago.exe: $(OBJ) 
	g++ -o $@ $(LDFLAG) $(OBJ) -lusb -lsqstdlib -lsquirrel

script_flash.o: squirrel_wrap.h
script_dump.o: squirrel_wrap.h
