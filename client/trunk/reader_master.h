#ifndef _READER_MASTER_H_
#define _READER_MASTER_H_
#include "type.h"
#include <windows.h>
//C++ の Class もどきを C で実装している感が増してきた...
struct reader_driver{
	int (*open_or_close)(int oc);
	void (*init)(void);
	void (*cpu_read)(long address, long length, u8 *data);
	void (*ppu_read)(long address, long length, u8 *data);
	void (*cpu_6502_write)(long address, long data, long wait_msec);
	void (*cpu_flash_write)(long address, long data);
	void (*ppu_write)(long address, long data);
	const char *name;
};
int paralellport_open_or_close(int oc);
const struct reader_driver *reader_driver_get(const char *name);
enum{
	READER_OPEN, READER_CLOSE
};
enum{
	ADDRESS_MASK_A0toA12 = 0x1fff,
	ADDRESS_MASK_A0toA14 = 0x7fff,
	ADDRESS_MASK_A15 = 0x8000
};
enum{ 
	M2_CONTROL_TRUE, M2_CONTROL_FALSE
};
/*
static inline は共有マクロ扱い
*/
static inline int bit_set(int data, const int bit)
{
	data |= 1 << bit;
	return data;
}

static inline int bit_clear(int data, const int bit)
{
	data &= ~(1 << bit);
	return data;
}

static inline void wait(long msec)
{
	if(msec == 0){
		return;
	}
	//const long waittime = 100000;
	Sleep(msec);
}
#endif
