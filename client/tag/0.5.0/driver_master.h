#ifndef _DRIVER_MASTER_H_
#define _DRIVER_MASTER_H_
#include "type.h"
struct driver{
	void (*init)(void);
	void (*cpu_read)(long address, long length, u8 *data);
	void (*ppu_read)(long address, long length, u8 *data);
	void (*cpu_write)(long address, long data);
	void (*ppu_write)(long address, long data);
	const char *name;
};
const struct driver *driver_get(const char *name);
#endif
