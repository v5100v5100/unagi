#ifndef _READER_MASTER_H_
#define _READER_MASTER_H_
#include "type.h"
struct reader_driver{
	void (*init)(void);
	void (*cpu_read)(long address, long length, u8 *data);
	void (*ppu_read)(long address, long length, u8 *data);
	void (*cpu_6502_write)(long address, long data);
	void (*cpu_flash_write)(long address, long data);
	void (*ppu_write)(long address, long data);
	const char *name;
};
const struct reader_driver *reader_driver_get(const char *name);
#endif
