#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <usb.h>
#include <kazzo_request.h>
#include <kazzo_task.h>
#include "memory_manage.h"
#include "reader_master.h"
#include "usb_device.h"
#include "reader_kazzo.h"
#include "widget.h"

static inline usb_dev_handle *device_open(void)
{
	usb_dev_handle *handle = NULL;
	const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID};
	const unsigned char rawPid[2] = {USB_CFG_DEVICE_ID};
	char vendor[] = {USB_CFG_VENDOR_NAME, 0};
	char product[] = {USB_CFG_DEVICE_NAME, 0};
	int vid, pid;

	/* compute VID/PID from usbconfig.h so that there is a central source of information */
	vid = (rawVid[1] << 8) | rawVid[0];
	pid = (rawPid[1] << 8) | rawPid[0];

	if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) == 0){
		return handle;
	}
	return NULL;
}

struct reader_handle{
	usb_dev_handle *handle;
	void (*except)(const char *str);
};

static const struct reader_handle *kazzo_open(void (*except)(const char *str))
{
	struct reader_handle *h;
	usb_dev_handle *usb = device_open();
	if(usb == NULL){
		return NULL;
	}
	h = Malloc(sizeof(struct reader_handle));
	h->handle = usb;
	h->except = except;
	return h;
}

static void kazzo_close(const struct reader_handle *h)
{
	usb_close(h->handle);
	Free((void *) h);
}
enum{
	TIMEOUT = 4000
};
//-------- read sequence --------
static void device_read(const struct reader_handle *h, enum request r, enum index index, long address, long length, uint8_t *data)
{
	int cnt = usb_control_msg(
		h->handle, 
		USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
		r, address, 
		index, (char *) data, length, TIMEOUT
	);
	if(cnt != length){
		h->except(usb_strerror());
//		puts(__FUNCTION__);
//		puts(usb_strerror());
//		exit(1);
	}
}
static void read_main(const struct reader_handle *h, const struct gauge *g, const enum request r, enum index index, long address, long length, uint8_t *data)
{
	const int packet = READ_PACKET_SIZE;
	while(length >= packet){
		device_read(
			h, r, index, address, packet, data
		);
		data += packet;
		address += packet;
		length -= packet;
		g->value_add(g->bar, g->label, packet);
	}
	if(length != 0){
		device_read(
			h, r, index, address, length, data
		);
		g->value_add(g->bar, g->label, packet);
	}
}
static void kazzo_cpu_read(const struct reader_handle *h, const struct gauge *g, long address, long length, uint8_t *data)
{
	read_main(h, g, REQUEST_CPU_READ, INDEX_IMPLIED, address, length, data);
//	read_main(REQUEST_CPU_READ_6502, address, length, data);
}
static void kazzo_ppu_read(const struct reader_handle *h, const struct gauge *g, long address, long length, uint8_t *data)
{
	read_main(h, g, REQUEST_PPU_READ, INDEX_IMPLIED, address, length, data);
}
//-------- write sequence --------
/*
When host send data that contains 0xff many times, v-usb losts some 
bits. To prevent losting bits, mask data xor 0xa5;
*/
static void device_write(const struct reader_handle *h, enum request w, enum index index, long address, long length, const uint8_t *data)
{
	uint8_t *d = Malloc(length);
	int i;
	memcpy(d, data, length);
	for(i = 0; i < length; i++){
		d[i] ^= 0xa5;
	}
	int cnt = usb_control_msg(
		h->handle, 
		USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
		w, address, 
		index, (char *) d, length, TIMEOUT
	);
	if(cnt != length){
//		puts(__FUNCTION__);
//		puts(usb_strerror());
//		exit(1);
		h->except(usb_strerror());
	}
	Free(d);
}

static void kazzo_init(const struct reader_handle *h)
{
	device_write(h, REQUEST_PHI2_INIT, INDEX_IMPLIED, 0, 0, NULL);
}

