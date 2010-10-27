all: $(TARGET)
VPATH = ..
ifeq ($(RELEASE),1)
  CFLAGS = -O2 -DNDEBUG
else
  CFLAGS = -g -O0
endif
CFLAGS += -Wall -Werror -DDEBUG=1 -DANAGO=1
CFLAGS += -I.. -I$(SQUIRREL)/include -I$(KAZZO)
ifneq ($(strip $(LIBUSB)),)
  CFLAGS += -I$(LIBUSB)/include
endif
LDFLAG = -L$(SQUIRREL)/lib
ifneq ($(strip $(LIBUSB)),)
  LDFLAG += -L$(LIBUSB)/lib/gcc 
endif
CC = gcc
OBJ = anago_cui.o header.o crc32.o file.o widget.o cui_gauge.o \
	script_program.o script_dump.o script_common.o \
	progress.o flash_device.o \
	reader_kazzo.o usb_device.o squirrel_wrap.o memory_manage.o

clean:
	rm -f $(OBJ)
$(TARGET): $(OBJ) 
	g++ -o $@ $(LDFLAG) $(OBJ) -lusb -lsqstdlib -lsquirrel

script_flash.o: squirrel_wrap.h
script_dump.o: squirrel_wrap.h
