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
#include "flashmemory.h"
/*
driver for Winbond W29C020, W49F002
*/
/*
----memory detail----
VRC6 CPU memory bank
cpu address|rom address    |page|task
$8000-$bfff|0x00000-0x03fff|0   |write (0x2aaa & 0x3fff) + 0x8000
$c000-$dfff|0x04000-0x05fff|2   |write (0x5555 & 0x1fff) + 0xc000
$e000-$efff|0x3e000-0x3ffff|fix |boot area

MMC3 CPU memory bank
cpu address|rom address    |page|task
$8000-$9fff|0x02000-0x03fff|1   |write (0x2aaa & 0x1fff) + 0x8000
$a000-$bfff|0x04000-0x05fff|2   |write (0x5555 & 0x1fff) + 0xa000
$c000-$efff|末尾           |fix |boot area

generic CPU memory bank
cpu address|rom address    |page|task
$8000-$9fff|0x02000-0x03fff|1   |write (0x2aaa & 0x1fff) + 0x8000
$a000-$bfff|0x04000-0x05fff|2   |write (0x5555 & 0x1fff) + 0xa000
$c000-$dfff|n * 0x2000     |n   |write area
$e000-$efff|末尾           |fix |boot area, 未使用

generic PPU memory bank
ppu address|rom address    |page|task
$0000-$03ff|0x02800-0x02bff|0x0a|write (0x2aaa & 0x03ff) + 0
$0400-$07ff|0x05400-0x057ff|0x15|write (0x5555 & 0x03ff) + 0x400
$0800-$0fff|未使用
$1000-$1fff|n * 0x0400     |n   |write area
*/
/*
JEDEC flash memory command
http://www.sst.com/downloads/software_driver/SST49LF002A.txt
*/
struct flash_task{
	long address, data;
};
enum{
	flash_task_end = 0x46494649
};
static const struct flash_task PRODUCTID_ENTRY[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x90},
	{flash_task_end, 0}
};
static const struct flash_task PRODUCTID_EXIT[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0xf0},
	{flash_task_end, 0}
};
static const struct flash_task PROTECT_DISABLE[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0xa0},
	{flash_task_end, 0}
};
static const struct flash_task PROTECT_ENABLE[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x80},
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x20},
	{flash_task_end, 0}
};
//boot lock lockout enable をいれないとバンク切り替えをすると
//先頭の0x100byteぐらいが書き換えられる?
static const struct flash_task BOOTBLOCK_FIRST[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x80},
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x40},
	{0x0000, 0},
	{flash_task_end, 0}
};
static const struct flash_task ERASE[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x80},
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x10},
	{flash_task_end, 0}
};

static void task_set(const struct flash_order *d, const struct flash_task *t)
{
	while(t->address != flash_task_end){
		long logical_address = 0;
		switch(t->address){
		case 0:
			logical_address = d->task_0000;
			break;
		case 0x2aaa:
			logical_address = d->task_2aaa;
			break;
		case 0x5555:
			logical_address = d->task_5555;
			break;
		default:
			assert(0);
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
	task_set(d, PRODUCTID_ENTRY);
	d->read(d->task_0000, 3, data);
	task_set(d, PRODUCTID_EXIT);
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

/*
---- erase ----
*/
static void flash_erase(const struct flash_order *d)
{
	task_set(d, ERASE);
	toggle_check(d, d->task_2aaa);
	//Sleep(200); //Tec 0.2 sec
}

/*
---- program ----
*/
static int program_byte(const struct flash_order *d, long address, const u8 *data, long length)
{
	while(length != 0){
		if(*data != 0xff){
			task_set(d, PROTECT_DISABLE);
			d->flash_write(address, *data);
			if(toggle_check(d, address) == NG){
				if(DEBUG == 1){
					printf("%s NG\n", __FUNCTION__);
				}
				return NG;
			}
			if(0){
			u8 putdata;
			d->read(address, 1, &putdata);
			if(putdata != *data){
				printf("%s %06x retry\n", __FUNCTION__, (int) address);
				continue;
			}
			}
		}
		if((DEBUG == 1) && (address & 0x1f) == 0){
			printf("%s %06x\n", __FUNCTION__, (int) address);
			fflush(stdout);
		}
		address++;
		data++;
		length--;
	}
	return OK;
}

static int program_pagewrite(const struct flash_order *d, long address, const u8 *data, long length)
{
	const long toggle_address = address;
	task_set(d, PROTECT_DISABLE);
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

	//task_set(d, PROTECT_ENABLE);
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
static void w49f002_write(const struct flash_order *d)
{
	program_byte(d, d->address, d->data, d->length);
	compare(d, d->address, d->data, d->length);
}

static void w29c020_write(const struct flash_order *d)
{
	const long pagesize = 0x80;
	int retry = 0;
	{
		long a = d->address;
		long i = d->length;
		const u8 *dd;
		u8 *cmp;

		dd = d->data;
		cmp = malloc(pagesize);
		while(i != 0){
			int result = program_pagewrite(d, a, dd, pagesize);
			if(result == NG){
				printf("%s: write error\n", __FUNCTION__);
				free(cmp);
				return;
			}
			d->read(a, pagesize, cmp);
			if(memcmp(cmp, dd, pagesize) == 0){
				a += pagesize;
				dd += pagesize;
				i -= pagesize;
			}else{
				retry++;
			}
		}
		free(cmp);
	}

	printf("write ok. retry %d\n", retry);
	compare(d, d->address, d->data, d->length);
	task_set(d, BOOTBLOCK_FIRST);
	Sleep(10);
}

/*
デバイスリスト
*/
static const struct flash_driver DRIVER_W29C020 = {
	name: "W29C020",
	capacity: 0x40000,
	id_manufacurer: 0xda,
	id_device: 0x45,
	productid_check: productid_check,
	erase: flash_erase,
	write: w29c020_write
};

static const struct flash_driver DRIVER_W49F002 = {
	name: "W49F002",
	capacity: 0x40000,
	id_manufacurer: 0xda,
	id_device: 0xae,
	productid_check: productid_check,
	erase: flash_erase,
	write: w49f002_write
};

static const struct flash_driver *DRIVER_LIST[] = {
	&DRIVER_W29C020, &DRIVER_W49F002,
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
