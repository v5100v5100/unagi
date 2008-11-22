OBJ = \
	unagi.o script.o header.o \
	driver_master.o driver_onajimi.o driver_hongkongfc.o \
	file.o textutil.o giveio.o unagi.res.o
OBJ_HK = giveio.o driver_hongkongfc.o
TARGET = unagi.exe
CFLAGS = -O0 -Wall -g -DDEBUG=1
#CFLAGS = -O2 -Wall -DDEBUG=0

all: $(TARGET)
hk.exe: $(OBJ_HK)
	gcc -o $@ $(OBJ_HK)
clean: 
	rm -f $(OBJ) $(TARGET)
$(TARGET): $(OBJ)
	gcc -o $@ $(OBJ)
unagi.res.o: unagi.rc unagi.ico
	windres -i $< -o $@

#---- depend file ----
driver_hongkongfc.o: driver_hongkongfc.c type.h paralellport.h \
  driver_master.h driver_hongkongfc.h hard_hongkongfc.h
driver_master.o: driver_master.c driver_master.h type.h driver_onajimi.h \
  driver_hongkongfc.h
driver_onajimi.o: driver_onajimi.c type.h paralellport.h hard_onajimi.h \
  driver_master.h driver_onajimi.h
file.o: file.c file.h type.h
giveio.o: giveio.c giveio.h
header.o: header.c type.h file.h header.h
script.o: script.c type.h file.h driver_master.h giveio.h textutil.h \
  header.h script.h
textutil.o: textutil.c type.h textutil.h
unagi.o: unagi.c type.h driver_master.h giveio.h file.h script.h
