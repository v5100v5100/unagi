all: $(APP_GUI) $(APP_CUI)
VPATH = ..
ifeq ($(RELEASE),1)
  CPPFLAGS += -O2 -DNDEBUG -DDEBUG=0
else
  CPPFLAGS += -g -O0 -DDEBUG=1
endif
CPPFLAGS += -Wall -Werror
CFLAGS = -I.. -I$(SQUIRREL)/include -I$(KAZZO)
ifneq ($(strip $(LIBUSB)),)
  CFLAGS += -I$(LIBUSB)/include
endif
CXXFLAGS += -I.. `$(WX_CONFIG) --cppflags`

LDFLAG = -L$(SQUIRREL)/lib
ifneq ($(strip $(LIBUSB)),)
  LDFLAG += -L$(LIBUSB)/lib/gcc 
endif
CC = gcc

OBJ_CUI = anago_cui.o cui_gauge.o $(ICON_CUI)
OBJ_GUI = anago_gui.o anago_frame.o nescartxml.o $(ICON_GUI)
OBJ_CORE = romimage.o crc32.o file.o widget.o \
	reader_kazzo.o reader_dummy.o usb_device.o squirrel_wrap.o \
	script_program.o script_dump.o script_common.o flash_device.o

ifeq ($(strip $(RELEASE)),)
	OBJ_CORE += memory_manage.o
endif
ifneq ($(strip $(APP_CUI)),)
	OBJ_CUI += $(OBJ_CORE)
	OBJ_GUI += $(OBJ_CORE)
else
	OBJ_GUI += $(OBJ_CUI)
	OBJ_GUI += $(OBJ_CORE)
endif

$(APP_GUI): $(OBJ_GUI) 
	g++ -o $@ $(LDFLAG) $(OBJ_GUI) `$(WX_CONFIG) --libs core,adv,xml` -lusb -lsqstdlib -lsquirrel
$(APP_CUI): $(OBJ_CUI) 
	g++ -o $@ $(LDFLAG) $(OBJ_CUI) -lusb -lsqstdlib -lsquirrel
clean:
	rm -f $(OBJ_CUI) $(OBJ_GUI) $(OBJ_CORE) anago.d

anago.d:
	gcc -MM $(CFLAGS) $(CPPFLAGS) *.c > $@
	g++ -MM $(CFLAGS) $(CXXFLAGS) *.cpp >> $@
.SUFFIXES: .rc .res.o
.rc.res.o:
	`$(WX_CONFIG) --rescomp` -i $< -o $@
-include anago.d
