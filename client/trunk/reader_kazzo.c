#include <assert.h>
#include <stdlib.h>
#include <usb.h>
#include <kazzo_request.h>
#include <kazzo_task.h>
#include "reader_master.h"
#include "usb_device.h"
#include "reader_kazzo.h"

static usb_dev_handle *device_open(void)
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
	fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
	return NULL;
}
static usb_dev_handle *handle = NULL;
static int kazzo_open_close(enum reader_control oc)
{
	switch(oc){
	case READER_OPEN:
		handle = device_open();
		return handle == NULL ? NG : OK;
	case READER_CLOSE:
		usb_close(handle);
		handle = NULL;
		return OK;
	}
	return NG;
}
enum{
	TIMEOUT = 4000
};
//-------- read sequence --------
static void device_read(usb_dev_handle *handle, enum request r, long address, long length, uint8_t *data)
{
	int cnt = usb_control_msg(
		handle, 
		USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
		r, address, 
		0, data, length, TIMEOUT
	);
	if(cnt != length){
		usb_strerror();
		exit(1);
	}
}
static void read_main(const enum request r, long address, long length, uint8_t *data)
{
	const int packet = READ_PACKET_SIZE;
	while(length >= packet){
		device_read(handle, r, address, packet, data);
		data += packet;
		address += packet;
		length -= packet;
	}
	if(length != 0){
		device_read(handle, r, address, length, data);
	}
}
static void kazzo_cpu_read(long address, long length, uint8_t *data)
{
	read_main(REQUEST_CPU_READ, address, length, data);
//	read_main(REQUEST_CPU_READ_6502, address, length, data);
}
static void kazzo_ppu_read(long address, long length, uint8_t *data)
{
	read_main(REQUEST_PPU_READ, address, length, data);
}
//-------- write sequence --------
static void device_write(usb_dev_handle *handle, enum request w, long address, long length, const uint8_t *data)
{
	//Removing const attribute is not good method....
	int cnt = usb_control_msg(
		handle, 
		USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
		w, address, 
		0, (uint8_t *) data, length, TIMEOUT
	);
	if(cnt != length){
		usb_strerror();
		exit(1);
	}
}

static void kazzo_init(void)
{
	device_write(handle, REQUEST_PHI2_INIT, 0, 0, NULL);
}

static void kazzo_cpu_write_6502(long address, long length, const uint8_t *data)
{
	device_write(handle, REQUEST_CPU_WRITE_6502, address, length, data);
}
/*static void kazzo_cpu_write_flash(long address, long data)
{
	uint8_t d = (uint8_t) (data & 0xff);
	device_write(handle, REQUEST_CPU_WRITE_FLASH, address, 1, &d);
}*/
static void kazzo_ppu_write(long address, long length, const uint8_t *data)
{
	device_write(handle, REQUEST_PPU_WRITE, address, length, data);
}

static inline void pack_short_le(long l, uint8_t *t)
{
	t[0] = l & 0xff;
	t[1] = (l >> 8) & 0xff;
}
static void flash_config(enum request r, long c000x, long c2aaa, long c5555, long unit)
{
	const int size = 2 * 4;
	uint8_t t[size];
	assert(unit >= 1 && unit < 0x400);
	pack_short_le(c000x, t);
	pack_short_le(c2aaa, t + 2);
	pack_short_le(c5555, t + 4);
	pack_short_le(unit, t + 6);
	device_write(handle, r, 0, size, t);
}
static void kazzo_cpu_flash_config(long c000x, long c2aaa, long c5555, long unit)
{
	flash_config(REQUEST_CPU_FLASH_CONFIG_SET, c000x, c2aaa, c5555, unit);
}
static void kazzo_ppu_flash_config(long c000x, long c2aaa, long c5555, long unit)
{
	flash_config(REQUEST_PPU_FLASH_CONFIG_SET, c000x, c2aaa, c5555, unit);
}

static inline void flash_execute(enum request p, enum request s, long address, const uint8_t *data, int size, bool dowait)
{
	uint8_t status;
	device_write(handle, p, address, size, data);
	if(dowait == true){
		do{
			wait(1);
			device_read(handle, s, 0, 1, &status);
		}while(status != KAZZO_TASK_FLASH_IDLE);
	}
}
static void kazzo_cpu_flash_erase(long address, bool dowait)
{
	flash_execute(REQUEST_CPU_FLASH_ERASE, REQUEST_CPU_FLASH_STATUS, address, NULL, 0, dowait);
}
static void kazzo_ppu_flash_erase(long address, bool dowait)
{
	flash_execute(REQUEST_PPU_FLASH_ERASE, REQUEST_PPU_FLASH_STATUS, address, NULL, 0, dowait);
}

static long flash_program(enum request p, enum request s, long address, long length, const uint8_t *data, bool dowait)
{
	if(dowait == false){
		flash_execute(p, s, address, data, FLASH_PACKET_SIZE, dowait);
		return FLASH_PACKET_SIZE;
	}
	long count = 0;
	while(length >= FLASH_PACKET_SIZE){
		flash_execute(p, s, address, data, FLASH_PACKET_SIZE, dowait);
		address += FLASH_PACKET_SIZE;
		data += FLASH_PACKET_SIZE;
		count += FLASH_PACKET_SIZE;
		length -= FLASH_PACKET_SIZE;
	}
	return count;
}
static long kazzo_cpu_flash_program(long address, long length, const uint8_t *data, bool dowait)
{
	enum request p = REQUEST_CPU_FLASH_PROGRAM;
	enum request s = REQUEST_CPU_FLASH_STATUS;
	return flash_program(p, s, address, length, data, dowait);
}
static long kazzo_ppu_flash_program(long address, long length, const uint8_t *data, bool dowait)
{
	return flash_program(REQUEST_PPU_FLASH_PROGRAM, REQUEST_PPU_FLASH_STATUS, address, length, data, dowait);
}

static void kazzo_flash_status(uint8_t s[2])
{
	read_main(REQUEST_BOTH_FLASH_STATUS, 0, 2, s);
}
static void kazzo_cpu_flash_device_get(uint8_t s[2])
{
	read_main(REQUEST_CPU_FLASH_DEVICE, 0, 2, s);
}
static void kazzo_ppu_flash_device_get(uint8_t s[2])
{
	read_main(REQUEST_PPU_FLASH_DEVICE, 0, 2, s);
}
static uint8_t kazzo_vram_connection(void)
{
	uint8_t s;
	read_main(REQUEST_VRAM_CONNECTION, 0, 1, &s);
	return s;
}
const struct reader_driver DRIVER_KAZZO = {
	.name = "kazzo",
	.open_or_close = kazzo_open_close,
	.init = kazzo_init,
	.cpu_read = kazzo_cpu_read, .ppu_read = kazzo_ppu_read,
	.cpu_write_6502 = kazzo_cpu_write_6502,
	.flash_support = true,
	.ppu_write = kazzo_ppu_write,
	.cpu_flash_config = kazzo_cpu_flash_config,
	.cpu_flash_erase = kazzo_cpu_flash_erase,
	.cpu_flash_program = kazzo_cpu_flash_program,
	.cpu_flash_device_get = kazzo_cpu_flash_device_get,
	.ppu_flash_config = kazzo_ppu_flash_config,
	.ppu_flash_erase = kazzo_ppu_flash_erase,
	.ppu_flash_program = kazzo_ppu_flash_program,
	.ppu_flash_device_get = kazzo_ppu_flash_device_get,
	.flash_status = kazzo_flash_status,
	.vram_connection = kazzo_vram_connection
};
