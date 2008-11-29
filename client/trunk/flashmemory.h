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
#ifndef _FLASHMEMORY_H_
#define _FLASHMEMORY_H_
#include "driver_master.h"
struct flash_driver{
	const char *name;
	long capacity;
	u8 id_manufacurer, id_device;
	int (*productid_check)(const struct reader_driver *d, const struct flash_driver *f, long address_0000, long address_2aaa, long address_5555);
	void (*erase)(const struct reader_driver *d, long address_2aaa, long address_5555);
	void (*write)(const struct reader_driver *d, long address, const const u8 *data, long length, long address_2aaa, long address_5555);
};
const struct flash_driver *flash_driver_get(const char *name);
#endif
