/*
famicom ROM cartridge utility - unagi
flash memory driver

Copyright (C) 2008 鰻開発協同組合

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
static const struct flash_task ERASE_CHIP[] = {
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x80},
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x10},
	{FLASH_COMMAND_END, 0}
};

static const struct flash_task ERASE_SECTOR[] = {
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	{ADDRESS_5555, 0x80},
	{ADDRESS_5555, 0xaa},
	{ADDRESS_2AAA, 0x55},
	//このあと sectoraddress に 0x30 を write
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
#define dprintf if(DEBUG==1) printf
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

static int productid_sram(const struct flash_order *d, const struct flash_driver *f)
{
	return OK;
}
/*
---- toggle check ----
databit6
*/
const int CHECK_RETRY_MAX = 0x10000;
static int toggle_check_d6(const struct flash_order *d, long address)
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

static int toggle_check_d2d5d6(const struct flash_order *d, long address)
{
	u8 predata;
	int retry = 0;
	d->read(address, 1, &predata);
	do{
		u8 data;
		d->read(address, 1, &data);
		//DQ6 toggle check
		if((predata & 0x40) == (data & 0x40)){
			return OK;
		}
		//DQ5 == 0 ならやりなおし
		dprintf("toggle out %06x \n", (int) address);
		if(data & 0x20){
			//recheck toggle bit, read twice
			u8 t[2];
			d->read(address, 1, &t[0]);
			d->read(address, 1, &t[1]);
			if((t[0] & 0x40) == (t[1] & 0x40)){
				return OK;
			}
			//Program/Erase operation not complete, write reset command.
			return NG;
		}
		retry++;
	}while(retry < CHECK_RETRY_MAX);
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

/*
---- erase ----
*/
static void flash_erase_chip_2aaa(const struct flash_order *d)
{
	command_set(d, ERASE_CHIP);
	toggle_check_d6(d, d->command_2aaa);
	Sleep(200); //Tec 0.2 sec
}

static void flash_erase_chip_02aa(const struct flash_order *d)
{
	command_set(d, ERASE_CHIP);
	toggle_check_d2d5d6(d, d->command_2aaa);
	Sleep(5200); //AM29F020 1.0 sec なんだけどこれって sector 単位?
}

#if DEBUG==1
static void sram_erase(const struct flash_order *d)
{
	//bank 切り替えが伴うので実装できない
}
#endif

/*
---- program ----
*/
static int program_byte(const struct flash_order *d, long address, const u8 *data, long length)
{
	int retry = 0;
	while(length != 0){
		if(*data != 0xff){
			fflush(stdout);
			command_set(d, PROTECT_DISABLE);
			d->flash_write(address, *data);
			if(toggle_check_d6(d, address) == NG){
				dprintf("%s NG\n", __FUNCTION__);
				return NG;
			}
		}
		u8 dummy;
		d->read(address, 1, &dummy);
		if(*data == dummy){
			address++;
			data++;
			length--;
			retry = 0;
		}else if(retry > 8){
			dprintf("%s %06x error\n", __FUNCTION__, (int) address);
			address++;
			data++;
			length--;
			retry = 0;
			if(DEBUG == 0){
				return NG;
			}
		}else{
			retry++;
		}
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
	int ret = toggle_check_d6(d, toggle_address);
	if(0){
		data--;
		polling_check(d, address - 1, *data);
	}

	return ret;
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
	flash_erase_chip_2aaa(d);
}

static void w49f002_write(const struct flash_order *d, long address, long length, const struct memory *m)
{
	program_byte(d, address, m->data, length);
//	dprintf("write %s 0x%06x done\n", m->name, (int) m->offset);
}

static void am29f040_init(const struct flash_order *d)
{
	flash_erase_chip_02aa(d);
}

static void am29f040_write(const struct flash_order *d, long address, long length, const struct memory *m)
{
	const u8 *data;
	data = m->data;
	while(length != 0){
		if(*data != 0xff){
			command_set(d, PROTECT_DISABLE);
			d->flash_write(address, *data);
			if(toggle_check_d2d5d6(d, address) == NG){
				dprintf("%s NG\n", __FUNCTION__);
				return;
			}
		}
		address++;
		data++;
		length--;
	}
}

static void init_nop(const struct flash_order *d)
{
/*
page write mode ではとくになし
*/
}

static void w29c040_write(const struct flash_order *d, long address, long length, const struct memory *m)
{
	u8 *cmp;
	int ngblock = 0;
	int retry = 0;
	assert(d->pagesize != 0);
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
				dprintf("write %s 0x%06x\n", m->name, (int) offset);
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
		dprintf("%s 0x%06x, ngblock %d\n", m->name, (int) m->offset, ngblock);
		if(retry >= 3 && ngblock >= 16){
			dprintf("skip\n");
			break;
		}
		else if(retry > 12){
			dprintf("skip\n");
			break;
		}
		retry++;
		fflush(stdout);
	}while(ngblock != 0);

	free(cmp);
}

static void sram_write(const struct flash_order *d, long address, long length, const struct memory *m)
{
	const u8 *data;
	data = m->data;
	while(length != 0){
		d->flash_write(address, *data);
		address++;
		data++;
		length--;
	}
}

static void dummy_write(const struct flash_order *d, long address, long length, const struct memory *m)
{
}

/*
デバイスリスト
*/
//0x80 以降は本当のデバイス重複しないと思う. 誰か JEDEC のとこをしらべて.
static const struct flash_driver DRIVER_SRAM256K = {
	.name = "SRAM256K",
	.capacity = 0x8000,
	.pagesize = 0,
	.id_manufacurer = FLASH_ID_DEVICE_SRAM,
	.id_device = FLASH_ID_DEVICE_SRAM,
	.productid_check = productid_sram,
#if DEBUG==1
	.erase = sram_erase,
#endif
	.init = init_nop,
	.write = sram_write
};

static const struct flash_driver DRIVER_DUMMY = {
	.name = "dummy",
	.capacity = 0x40000,
	.pagesize = 0,
	.id_manufacurer = FLASH_ID_DEVICE_DUMMY,
	.id_device = FLASH_ID_DEVICE_DUMMY,
	.productid_check = productid_sram,
#if DEBUG==1
	.erase = sram_erase,
#endif
	.init = init_nop,
	.write = dummy_write
};

static const struct flash_driver DRIVER_W29C020 = {
	.name = "W29C020",
	.capacity = 0x40000,
	.pagesize = 0x80,
	.id_manufacurer = 0xda,
	.id_device = 0x45,
	.productid_check = productid_check,
#if DEBUG==1
	.erase = flash_erase_chip_2aaa,
#endif
	.init = init_nop,
	.write = w29c040_write
};

static const struct flash_driver DRIVER_W29C040 = {
	.name = "W29C040",
	.capacity = 0x80000,
	.pagesize = 0x100,
	.id_manufacurer = 0xda,
	.id_device = 0x46,
	.productid_check = productid_check,
#if DEBUG==1
	.erase = flash_erase_chip_2aaa,
#endif
	.init = init_nop,
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
	.erase = flash_erase_chip_2aaa,
#endif
	.init = w49f002_init,
	.write = w49f002_write
};

/*
MANUFATUTER ID 0x7f1c
EN29F002T DEVICE ID 0x7f92
EN29F002B DEVICE ID 0x7f97

command address が 0x00555, 0x00aaa になってる
*/
static const struct flash_driver DRIVER_EN29F002T = {
	.name = "EN29F002T",
	.capacity = 0x40000,
	.pagesize = 0,
	.id_manufacurer = 0x1c,
	.id_device = 0x92,
	.productid_check = productid_check,
#if DEBUG==1
	.erase = flash_erase_chip_02aa,
#endif
	.init = am29f040_init,
	.write = am29f040_write
};

static const struct flash_driver DRIVER_AM29F040B = {
	.name = "AM29F040B",
	.capacity = 0x80000,
	.pagesize = 0,
	.id_manufacurer = 0x01,
	.id_device = 0xa4,
	.productid_check = productid_check,
#if DEBUG==1
	.erase = flash_erase_chip_02aa,
#endif
	.init = am29f040_init,
	.write = am29f040_write
};

static const struct flash_driver *DRIVER_LIST[] = {
	&DRIVER_W29C020, &DRIVER_W29C040, 
	&DRIVER_W49F002, &DRIVER_EN29F002T, &DRIVER_AM29F040B,
	&DRIVER_SRAM256K, 
	&DRIVER_DUMMY,
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
