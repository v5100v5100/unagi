all: anago.exe
LIBUSB = d:/dev/LibUSB-Win32
SQUIRREL = ../SQUIRREL2
VPATH = ..
CFLAGS = -g -O0 -Wall -I.. -I$(LIBUSB)/include -I$(SQUIRREL)/include -DDEBUG=1 -DANAGO=1
LDFLAG = -L. -L$(LIBUSB)/lib/gcc -L$(SQUIRREL)/lib
CC = gcc
OBJ = anago.o flashmemory.o header.o crc32.o file.o script.o \
	reader_dummy.o reader_kazzo.o usb_device.o squirrel_wrap.o memory_manage.o
clean:
	rm -f $(OBJ)
anago.exe: $(OBJ) 
	g++ -o $@ $(LDFLAG) $(OBJ) -lusb -lsqstdlib -lsquirrel
