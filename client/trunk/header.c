/*
famicom ROM cartridge utility - unagi
iNES header/buffer control

todo:
* buffer の malloc 方法のやり直し?
* (このソース外の仕事だけど)mirror, battery, mapper number をコマンドラインからも指定できるようにする
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "type.h"
#include "file.h"
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

void nesfile_create(struct romimage *r, const char *romfilename)
{
	//RAM adapter bios size 0x2000 は変更しない
	if(r->cpu_rom.size >= PROGRAM_ROM_MIN){
		mirroring_fix(&(r->cpu_rom), PROGRAM_ROM_MIN);
	}
	if(r->ppu_rom.size != 0){
		mirroring_fix(&(r->ppu_rom), CHARCTER_ROM_MIN);
	}

	FILE *f;
	nesheader_set(r, r->neshead);
	f = fopen(romfilename, "wb");
	fseek(f, 0, SEEK_SET);
	//RAM adapter bios には NES ヘッダを作らない
	if(r->cpu_rom.size >= PROGRAM_ROM_MIN){ 
		fwrite(r->neshead, sizeof(u8), NES_HEADER_SIZE, f);
	}
	fwrite(r->cpu_rom.data, sizeof(u8), r->cpu_rom.size, f);
	if(r->ppu_rom.size != 0){
		fwrite(r->ppu_rom.data, sizeof(u8), r->ppu_rom.size, f);
	}
	fclose(f);
}

/*
NES fileimage がそのまま使えるような領域を確保し、各用途別ポインタを張る
+0     NESHEADER
+0x10  Program ROM
+....  Charcter ROM (ない場合は NULL)
+....  Backup RAM (ない場合は NULL)

NES file 出力時は neshead から ROM サイズ分出力するだけ。
解放時は nesheader を free するだけ
*/
int nesbuffer_malloc(struct romimage *r)
{
	u8 *p;
	const int nessize = NES_HEADER_SIZE + r->cpu_rom.size + r->ppu_rom.size + r->cpu_ram_read.size;
	r->neshead = malloc(nessize);
	if(r->neshead == NULL){
		printf("%s: malloc failed\n", __FUNCTION__);
		return NG;
	}
	p = r->neshead + NES_HEADER_SIZE;
	if(r->cpu_rom.size != 0){
		r->cpu_rom.data = p;
	}
	p += r->cpu_rom.size;
	if(r->ppu_rom.size != 0){
		r->ppu_rom.data = p;
	}
	p += r->ppu_rom.size;
	if(r->cpu_ram_read.size != 0){
		r->cpu_ram_read.data = p;
	}
	return OK;
}

void nesbuffer_free(struct romimage *t)
{
	free(t->neshead);
}

void backupram_create(const struct memory *r, const char *ramfilename)
{
	buf_save(r->data, ramfilename, r->size);
}