static void write_memory(const struct reader_handle *h, enum request r, long address, long length, const uint8_t *data)
{
	while(length != 0){
		long l = length >= FLASH_PACKET_SIZE ? FLASH_PACKET_SIZE : length;
		device_write(h, r, INDEX_IMPLIED, address, l, data);
		address += l;
		data += l;
		length -= l;
	}
}
static void kazzo_cpu_write_6502(const struct reader_handle *h, long address, long length, const uint8_t *data)
{
	write_memory(h, REQUEST_CPU_WRITE_6502, address, length, data);
}

static void kazzo_ppu_write(const struct reader_handle *h, long address, long length, const uint8_t *data)
{
	write_memory(h, REQUEST_PPU_WRITE, address, length, data);
}

static inline void pack_short_le(long l, uint8_t *t)
{
	t[0] = l & 0xff;
	t[1] = (l >> 8) & 0xff;
}
static void flash_config(const struct reader_handle *h, enum request r, enum index index, long c000x, long c2aaa, long c5555, long unit, bool retry)
{
	const int size = 2 * 4 + 1;
	uint8_t buf[size];
	uint8_t *t = buf;
	assert(unit >= 1 && unit < 0x400);
	pack_short_le(c000x, t);
	t += sizeof(uint16_t);
	pack_short_le(c2aaa, t);
	t += sizeof(uint16_t);
	pack_short_le(c5555, t);
	t += sizeof(uint16_t);
	pack_short_le(unit, t);
	t += sizeof(uint16_t);
	*t = retry == true ? 1 : 0;
	device_write(h, r, index, 0, size, buf);
}
static void kazzo_cpu_flash_config(const struct reader_handle *h, long c000x, long c2aaa, long c5555, long unit, bool retry)
{
	flash_config(h, REQUEST_FLASH_CONFIG_SET, INDEX_CPU, c000x, c2aaa, c5555, unit, retry);
}
static void kazzo_ppu_flash_config(const struct reader_handle *h, long c000x, long c2aaa, long c5555, long unit, bool retry)
{
	flash_config(h, REQUEST_FLASH_CONFIG_SET, INDEX_PPU, c000x, c2aaa, c5555, unit, retry);
}

static inline void flash_execute(const struct reader_handle *h, enum request p, enum request s, enum index index, long address, const uint8_t *data, int size, bool dowait, bool skip)
{
	uint8_t status;
	int filled = 1;
	if(skip == true){
		uint8_t *filldata = Malloc(size);
		memset(filldata, 0xff, size);
		filled = memcmp(filldata, data, size);
		if(0){ //nesasm fill 0 to unused area. When this routine is enabled, programming will speed up and compare mode will not work.
			memset(filldata, 0, size);
			filled &= memcmp(filldata, data, size);
		}
		Free(filldata);
	}
	if(filled != 0){
		device_write(h, p, index, address, size, data);
	}
	if(dowait == true){
		do{
			wait(10);
			device_read(h, s, index, 0, 1, &status);
		}while(status != KAZZO_TASK_FLASH_IDLE);
	}
}
static void kazzo_cpu_flash_erase(const struct reader_handle *h, long address, bool dowait)
{
	flash_execute(h, REQUEST_FLASH_ERASE, REQUEST_FLASH_STATUS, INDEX_CPU, address, NULL, 0, dowait, false);
}
static void kazzo_ppu_flash_erase(const struct reader_handle *h, long address, bool dowait)
{
	flash_execute(h, REQUEST_FLASH_ERASE, REQUEST_FLASH_STATUS, INDEX_PPU, address, NULL, 0, dowait, false);
}

