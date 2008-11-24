/*
famicom ROM cartridge utility - unagi
iNES header/buffer control
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "type.h"
#include "file.h"
#include "crc32.h"
#include "header.h"

enum{
	NES_HEADER_SIZE = 0x10,
	PROGRAM_ROM_MIN = 0x4000,
	CHARCTER_ROM_MIN = 0x2000
};
static const u8 NES_HEADER_INIT[NES_HEADER_SIZE] = {
	'N', 'E', 'S', 0x1a, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static void nesheader_set(const struct romimage *r, u8 *header)
{
	memcpy(header, NES_HEADER_INIT, NES_HEADER_SIZE);
	header[4] = r->cpu_rom.size / PROGRAM_ROM_MIN;
	header[5] = r->ppu_rom.size / CHARCTER_ROM_MIN;
	if(r->mirror == MIRROR_VERTICAL){
		header[6] |= 0x01;
	}
	if((r->cpu_ram_read.size != 0) || (r->backupram != 0)){
		header[6] |= 0x02;
	}
	//4 screen は無視
	header[6] |= (r->mappernum & 0x0f) << 4;
	header[7] |= r->mappernum & 0xf0;
}

static void mirroring_fix(struct memory *m, long min)
{
	long mirror_size = m->size / 2;
	while(mirror_size >= min){
		const u8 *halfbuf;
		halfbuf = m->data;
		halfbuf += mirror_size;
		if(memcmp(m->data, halfbuf, mirror_size) != 0){
			const long ret = mirror_size * 2;
			if(m->size != ret){
				printf("mirroring %s rom fixed\n", m->name);
				m->size = ret;
			}
			return;
		}
		mirror_size /= 2;
	}
	if(m->size != min){
		printf("mirroring %s rom fixed\n", m->name);
		m->size = min;
	}
}

//hash は sha1 にしたいが他のデータベースにあわせて crc32 にしとく
static void rominfo_print(const struct memory *m)
{
	if(m->size != 0){
		const uint32_t crc = crc32_get(m->data, m->size);
		printf("%s ROM size 0x%06x, crc32 0x%08x\n", m->name, m->size, (const int) crc);
	}else{
		printf("%s RAM\n", m->name);
	}
}

void nesfile_create(struct romimage *r, const char *romfilename)
{
	//RAM adapter bios size 0x2000 は変更しない
	if(r->cpu_rom.size >= PROGRAM_ROM_MIN){
		mirroring_fix(&(r->cpu_rom), PROGRAM_ROM_MIN);
	}
	if(r->ppu_rom.size != 0){
		mirroring_fix(&(r->ppu_rom), CHARCTER_ROM_MIN);
	}
	//修正済み ROM 情報表示
	printf("mapper %d\n", (int) r->mappernum);
	rominfo_print(&(r->cpu_rom));
	rominfo_print(&(r->ppu_rom));

	FILE *f;
	u8 header[NES_HEADER_SIZE];
	nesheader_set(r, header);
	f = fopen(romfilename, "wb");
	fseek(f, 0, SEEK_SET);
	//RAM adapter bios には NES ヘッダを作らない
	if(r->cpu_rom.size >= PROGRAM_ROM_MIN){ 
		fwrite(header, sizeof(u8), NES_HEADER_SIZE, f);
	}
	fwrite(r->cpu_rom.data, sizeof(u8), r->cpu_rom.size, f);
	if(r->ppu_rom.size != 0){
		fwrite(r->ppu_rom.data, sizeof(u8), r->ppu_rom.size, f);
	}
	fclose(f);
}

static inline void memory_malloc(struct memory *m)
{
	m->data = NULL;
	if(m->size != 0){
		m->data = malloc(m->size);
	}
}

int nesbuffer_malloc(struct romimage *r)
{
	memory_malloc(&(r->cpu_rom));
	memory_malloc(&(r->ppu_rom));
	memory_malloc(&(r->cpu_ram_read));
	return OK;
}

static inline void memory_free(struct memory *m)
{
	if(m->size != 0){
		free(m->data);
		m->data = NULL;
	}
}
void nesbuffer_free(struct romimage *r)
{
	memory_free(&(r->cpu_rom));
	memory_free(&(r->ppu_rom));
	memory_free(&(r->cpu_ram_read));
}

void backupram_create(const struct memory *r, const char *ramfilename)
{
	buf_save(r->data, ramfilename, r->size);
}
