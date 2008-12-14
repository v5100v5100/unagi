/*
famicom ROM cartridge utility - unagi
flash memory driver

Copyright (C) 2008  sato_tiff

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

flashmemory.c だけの警告
このソースコードを参考、転用してシェアウェアなどで利益を得ないこと。
判明した場合は LGPL が適用され、該当箇所のソースを公開する必要がある。
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "type.h"
#include "header.h"
#include "flashmemory.h"
/*
driver for Winbond W29C020, W49F002
*/
/*
JEDEC flash memory command
http://www.sst.com/downloads/software_driver/SST49LF002A.txt
*/
struct flash_task{
	long address, data;
};
enum{
	ADDRESS_0000 = 0,
	ADDRESS_2AAA = 0x2aaa,
	ADDRESS_5555 = 0x5555,
	FLASH_COMMAND_END
};
static const struct flash_task PRODUCTID_ENTRY[] = {
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x90},
	{FLASH_COMMAND_END, 0}
};
static const struct flash_task PRODUCTID_EXIT[] = {
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0xf0},
	{FLASH_COMMAND_END, 0}
};
static const struct flash_task PROTECT_DISABLE[] = {
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0xa0},
	{FLASH_COMMAND_END, 0}
};
static const struct flash_task PROTECT_ENABLE[] = {
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x80},
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x20},
	{FLASH_COMMAND_END, 0}
};
static const struct flash_task ERASE[] = {
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x80},
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x10},
	{FLASH_COMMAND_END, 0}
};

static const struct flash_task PP[] = {
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x80},
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x60},
	{FLASH_COMMAND_END, 0}
};

static void command_set(const struct flash_order *d, const struct flash_task *t)
{
	while(t->address != FLASH_COMMAND_END){
		long logical_address = 0;
		switch(t->address){
		case ADDRESS_0000: //bank によっては設定できないかも?
			logical_address = d->command_0000;
			break;
		case ADDRESS_2AAA:
			logical_address = d->command_2aaa;
			break;
		case ADDRESS_5555:
			logical_address = d->command_5555;
			break;
		default:
			assert(0); //unknown task address
		}
		d->flash_write(logical_address, t->data);
		t++;
	}
}

/*
---- product ID check ----
*/
static int productid_check(const struct flash_order *d, const struct flash_driver *f)
{
	u8 data[3];
	command_set(d, PRODUCTID_ENTRY);
	d->read(d->command_0000, 3, data);
	command_set(d, PRODUCTID_EXIT);
	if(f->id_manufacurer != data[0]){
		return NG;
	}
	if(f->id_device != data[1]){
		return NG;
	}
	return OK;
}

/*
---- toggle check ----
databit6
*/
const int CHECK_RETRY_MAX = 0x10000;
static int toggle_check(const struct flash_order *d, long address)
{
	u8 predata;
	int retry = 0;
	d->read(address, 1, &predata); //read DQ6
	predata &= 0x40;
	while(retry < CHECK_RETRY_MAX){
		u8 data;
		d->read(address, 1, &data); //read DQ6 again
		data &= 0x40;
		if(predata == data){
			return OK;
		}
		predata = data;
		retry++;
	}
	return NG;
}

/*
---- polling check ----
databit7
*/
static int polling_check(const struct flash_order *d, long address, u8 truedata)
{
	int retry = 0;
	
	truedata &= 0x80;
	while(retry < CHECK_RETRY_MAX){
		u8 data;
		d->read(address, 1, &data);
		data &= 0x80;
		if(truedata == data){
			return OK;
		}
		retry++;
	}
	return NG;
}

static void bootblock_lockout(const struct flash_order *d)
{
	u8 dummy[3];
	command_set(d, PP);
	d->read(0x8000 ,3, dummy);
	printf("%02x %02x %02x \n", dummy[0], dummy[1], dummy[2]);
	d->read(0xfff2 ,1, dummy);
	command_set(d, PRODUCTID_EXIT);
}
/*
---- erase ----
*/
static void flash_erase(const struct flash_order *d)
{
	if(0) bootblock_lockout(d);
	command_set(d, ERASE);
	toggle_check(d, d->command_2aaa);
	Sleep(200); //Tec 0.2 sec
}

/*
---- program ----
*/
static int program_byte(const struct flash_order *d, long address, const u8 *data, long length)
{
	while(length != 0){
		if(*data != 0xff){
			u8 dummy;
			d->read(address, 1, &dummy);
			if(*data != dummy){
				printf("%s %06x\n", __FUNCTION__, (int) address);
				fflush(stdout);
				command_set(d, PROTECT_DISABLE);
				d->flash_write(address, *data);
				if(toggle_check(d, address) == NG){
					if(DEBUG == 1){
						printf("%s NG\n", __FUNCTION__);
					}
					return NG;
				}
				Sleep(1);
			}
		}
		address++;
		data++;
		length--;
	}
	return OK;
}

static int program_pagewrite(const struct flash_order *d, long address, const u8 *data, long length)
{
	const long toggle_address = address ;
	command_set(d, PROTECT_DISABLE);
	while(length != 0){
		d->flash_write(address, *data);
		address++;
		data++;
		length--;
	}
	Sleep(15);
	int ret = toggle_check(d, toggle_address);
	if(0){
		data--;
		address -= 1;
		polling_check(d, address - 1, *data);
	}

	//command_set(d, PROTECT_ENABLE);
	//Sleep(15);
	return ret;
}

