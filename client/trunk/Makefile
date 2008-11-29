OBJ = \
	unagi.o script.o header.o crc32.o \
	driver_master.o driver_onajimi.o driver_hongkongfc.o \
	flashmemory.o \
	file.o textutil.o giveio.o unagi.res.o
OBJ_HK = giveio.o driver_hongkongfc.o
TARGET = unagi.exe
ifeq ($(RELEASE),1)
	CFLAGS = -O2 -Wall -DDEBUG=0 -DNDEBUG
else
	CFLAGS = -O0 -Wall -g -DDEBUG=1
endif
ifeq ($(PROFILE),1)
	CFLAGS = -O2 -Wall -pg -DDEBUG=0
	LDFLAG = -pg
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
