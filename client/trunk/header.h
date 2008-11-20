#ifndef _HEADER_H_
#define _HEADER_H_
struct memory{
	const char *name;
	int size, offset;
	u8 *data;
};
struct romimage{
	u8 *neshead;
	struct memory cpu_rom, ppu_rom, cpu_ram_read, cpu_ram_write;
	int mode;
	const char *ramfile;
	int mappernum;
	int mirror;
};

enum{
	MIRROR_HORIZONAL = 0,
	MIRROR_VERTICAL,
	MIRROR_PROGRAMABLE = MIRROR_HORIZONAL
};
enum{
	MODE_ROM_DUMP,
	MODE_RAM_DUMP,
	MODE_RAM_WRITE
};

int nesbuffer_malloc(struct romimage *r);
void nesfile_create(struct romimage *r, const char *romfilename);
void nesbuffer_free(struct romimage *r);
void backupram_create(const struct memory *r, const char *ramfilename);
#endif
