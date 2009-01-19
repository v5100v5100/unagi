OBJ = \
	unagi.o script.o syntax.o header.o crc32.o \
	reader_master.o reader_onajimi.o reader_hongkongfc.o \
	flashmemory.o \
	file.o textutil.o giveio.o unagi.res.o
TARGET = unagi.exe
CC = gcc
CFLAGS = -Wall -Werror -Wmissing-declarations -I..#-Wcast-qual
