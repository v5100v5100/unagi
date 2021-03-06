#ifndef _HEADER_H_
#define _HEADER_H_
struct memory{
	const char *name;
	int size, offset;
	u8 *data;
};
struct romimage{
	u8 *neshead;
	struct memory cpu_rom, cpu_ram, ppu_rom;
	int mappernum;
	int mirror;
};

enum{
	MIRROR_HORIZONAL = 0,
	MIRROR_VERTICAL,
	MIRROR_PROGRAMABLE = MIRROR_HORIZONAL
};
int nesbuffer_malloc(struct romimage *r);
void nesfile_create(struct romimage *r, const char *romfilename);
void nesbuffer_free(struct romimage *r);
void backupram_create(const struct memory *r, const char *ramfilename);
#endif
