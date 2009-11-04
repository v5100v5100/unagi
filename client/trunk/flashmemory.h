/*
famicom ROM cartridge utility - unagi
flash memory driver

Copyright (C) 2008-2009 ����ȯ��Ʊ�ȹ�

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

flashmemory.c �����ηٹ�
���Υ����������ɤ򻲹͡�ž�Ѥ��ƥ������������ʤɤ����פ����ʤ����ȡ�
Ƚ���������� LGPL ��Ŭ�Ѥ��졢�����ս�Υ��������������ɬ�פ����롣
*/
#ifndef _FLASHMEMORY_H_
#define _FLASHMEMORY_H_

struct flash_order{
	//JEDEC command �򽼤Ƥ� CPU/PPU �������ɥ쥹
	long command_0000, command_2aaa, command_5555;
	long command_mask;
	long pagesize;
	//struct reader_driver �δؿ��ݥ��󥿤��Ϥ����
	void (*config)(long c000, long c2aaa, long c5555, long unit);
	void (*erase)(long address, bool dowait);
	void (*write)(long address, long length, uint8_t *data);
	long (*program)(long address, long length, const uint8_t *data, bool dowait);
};

struct memory;
struct flash_driver{
	const char *name;
	long capacity, pagesize;
	long command_mask;
	long erase_wait; //unit is msec
	u8 id_manufacurer, id_device;
	int (*productid_check)(const struct flash_order *d, const struct flash_driver *f);
	void (*init)(const struct flash_order *d);
	void (*program)(const struct flash_order *d, long address, long length, const struct memory *m);
};
const struct flash_driver FLASH_DRIVER_UNDEF;
const struct flash_driver *flash_driver_get(const char *name);

//0x80 �ʹߤ������ΥǥХ�����ʣ���ʤ��Ȼפ�. ï�� JEDEC �ΤȤ��򤷤�٤�.
enum{
	FLASH_ID_DEVICE_SRAM = 0xf0, 
	FLASH_ID_DEVICE_DUMMY
};
#endif
