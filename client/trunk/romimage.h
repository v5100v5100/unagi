#ifndef _ROMIAMGE_H_
#define _ROMIAMGE_H_
enum mode{
	MODE_TEST,
	MODE_ROM_DUMP,
	MODE_RAM_READ,
	MODE_RAM_WRITE,
	MODE_ROM_PROGRAM
};
enum trastype{
	TRANSTYPE_EMPTY,
	TRANSTYPE_TOP,
	TRANSTYPE_BOTTOM,
	TRANSTYPE_FULL,
};
enum memory_attribute{
	MEMORY_ATTR_READ, MEMORY_ATTR_WRITE, MEMORY_ATTR_NOTUSE
};
struct memory{
	const wgChar *name;
	int size, offset;
	enum memory_attribute attribute;
	enum trastype transtype;
	uint8_t *data;
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
enum vram_mirroring{
	MIRROR_HORIZONAL = 0,
	MIRROR_VERTICAL,
	MIRROR_PROGRAMABLE = MIRROR_HORIZONAL
};
struct romimage{
	struct memory cpu_rom, ppu_rom, cpu_ram;
	long mappernum;
	enum vram_mirroring mirror;
	int backupram;
};

enum{
	MEMORY_AREA_CPU_RAM, MEMORY_AREA_CPU_ROM, MEMORY_AREA_PPU
};

bool nesbuffer_malloc(struct romimage *r, int mode);
void nesfile_create(const struct textcontrol *l, struct romimage *r, const wgChar *romfilename);
void nesbuffer_free(struct romimage *r, int mode);
void backupram_create(const struct memory *r, const wgChar *ramfilename);
int memorysize_check(const long size, int region);
bool nesfile_load(const struct textcontrol *l, const wgChar *file, struct romimage *r);
#endif
