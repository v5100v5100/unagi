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
#include "driver_master.h"
#include "flashmemory.h"
/*
driver for Winbond W29C020, W49F002
*/
/*
----memory detail----
VRC6 CPU memory bank
cpu address|rom address    |page|task
$8000-$bfff|0x00000-0x03fff|0   |write 0x2aaa+0x8000
$c000-$dfff|0x04000-0x05fff|2   |write 0x5555+0x8000
$e000-$efff|0x3e000-0x3ffff|fix |boot area

MMC3 CPU memory bank
cpu address|rom address    |page|task
$8000-$9fff|0x02000-0x03fff|1   |write 0x2aaa+0x8000-0x2000
$a000-$bfff|0x04000-0x05fff|2   |write 0x5555+0x8000-0x4000
$c000-$efff|末尾           |fix |boot area

generic CPU memory bank
cpu address|rom address    |page|task
$8000-$9fff|0x02000-0x03fff|1   |write 0x2aaa+0x8000-0x2000
$a000-$bfff|0x04000-0x05fff|2   |write 0x5555+0x8000-0x4000
$c000-$dfff|n * 0x2000     |n   |write area
$e000-$efff|末尾           |fix |boot area, 未使用

generic PPU memory bank
ppu address|rom address    |page|task
$0000-$03ff|0x02800-0x02bff|0x0a|write 0x2aaa-0x2800
$0400-$07ff|0x05400-0x057ff|0x15|write 0x5555-0x5400
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
static const struct flash_task ERASE[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x80},
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x10},
	{flash_task_end, 0}
};

static void task_set(const struct reader_driver *d, const struct flash_task *t, long address_2aaa, long address_5555)
{
	while(t->address != flash_task_end){
		long cpu_address = 0;
		switch(t->address){
		case 0x2aaa:
			cpu_address = address_2aaa;
			break;
		case 0x5555:
			cpu_address = address_5555;
			break;
		default:
			assert(0);
		}
		d->cpu_flash_write(cpu_address, t->data);
		t++;
	}
}

/*
---- product ID check ----
*/
static int productid_check(const struct reader_driver *d, const struct flash_driver *f, long address_0000, long address_2aaa, long address_5555)
{
	u8 data[2];
	task_set(d, PRODUCTID_ENTRY, address_2aaa, address_5555);
	d->cpu_read(address_0000, 2, data);
	task_set(d, PRODUCTID_EXIT, address_2aaa, address_5555);
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
static int toggle_check(const struct reader_driver *d, long address)
{
	u8 predata;
	int retry = 0;
	d->cpu_read(address, 1, &predata); //read DQ6
	predata &= 0x40;
	while(retry < CHECK_RETRY_MAX){
		u8 data;
		d->cpu_read(address, 1, &data); //read DQ6 again
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
static int polling_check(const struct reader_driver *d, long address, u8 truedata)
{
	int retry = 0;
	
	truedata &= 0x80;
	while(retry < CHECK_RETRY_MAX){
		u8 data;
		d->cpu_read(address, 1, &data);
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
static void flash_erase(const struct reader_driver *d, long address_2aaa, long address_5555)
{
	task_set(d, ERASE, address_2aaa, address_5555);
	toggle_check(d, address_2aaa);
	//Sleep(200); //Tec 0.2 sec
}

/*
---- program ----
*/
static int program_byte(const struct reader_driver *d, long address, const u8 *data, long length, long address_2aaa, long address_5555)
{
	while(length != 0){
		if(*data != 0xff){
			task_set(d, PROTECT_DISABLE, address_2aaa, address_5555);
			d->cpu_flash_write(address, *data);
		}
		if(toggle_check(d, address) == NG){
			return NG;
		}
		address++;
		data++;
		length--;
	}
	return OK;
}

static int program_pagewrite(const struct reader_driver *d, long address, const u8 *data, long length, long address_2aaa, long address_5555)
{
	task_set(d, PROTECT_DISABLE, address_2aaa, address_5555);
	while(length != 0){
		d->cpu_flash_write(address, *data);
		address++;
		data++;
		length--;
	}
	Sleep(1);
	return toggle_check(d, address);
}

/*
---- block compare ----
*/
static void compare(const struct reader_driver *d, long address, const u8 *data, long length)
{
	u8 *romdata, *r;
	romdata = malloc(length);
	d->cpu_read(address, length, romdata);
	r = romdata;
	while(length != 0){
		if(*r != *data){
			printf("%06x\n", (int)address);
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
static void w49f002_write(const struct reader_driver *d, long address, const u8 *data, long length, long address_2aaa, long address_5555)
{
	program_byte(d, address, data, length, address_2aaa, address_5555);
	compare(d, address, data, length);
}

static void w29c020_write(const struct reader_driver *d, long address, const u8 *data, long length, long address_2aaa, long address_5555)
{
	{
		const long pagesize = 0x80;
		long a = address;
		long i = length;
		const u8 *dd;
		dd = data;
		while(i >= 0){
			int result = program_pagewrite(d, a, dd, pagesize, address_2aaa, address_5555);
			if(result == NG){
				return;
			}
			a += pagesize;
			dd += pagesize;
			i -= pagesize;
		}
	}
	compare(d, address, data, length);
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
