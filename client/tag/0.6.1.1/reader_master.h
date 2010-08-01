#ifndef _READER_MASTER_H_
#define _READER_MASTER_H_
#include "type.h"
#ifdef WIN32
 #include <windows.h>
#else
 #include <unistd.h>
#endif

//C++ の Class もどきを C で実装している感が増してきた...
enum reader_control{
	READER_OPEN, READER_CLOSE
};
struct reader_driver{
	const char *name;
	int (*open_or_close)(enum reader_control oc);
	void (*init)(void);
	void (*cpu_read)(long address, long length, uint8_t *data);
	void (*cpu_write_6502)(long address, long length, const uint8_t *data);
	void (*ppu_read)(long address, long length, uint8_t *data);
	void (*ppu_write)(long address, long length, const uint8_t *data);
	bool flash_support;
	void (*cpu_flash_config)(long c000x, long c2aaa, long c5555, long unit, bool retry);
	void (*cpu_flash_erase)(long address, bool wait);
	long (*cpu_flash_program)(long address, long length, const uint8_t *data, bool wait, bool skip);
	void (*cpu_flash_device_get)(uint8_t s[2]);
	void (*ppu_flash_config)(long c000x, long c2aaa, long c5555, long unit, bool retry);
	void (*ppu_flash_erase)(long address, bool wait);
	long (*ppu_flash_program)(long address, long length, const uint8_t *data, bool wait, bool skip);
	void (*ppu_flash_device_get)(uint8_t s[2]);
	void (*flash_status)(uint8_t s[2]);
	uint8_t (*vram_connection)(void);
};
int paralellport_open_or_close(enum reader_control oc);
const struct reader_driver *reader_driver_get(const char *name);
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
#ifdef WIN32
	Sleep(msec);
#else
	usleep(msec * 1000);
#endif
}
#endif
