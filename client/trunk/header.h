#ifndef _HEADER_H_
#define _HEADER_H_
#include "flashmemory.h"
struct memory{
	const char *name;
	int size, offset;
	u8 *data;
};
/*
ROM image 内 struct memory のモード別の使い方
MODE_ROM_DUMP
	cpu_rom ROM 読み込みバッファ, file out
	ppu_rom ROM 読み込みバッファ, file out
	cpu_ram 未使用
MODE_RAM_READ
	cpu_rom 未使用
	ppu_rom 未使用
	cpu_ram RAM 読み込みバッファ. file out
MODE_RAM_WRITE
	cpu_rom 未使用
	ppu_rom 未使用
	cpu_ram RAM 書き込みバッファ. . file in
MODE_ROM_PROGRAM
	cpu_rom ROM 書き込みバッファ, file in
	ppu_rom ROM 書き込みバッファ, file in
	cpu_ram 未使用
*/
struct romimage{
	struct memory cpu_rom, ppu_rom, cpu_ram;
	struct flash_order cpu_flash, ppu_flash;
	long mappernum;
	int mirror, backupram;
};

enum{
	MIRROR_HORIZONAL = 0,
	MIRROR_VERTICAL,
	MIRROR_PROGRAMABLE = MIRROR_HORIZONAL
};

int nesbuffer_malloc(struct romimage *r, int mode);
void nesfile_create(struct romimage *r, const char *romfilename);
void nesbuffer_free(struct romimage *r, int mode);
void backupram_create(const struct memory *r, const char *ramfilename);
#endif
