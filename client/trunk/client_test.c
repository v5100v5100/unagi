#include <stdio.h>
#include <stdlib.h>
#include "type.h"
#include "reader_master.h"
#include "flashmemory.h"
#include "giveio.h"
#include "file.h"
#include "client_test.h"

static void read_save(const struct reader_driver *d, const char *file, long length)
{
	u8 *data;
	data = malloc(length);
	d->cpu_read(0x8000, length, data);
	buf_save(data, file, length);
	free(data);
}

void test(const char *drivername, const char *file)
{
	const struct reader_driver *d;
	d = reader_driver_get(drivername);
	if(d == NULL){
		printf("%s: reader driver not found\n", __FUNCTION__);
		return;
	}
	const struct flash_driver *f;
	f = flash_driver_get("W29C020");
	if(f == NULL){
		printf("%s: flash driver not found\n", __FUNCTION__);
		return;
	}
	
	const int gg = giveio_start();
	switch(gg){
	case GIVEIO_OPEN:
	case GIVEIO_START:
	case GIVEIO_WIN95:
		d->init();
		break;
	default:
	case GIVEIO_ERROR:
		printf("Can't Access Direct IO %d\n", gg);
		return;
	}

	if(0){
		read_save(d, "winit.bin", 0x4000);
	}
	//CPU bank init
	d->cpu_6502_write(0x8000, 0);
	d->cpu_6502_write(0xc000, 2);
	//PPU bank init
	d->cpu_6502_write(0xb003, 0x20);
	d->cpu_6502_write(0xd000, 0x0a);
	d->cpu_6502_write(0xd002, 0x15);
	d->cpu_6502_write(0xd001, 0x0a);
	d->cpu_6502_write(0xd003, 0x15);
	d->cpu_6502_write(0xe000, 0x00);
	d->cpu_6502_write(0xe002, 0x01);
	d->cpu_6502_write(0xe001, 0x02);
	d->cpu_6502_write(0xe003, 0x03);

	switch(file[0]){
	case 'b':{
		const int testbufsize = 0x100;
		u8 testbuf[testbufsize];
		int i;
		d->cpu_read(0x6000, testbufsize, testbuf);
		for(i=0;i<0x10;i++){
			printf("%02x ", testbuf[i]);
		}
		}break;
	case 'e': case 'f': case 'p':{
		struct flash_order order = {
			address: 0x8000,
			length: 0x4000,
			data: NULL,
			flash_write: d->cpu_flash_write,
			read: d->cpu_read,
			task_0000: 0,
			task_2aaa: (0x2aaa & 0x3fff) + 0x8000,
			task_5555: (0x5555 & 0x1fff) + 0xc000
		};
		switch(file[0]){
		case 'f':{
			int size;
			order.data = buf_load_full(file, &size);
			if(order.data == NULL){
				break;
			}
			f->write(&order);
			free((void *) order.data);
			}break;
		case 'e':
			f->erase(&order);
			break;
		case 'p':
			if(f->productid_check(&order, f) == NG){
				printf("product id error\n");
			}
			break;
		}
		}break;
	case 'E': case 'P':  {
		struct flash_order order = {
			address: 0,
			length: 0x400,
			data: NULL,
			flash_write: d->ppu_write,
			read: d->ppu_read,
			task_0000: 0,
			task_2aaa: (0x2aaa & 0x03ff) + 0,
			task_5555: (0x5555 & 0x03ff) + 0x400
		};
		switch(file[0]){
		case 'E':
			f->erase(&order);
			break;
		case 'P':
			if(f->productid_check(&order, f) == NG){
				printf("product id error\n");
			}
			break;
		}
		}break;
	}
	if(gg != GIVEIO_WIN95){
		giveio_stop(GIVEIO_STOP);
	}
	return;
}

