OBJ = \
	unagi.o paralellport.o script.o header.o \
	file.o textutil.o giveio.o unagi.res.o
OBJ_HK = giveio.o hongkongfc.o
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
file.o: file.c file.h type.h
giveio.o: giveio.c giveio.h
header.o: header.c type.h file.h header.h
paralellport.o: paralellport.c type.h kairo.h paralellport.h
script.o: script.c type.h file.h paralellport.h giveio.h textutil.h \
  header.h script.h
unagi.o: unagi.c type.h paralellport.h giveio.h script.h
textutil.o: textutil.c type.h textutil.h
hongkongfc.o: hongkongfc.c type.h hongkongfc_hard.h giveio.h
