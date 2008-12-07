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

flashmemory.c �����ηٹ�
���Υ����������ɤ򻲹͡�ž�Ѥ��ƥ������������ʤɤ����פ����ʤ����ȡ�
Ƚ���������� LGPL ��Ŭ�Ѥ��졢�����ս�Υ��������������ɬ�פ����롣
*/
#ifndef _FLASHMEMORY_H_
#define _FLASHMEMORY_H_
struct flash_order{
	//JEDEC command �򽼤Ƥ� CPU/PPU �������ɥ쥹
	long command_0000, command_2aaa, command_5555;
	//struct reader_driver �δؿ��ݥ��󥿤��Ϥ����
	void (*flash_write)(long address, long data);
	void (*read)(long address, long length, u8 *data);
};

struct flash_driver{
	const char *name;
	long capacity;
	u8 id_manufacurer, id_device;
	int (*productid_check)(const struct flash_order *d, const struct flash_driver *f);
#if DEBUG==1
	void (*erase)(const struct flash_order *d);
#endif
	void (*init)(const struct flash_order *d);
	void (*write)(const struct flash_order *d, long address, long length, const u8 *data);
};

const struct flash_driver *flash_driver_get(const char *name);
#endif
