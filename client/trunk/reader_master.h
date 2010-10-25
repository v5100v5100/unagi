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

struct reader_memory_access{
	void (*memory_read)(const int *d, const struct gauge *g, long address, long length, uint8_t *data);
	void (*memory_write)(const int *d, long address, long length, const uint8_t *data);
	void (*flash_config)(const int *d, long c000x, long c2aaa, long c5555, long unit, bool retry);
	void (*flash_erase)(const int *d, long address, bool wait);
	long (*flash_program)(const int *d, const struct gauge *g, long address, long length, const uint8_t *data, bool wait, bool skip);
	void (*flash_device_get)(const int *d, uint8_t s[2]);
};
struct reader_control{
	const char *name;
	const int *(*open)(void);
	void (*close)(const int *h);
	void (*init)(const int *d);
	void (*flash_status)(const int *d, uint8_t s[2]);
	uint8_t (*vram_connection)(const int *d);
};
struct reader_driver{
	const struct reader_control control;
	const struct reader_memory_access cpu, ppu;
};

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
