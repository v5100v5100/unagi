#ifndef _READER_MASTER_H_
#define _READER_MASTER_H_
#include "type.h"
//C++ の Class もどきを C で実装している感が増してきた...
struct reader_driver{
	int (*open_or_close)(int oc);
	void (*init)(void);
	void (*cpu_read)(long address, long length, u8 *data);
	void (*ppu_read)(long address, long length, u8 *data);
	void (*cpu_6502_write)(long address, long data);
	void (*cpu_flash_write)(long address, long data);
	void (*ppu_write)(long address, long data);
	const char *name;
};
int paralellport_open_or_close(int oc);
const struct reader_driver *reader_driver_get(const char *name);
enum{
	READER_OPEN, READER_CLOSE
};
#endif