static void dump(const uint8_t *w, const uint8_t *r, long length)
{
	while(length != 0){
		if(memcmp(r, w, 0x10) != 0){
			int i;
			printf("* ");
			for(i = 0; i < 0x10; i+=4){
				printf("%02x %02x %02x %02x-", w[i], w[i+1], w[i+2], w[i+3]);
			}
			puts("");
			printf("  ");
			for(i = 0; i < 0x10; i+=4){
				printf("%02x %02x %02x %02x-", r[i], r[i+1], r[i+2], r[i+3]);
			}
			puts("");
		}
		w += 0x10;
		r += 0x10;
		length -= 0x10;
	}
}
static long flash_program(const struct reader_handle *h, const struct gauge *g, enum index index, long address, long length, const uint8_t *data, bool dowait, bool skip)
{
	enum request p = REQUEST_FLASH_PROGRAM;
	enum request s = REQUEST_FLASH_STATUS;
	if(dowait == false){
		flash_execute(h, p, s, index, address, data, FLASH_PACKET_SIZE, dowait, skip);
		g->value_add(g->bar, g->label, FLASH_PACKET_SIZE);
		return FLASH_PACKET_SIZE;
	}
	long count = 0;
	uint8_t *d = Malloc(FLASH_PACKET_SIZE);
	while(length >= FLASH_PACKET_SIZE){
		flash_execute(h, p, s, index, address, data, FLASH_PACKET_SIZE, dowait, skip);
		if(0){
			//device_read(handle, REQUEST_FLASH_BUFFER_GET, index, 0, FLASH_PACKET_SIZE, d);
			if(memcmp(d, data, FLASH_PACKET_SIZE) != 0){
				puts("packet send error");
				dump(data, d, FLASH_PACKET_SIZE);
			}
		}
		g->value_add(g->bar, g->label, FLASH_PACKET_SIZE);
		address += FLASH_PACKET_SIZE;
		data += FLASH_PACKET_SIZE;
		count += FLASH_PACKET_SIZE;
		length -= FLASH_PACKET_SIZE;
	}
	Free(d);
	return count;
}
static long kazzo_cpu_flash_program(const struct reader_handle *h, const struct gauge *g, long address, long length, const uint8_t *data, bool dowait, bool skip)
{
	return flash_program(h, g, INDEX_CPU, address, length, data, dowait, skip);
}
static long kazzo_ppu_flash_program(const struct reader_handle *h, const struct gauge *g, long address, long length, const uint8_t *data, bool dowait, bool skip)
{
	return flash_program(h, g, INDEX_PPU, address, length, data, dowait, skip);
}

static void kazzo_flash_status(const struct reader_handle *h, uint8_t s[2])
{
	read_main(h, &GAUGE_DUMMY, REQUEST_FLASH_STATUS, INDEX_BOTH, 0, 2, s);
}
static void kazzo_cpu_flash_device_get(const struct reader_handle *h, uint8_t s[2])
{
	read_main(h, &GAUGE_DUMMY, REQUEST_FLASH_DEVICE, INDEX_CPU, 0, 2, s);
}
static void kazzo_ppu_flash_device_get(const struct reader_handle *h, uint8_t s[2])
{
	read_main(h, &GAUGE_DUMMY, REQUEST_FLASH_DEVICE, INDEX_PPU, 0, 2, s);
}
static uint8_t kazzo_vram_connection(const struct reader_handle *h)
{
	uint8_t s;
	read_main(h, &GAUGE_DUMMY, REQUEST_VRAM_CONNECTION, INDEX_IMPLIED, 0, 1, &s);
	return s;
}
const struct reader_driver DRIVER_KAZZO = {
	.cpu = {
		.memory_read = kazzo_cpu_read, 
		.memory_write = kazzo_cpu_write_6502,
		.flash_config = kazzo_cpu_flash_config,
		.flash_erase = kazzo_cpu_flash_erase,
		.flash_program = kazzo_cpu_flash_program,
		.flash_device_get = kazzo_cpu_flash_device_get
	}, .ppu = {
		.memory_read = kazzo_ppu_read,
		.memory_write = kazzo_ppu_write,
		.flash_config = kazzo_ppu_flash_config,
		.flash_erase = kazzo_ppu_flash_erase,
		.flash_program = kazzo_ppu_flash_program,
		.flash_device_get = kazzo_ppu_flash_device_get
	}, .control  = {
		.name = "kazzo",
		.open = kazzo_open, .close = kazzo_close,
		.init = kazzo_init,
		.flash_status = kazzo_flash_status,
		.vram_connection = kazzo_vram_connection
	}
};