/*
---- block compare ----
*/
static void compare(const struct flash_order *d, long address, const u8 *data, long length)
{
	u8 *romdata, *r;
	int count = 0;
	romdata = malloc(length);
	d->read(address, length, romdata);
	r = romdata;
	while(length != 0){
		if(*r != *data){
			char safix = ' ';
			if((count & 7) == 7){
				safix = '\n';
			}
			count++;
			printf("%06x%c", (int)address, safix);
		}
		r++;
		data++;
		address++;
		length--;
	}
	free(romdata);
}

/*
固有デバイスドライバ
*/
static void w49f002_init(const struct flash_order *d)
{
/*
byte program mode では 1->0 にするだけ。 0->1 は erase のみ。
よって初期化時には erase を実行する
*/
	flash_erase(d);
}

static void w49f002_write(const struct flash_order *d, long address, long length, const struct memory *m)
{
	int writemiss = 0;
	int retry = 0;
	const u8 *data;
	u8 *compare;
	
	data = m->data;
	compare = malloc(length);
	do{
		if(program_byte(d, address, data, length) == NG){
			break;
		}
		d->read(address, length, compare);
		writemiss = memcmp(compare, data, length);
		if(retry > 20){
			printf("%s retry error\n", __FUNCTION__);
			break;
		}
		retry++;
	}while(writemiss != 0);
	free(compare);
}


static void w29c020_init(const struct flash_order *d)
{
/*
page write mode ではとくになし
*/
}

static void w29c020_write(const struct flash_order *d, long address, long length, const struct memory *m)
{
	int retry = 0;
	{
		long a = address;
		long i = length;
		const u8 *dd;
		u8 *cmp;

		dd = m->data;
		cmp = malloc(d->pagesize);
		while(i != 0){
			int result = program_pagewrite(d, a, dd, d->pagesize);
			if(result == NG){
				printf("%s: write error\n", __FUNCTION__);
				free(cmp);
				return;
			}
			d->read(a, d->pagesize, cmp);
			if(memcmp(cmp, dd, d->pagesize) == 0){
				a += d->pagesize;
				dd += d->pagesize;
				i -= d->pagesize;
			}else{
				if(retry >= 0x100){
					break;
				}
				retry++;
			}
		}
		free(cmp);
	}

	printf("write ok. retry %d\n", retry);
	compare(d, address, m->data, length);
	Sleep(10);
}

static void w29c040_write(const struct flash_order *d, long address, long length, const struct memory *m)
{
	u8 *cmp;
	int ngblock = 0;
	int retry = 0;
	cmp = malloc(d->pagesize);
	do{
		long a = address;
		long i = length;
		long offset = m->offset;
		const u8 *dd;

		dd = m->data;
		ngblock = 0;
		while(i != 0){
			d->read(a, d->pagesize, cmp);
			if(memcmp(cmp, dd, d->pagesize) != 0){
				ngblock++;
				printf("write %s 0x%06x\n", m->name, (int) offset);
				int result = program_pagewrite(d, a, dd, d->pagesize);
				if(result == NG){
					printf("%s: write error\n", __FUNCTION__);
					free(cmp);
					return;
				}
			}
			a += d->pagesize;
			dd += d->pagesize;
			offset += d->pagesize;
			i -= d->pagesize;
		}
		printf("%s 0x%06x, ngblock %d\n", m->name, (int) offset, ngblock);
		if(retry >= 3 && ngblock >= 16){
			printf("skip\n");
			break;
		}
		else if(retry > 12){
			printf("skip\n");
			break;
		}
		retry++;
		fflush(stdout);
	}while(ngblock != 0);

	free(cmp);
//	compare(d, address, data, length);
//	Sleep(10);
}

/*
デバイスリスト
*/
static const struct flash_driver DRIVER_W29C020 = {
	.name = "W29C020",
	.capacity = 0x40000,
	.pagesize = 0x80,
	.id_manufacurer = 0xda,
	.id_device = 0x45,
	.productid_check = productid_check,
#if DEBUG==1
	.erase = flash_erase,
#endif
	.init = w29c020_init,
	.write = w29c020_write
};

static const struct flash_driver DRIVER_W29C040 = {
	.name = "W29C040",
	.capacity = 0x80000,
	.pagesize = 0x100,
	.id_manufacurer = 0xda,
	.id_device = 0x46,
	.productid_check = productid_check,
#if DEBUG==1
	.erase = flash_erase,
#endif
	.init = w29c020_init,
	.write = w29c040_write
};

static const struct flash_driver DRIVER_W49F002 = {
	.name = "W49F002",
	.capacity = 0x40000,
	.pagesize = 0,
	.id_manufacurer = 0xda,
	.id_device = 0xae,
	.productid_check = productid_check,
#if DEBUG==1
	.erase = flash_erase,
#endif
	.init = w49f002_init,
	.write = w49f002_write
};

static const struct flash_driver *DRIVER_LIST[] = {
	&DRIVER_W29C020, &DRIVER_W29C040, &DRIVER_W49F002,
	NULL
};

const struct flash_driver *flash_driver_get(const char *name)
{
	const struct flash_driver **d;
	d = DRIVER_LIST;
	while(*d != NULL){
		if(strcmp(name, (*d)->name) == 0){
			return *d;
		}
		d++;
	}
	return NULL;
}
