OBJ = \
	unagi.o header.o crc32.o \
	script_engine.o script_syntax.o \
	reader_master.o reader_onajimi.o reader_hongkongfc.o \
	flashmemory.o \
	file.o textutil.o giveio.o unagi.res.o
TARGET = unagi.exe
CC = gcc
CFLAGS = -Wall -Werror -Wmissing-declarations -I..#-Wcast-qual
