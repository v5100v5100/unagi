#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
/*
Winbond W29C020, W49F002 test
*/
struct flash_task{
	long address, data;
};
enum{
	flash_task_end = 0x46494649
};

static const int FLASH_CPU_WRITE = (
	(1 << BITNUM_PPU_OUTPUT) |
	(1 << BITNUM_PPU_RW) |
	(1 << BITNUM_PPU_SELECT) |
	(1 << BITNUM_WRITEDATA_OUTPUT) |
	(0 << BITNUM_WRITEDATA_LATCH) |
	(0 << BITNUM_CPU_M2) |
	(1 << BITNUM_CPU_RW)
);

static void flash_cpu_write(long address, long data)
{
	int c = FLASH_CPU_WRITE;
	//全てのバスを止める
	data_port_latch(DATA_SELECT_CONTROL, c);
	address_set(address | ADDRESS_MASK_A15, ADDRESS_SET);
	data_port_set(c, data);
/*
W29C020
During the byte-load cycle, the addresses are latched by the falling 
edge of either CE or WE,whichever occurs last. The data are latched 
by the rising edge of either CE or WE, whicheveroccurs first.
*/
/*
W49F002
#CS or #WE が降りたときに address latch
#CS or #WE が上がったときに data latch
*/
	c = bit_clear(c, BITNUM_WRITEDATA_OUTPUT);
	//WE down
	c = bit_clear(c, BITNUM_CPU_RW);
//	c = bit_clear(c, BITNUM_CPU_M2);
	data_port_latch(DATA_SELECT_CONTROL, c);
	//CS down
	address_set(address & ADDRESS_MASK_A0toA14, ADDRESS_SET);
	//CS up
	address_set(address | ADDRESS_MASK_A15, ADDRESS_SET);
	//WE up
	data_port_latch(DATA_SELECT_CONTROL, FLASH_CPU_WRITE);
}

/*
memory detail
address 0x000000-0x03ffff
VRC6 memory bank
bank0 page0, liner 0x00000-0x03fff, cpu address $8000-$bfff
bank1 page2, liner 0x04000-0x05fff, cpu address $c000-$dfff
fix   liner 0x3e000-0x3ffff, cpu address $e000-$ffff
*/
static void cpu_page_address_get(long liner, long *page, long *offset)
{
	*page = liner / 0x4000;
	*offset = liner; // & 0x3fff;
}

static void cpu_liner_write_task(long liner, long data)
{
	long page, offset;
	cpu_page_address_get(liner, &page, &offset);
	flash_cpu_write(offset, data); //bank0 write
}

/*
PROTECT_ENABLE は W29C020 専用
*/
const struct flash_task PROTECT_DISABLE[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0xa0},
	{flash_task_end, 0}
};
const struct flash_task PROTECT_ENABLE[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x80},
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x20},
	{flash_task_end, 0}
};
const struct flash_task ERASE[] = {
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x80},
	{0x5555, 0xaa},
	{0x2aaa, 0x55},
	{0x5555, 0x10},
	{flash_task_end, 0}
};

static void task_set(const struct flash_task *t)
{
	while(t->address != flash_task_end){
		cpu_liner_write_task(t->address, t->data);
		t++;
	}
}

enum{WAIT_COUNT = 0x100};
static void cpu_liner_write_data(long liner, const u8 *data, long length)
{
	long page, offset;
	
	cpu_page_address_get(liner, &page, &offset);
	while(length != 0){
		if(*data != 0xff){
			task_set(PROTECT_DISABLE);
			flash_cpu_write(offset, *data);
		}
		u8 predata = *data & 0x40;
		while(1){
			u8 dummy;
			hk_cpu_read(0x8000 + offset, 1, &dummy);
			dummy &= 0x40;
			if(dummy != predata){
				predata = dummy;
				continue;
			}
			printf("offset %06x ok\n", (int) offset);
			fflush(stdout);
			break;
		}
		offset++;
		data++;
		length--;
	}
}

static void compare(const u8 *data, long length, long offset)
{
	u8 *romdata, *r;
	romdata = malloc(length);
	hk_cpu_read(0x8000, length, romdata);
	r = romdata;
	while(length != 0){
		if(*r != *data){
			printf("%06x\n", (int)offset);
		}
		r++;
		data++;
		offset++;
		length--;
	}
	free(romdata);
}

void flash_write(const u8 *data, long length, long banksize)
{
	//pause(10);
	/*
	W29C020
	sequentially load up to 128 byte of page data
	*/
	hk_cpu_write(0x8000, 0); //bank0 set
	hk_cpu_write(0xc000, 2); //bank1 set
	task_set(ERASE);
	Sleep(200); //Tec 0.2 sec
	long address = 0;
	while(length != 0){
		cpu_liner_write_data(address, data, banksize);
		compare(data, banksize, address);
		address += banksize;
		data += banksize;
		length -= banksize;
	}
	/*
	pause 10ms
	*/
	/*task_set(PROTECT_ENABLE);
	pause(10);*/
}


