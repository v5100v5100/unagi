OBJ = \
	unagi.o script.o header.o crc32.o \
	reader_master.o reader_onajimi.o reader_hongkongfc.o \
	flashmemory.o \
	file.o textutil.o giveio.o unagi.res.o
OBJ_HK = giveio.o reader_hongkongfc.o
TARGET = unagi.exe
CFLAGS = -Wall -Werror -Wmissing-declarations #-Wcast-qual
#else if が使えればもうちょい見やすくなるのに...
ifeq ($(RELEASE),1)
	CFLAGS += -O2 -DDEBUG=0 -DNDEBUG -fomit-frame-pointer 
else
	ifeq ($(PROFILE),1)
		CFLAGS += -O2 -pg -DDEBUG=0
		LDFLAG = -pg
	else
		OBJ += client_test.o
		CFLAGS += -O0 -g -DDEBUG=1
	endif
endif

all: $(TARGET) unagi.d
unagi.d:
	gcc -MM *.c > $@
hk.exe: $(OBJ_HK)
	gcc -o $@ $(OBJ_HK)
iodel.exe: iodel.o giveio.o
	gcc -o $@ iodel.o giveio.o
clean: 
	rm -f $(OBJ) $(TARGET) unagi.d
$(TARGET): $(OBJ)
	gcc $(LDFLAG) -o $@ $(OBJ)
ifeq ($(RELEASE),1)
	strip $@
endif
unagi.res.o: unagi.rc unagi.ico
	windres -i $< -o $@

#---- depend file ----
-include unagi.d
