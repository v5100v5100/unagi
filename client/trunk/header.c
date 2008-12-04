/*
famicom ROM cartridge utility - unagi
iNES header/buffer control
*/
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "type.h"
#include "file.h"
#include "crc32.h"
#include "config.h"
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
	if((r->cpu_ram.size != 0) || (r->backupram != 0)){
		header[6] |= 0x02;
	}
	//4 screen は無視
	header[6] |= (r->mappernum & 0x0f) << 4;
	header[7] |= r->mappernum & 0xf0;
}

/*
return値: error count
*/
static int mirroring_fix(struct memory *m, long min)
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
			return 0;
		}
		mirror_size /= 2;
	}
	
	u8 *ffdata;
	int ret = 0;
	ffdata = malloc(min);
	memset(ffdata, 0xff, min);
	if(memcmp(ffdata, m->data, min) == 0){
		printf("error: data is all 0xff\n");
		ret = 1;
	}else if(m->size != min){
		printf("mirroring %s rom fixed\n", m->name);
		m->size = min;
	}
	free(ffdata);
	
	return ret;
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
	int error = 0;
	//RAM adapter bios size 0x2000 は変更しない
	if(r->cpu_rom.size >= PROGRAM_ROM_MIN){
		error += mirroring_fix(&(r->cpu_rom), PROGRAM_ROM_MIN);
	}
	if(r->ppu_rom.size != 0){
		error += mirroring_fix(&(r->ppu_rom), CHARCTER_ROM_MIN);
	}
	/*if(error != 0){
		return;
	}*/
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

int nesbuffer_malloc(struct romimage *r, int mode)
{
	memory_malloc(&(r->cpu_rom));
	memory_malloc(&(r->ppu_rom));
	if(mode == MODE_RAM_READ){
		memory_malloc(&(r->cpu_ram));
	}
	return OK;
}

static inline void memory_free(struct memory *m)
{
	if(m->size != 0){
		free(m->data);
		m->data = NULL;
	}
}
void nesbuffer_free(struct romimage *r, int mode)
{
	memory_free(&(r->cpu_rom));
	memory_free(&(r->ppu_rom));
	if(mode == MODE_RAM_READ){
		memory_free(&(r->cpu_ram));
	}
}

void backupram_create(const struct memory *r, const char *ramfilename)
{
	buf_save(r->data, ramfilename, r->size);
}

static int nesfile_size_check(const char *errorprefix, const struct memory *m, long size)
{
	if(size < m->size){
		while(size == m->size){
			if(size < 0){
				printf("%s NES header %s romsize alignment error\n", errorprefix, m->name);
				return NG;
			}
			size -= m->size;
		}
		return NG;
	}
	//
	else if(size > m->size){
		printf("%s NES header cpuromsize too large\n", errorprefix);
		return NG;
	}
	return OK;
}

/*
memory size は 2乗されていく値が正常値.
ただし、region の最小値より小さい場合は test 用として正常にする
*/
int memorysize_check(const long size, int region)
{
	long min;
	switch(region){
	case MEMORY_AREA_CPU_ROM:
		min = PROGRAM_ROM_MIN;
		break;
	case MEMORY_AREA_CPU_RAM:
		min = 0x800; //いまのところ. taito 系はもっと小さいような気がする
		break;
	case MEMORY_AREA_PPU:
		min = CHARCTER_ROM_MIN;
		break;
	default:
		assert(0);
	}
	if(size <= min){
		return OK;
	}
	switch(size){
	case 0x004000:
	case 0x008000: //27256
	case 0x010000: //27512
	case 0x020000: //1M bit
	case 0x040000: //2M
	case 0x080000: //4M
	case 0x100000: //8M
		return OK;
	}
	return NG;
}

/*
romimage が bank の定義値より小さい場合は romarea の末尾に張る。 
同じデータを memcpy したほうが安全だが、とりあえずで。
*/
static void nesfile_datapointer_set(const u8 *buf, struct memory *m, long size)
{
	u8 *data;
	assert((size % CHARCTER_ROM_MIN) == 0);
	assert((m->size % CHARCTER_ROM_MIN) == 0);
	data = malloc(size);
	m->data = data;
	if(size < m->size){
		long fillsize = m->size - size;
		assert(fillsize >= 0); //fillsize is minus
		memset(data, 0xff, fillsize); //ROM の未使用領域は 0xff が基本
		data += fillsize;
		size -= fillsize;
	}
	memcpy(data, buf, size);
}

//flashmemory device capacity check が抜けてるけどどこでやるか未定
int nesfile_load(const char *errorprefix, const char *file, struct romimage *r)
{
	int imagesize;
	u8 *buf;
	
	buf = buf_load_full(file, &imagesize);
	if(buf == NULL || imagesize < (NES_HEADER_SIZE + PROGRAM_ROM_MIN)){
		printf("%s ROM image open error\n", errorprefix);
		return NG;
	}
	//nes header check
	if(memcmp(buf, NES_HEADER_INIT, 4) != 0){
		printf("%s NES header identify error\n", errorprefix);
		free(buf);
		return NG;
	}
	//mapper number check
	{
		long mapper = (buf[6] >> 4) & 0x0f;
		mapper |= buf[7] & 0xf0;
		if(r->mappernum != mapper){
			printf("%s NES header mapper error\n", errorprefix);
			free(buf);
			return NG;
		}
	}
	//NES/CPU/PPU imagesize check
	long cpusize, ppusize;
	{
		long offset = NES_HEADER_SIZE;
		//CPU
		cpusize = ((long) buf[4]) * PROGRAM_ROM_MIN;
		offset += cpusize;
		if(nesfile_size_check(errorprefix, &r->cpu_rom, cpusize) == NG){
			free(buf);
			return NG;
		}
		//PPU
		ppusize = ((long) buf[5]) * CHARCTER_ROM_MIN;
		offset += ppusize;
		if(ppusize != 0){
			if(nesfile_size_check(errorprefix, &r->ppu_rom, ppusize) == NG){
				free(buf);
				return NG;
			}
		}
		//NESfilesize
		if(offset != imagesize){
			printf("%s NES header filesize error\n", errorprefix);
			free(buf);
			return NG;
		}
	}
	/*
	image pointer set/ memcpy
	*/
	{
		u8 *d;
		d = buf;
		d += NES_HEADER_SIZE;
		nesfile_datapointer_set(d, &r->cpu_rom, cpusize);
		d += cpusize;
		if(ppusize != 0){
			nesfile_datapointer_set(d, &r->ppu_rom, ppusize);
		}
	}

	free(buf);
	return OK;
}
