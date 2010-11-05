#include <string.h>
#include "type.h"
#include "widget.h"
#include "reader_master.h"
#include "memory_manage.h"

struct reader_handle{
	void (*except)(const wgChar *str);
	const struct textcontrol *log;
};

static const struct reader_handle *dummy_open(void (*except)(const wgChar *str), const struct textcontrol *log)
{
	struct reader_handle *h;
	h = Malloc(sizeof(struct reader_handle));
	h->except = except;
	h->log = log;
	return h;
}

static void dummy_close(const struct reader_handle *h)
{
	Free((void *) h);
}

/*static void throw(const struct reader_handle *h)
{
#ifdef _UNICODE
		size_t length = strlen(usb_strerror());
		wchar_t *mm = Malloc(sizeof(wchar_t) * (length + 1));
		mbstowcs(mm, usb_strerror(), length + 1);
		h->except(mm);
		Free(mm);
#else
		h->except(usb_strerror());
#endif
}*/

static void dummy_read(const struct reader_handle *h, const struct gauge *g, long address, long length, uint8_t *data)
{
	const int packet = 0x200;
	while(length >= packet){
		wait(10);
		memset(data, 2, packet);
		data += packet;
		length -= packet;
		g->value_add(g->bar, g->label, packet);
	}
	if(length != 0){
		memset(data, 33, length);
		g->value_add(g->bar, g->label, length);
	}
}

static void dummy_init(const struct reader_handle *h)
{
}

static void dummy_cpu_write(const struct reader_handle *h, long address, long length, const uint8_t *data)
{
	if(length == 1){
		h->log->append(h->log->object, wgT(" cpu_write $%04x <- $%02x\n"), (int) address, *data);
	}
	Sleep(4);
}

static void dummy_write(const struct reader_handle *h, long address, long length, const uint8_t *data)
{
	Sleep(4);
}

static void dummy_flash_config(const struct reader_handle *h, long c000x, long c2aaa, long c5555, long unit, bool retry)
{
}

static void dummy_flash_erase(const struct reader_handle *h, long address, bool dowait)
{
	if(dowait == true){
		wait(10);
	}
}

static long dummy_flash_program(const struct reader_handle *h, const struct gauge *g, long address, long length, const uint8_t *data, bool dowait, bool skip)
{
	if(dowait == true){
		wait(20);
	}
	g->value_add(g->bar, g->label, 0x200);
	return 0x200;
}

static void dummy_flash_status(const struct reader_handle *h, uint8_t s[2])
{
	s[0] = 0;
	s[1] = 0;
}

static void dummy_flash_device_get(const struct reader_handle *h, uint8_t s[2])
{
	s[0] = 0;
	s[1] = 0;
}

static uint8_t dummy_vram_connection(const struct reader_handle *h)
{
	return 0;
}

const struct reader_driver DRIVER_DUMMY = {
	.cpu = {
		.memory_read = dummy_read, 
		.memory_write = dummy_cpu_write,
		.flash_config = dummy_flash_config,
		.flash_erase = dummy_flash_erase,
		.flash_program = dummy_flash_program,
		.flash_device_get = dummy_flash_device_get
	}, .ppu = {
		.memory_read = dummy_read,
		.memory_write = dummy_write,
		.flash_config = dummy_flash_config,
		.flash_erase = dummy_flash_erase,
		.flash_program = dummy_flash_program,
		.flash_device_get = dummy_flash_device_get
	}, .control  = {
		.name = wgT("dummy"),
		.open = dummy_open, .close = dummy_close,
		.init = dummy_init,
		.flash_status = dummy_flash_status,
		.vram_connection = dummy_vram_connection
	}
};
