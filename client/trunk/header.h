#ifndef _HEADER_H_
#define _HEADER_H_
#include "flashmemory.h"
struct memory{
	const char *name;
	int size, offset;
	u8 *data;
};
/*
ROM image �� struct memory �Υ⡼���̤λȤ���
MODE_ROM_DUMP
	cpu_rom ROM �ɤ߹��ߥХåե�, file out
	ppu_rom ROM �ɤ߹��ߥХåե�, file out
	cpu_ram ̤����
MODE_RAM_READ
	cpu_rom ̤����
	ppu_rom ̤����
	cpu_ram RAM �ɤ߹��ߥХåե�. file out
MODE_RAM_WRITE
	cpu_rom ̤����
	ppu_rom ̤����
	cpu_ram RAM �񤭹��ߥХåե�. . file in
MODE_ROM_PROGRAM
	cpu_rom ROM �񤭹��ߥХåե�, file in
	ppu_rom ROM �񤭹��ߥХåե�, file in
	cpu_ram ̤����
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
