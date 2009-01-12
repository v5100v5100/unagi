#ifndef _HEADER_H_
#define _HEADER_H_
#include "flashmemory.h"
struct memory{
	const char *name;
	int size, offset, attribute;
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
enum{
	MEMORY_AREA_CPU_RAM, MEMORY_AREA_CPU_ROM, MEMORY_AREA_PPU,
	MEMORY_ATTR_READ, MEMORY_ATTR_WRITE, MEMORY_ATTR_NOTUSE
};

#ifdef HEADEROUT
void nesheader_set(const struct romimage *r, u8 *header);
#endif
int nesbuffer_malloc(struct romimage *r, int mode);
void nesfile_create(struct romimage *r, const char *romfilename);
void nesbuffer_free(struct romimage *r, int mode);
void backupram_create(const struct memory *r, const char *ramfilename);
int memorysize_check(const long size, int region);
int nesfile_load(const char *errorprefix, const char *file, struct romimage *r);
#endif