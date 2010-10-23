#ifndef _READER_MASTER_H_
#define _READER_MASTER_H_
#include "type.h"
#ifdef WIN32
 #include <windows.h>
#else
 #include <unistd.h>
#endif

struct textcontrol;
struct gauge;
struct reader_handle{
	void *handle;
	struct textcontrol *text;
};

struct reader_memory_access{
	void (*memory_read)(const struct reader_handle *d, const struct gauge *g, long address, long length, uint8_t *data);
	void (*memory_write)(const struct reader_handle *d, long address, long length, const uint8_t *data);
	void (*flash_config)(const struct reader_handle *d, long c000x, long c2aaa, long c5555, long unit, bool retry);
	void (*flash_erase)(const struct reader_handle *d, long address, bool wait);
	long (*flash_program)(const struct reader_handle *d, const struct gauge *g, long address, long length, const uint8_t *data, bool wait, bool skip);
	void (*flash_device_get)(const struct reader_handle *d, uint8_t s[2]);
};
struct reader_control{
	const char *name;
	void (*open)(struct reader_handle *h);
	void (*close)(struct reader_handle *h);
	void (*init)(const struct reader_handle *d);
	void (*flash_status)(const struct reader_handle *d, uint8_t s[2]);
	uint8_t (*vram_connection)(const struct reader_handle *d);
};
struct reader_driver{
	const struct reader_control control;
	const struct reader_memory_access cpu, ppu;
};

#if 0
bool reader_driver_get(const char *name, struct reader_driver);
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
#endif
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
