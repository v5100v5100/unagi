/*
Winbond 29C020 test
*/
struct flash_task{
	long address, data
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
	address_set(address | ADDRESS_MASK_A15);
	data_port_set(c, data);
/*
During the byte-load cycle, the addresses are latched by the falling 
edge of either CE or WE,whichever occurs last. The data are latched 
by the rising edge of either CE or WE, whicheveroccurs first.
*/
	//WE down
	c = bit_clear(c, BITNUM_CPU_RW);
//	c = bit_clear(c, BITNUM_CPU_M2);
	data_port_latch(DATA_SELECT_CONTROL, c);
	//CS down
	address_set(address & ADDRESS_MASK_A0toA14);
	data_port_latch(DATA_SELECT_CONTROL, FLASH_CPU_WRITE);
}

/*
memory detail
address 0x000000-0x03ffff
VRC6 memory bank
page0 0-
page2 $4000-$7fff
*/
static void cpu_page_address_get(long liner, long *page, long *offset)
{
	*page = liner / 0x4000;
	*offset = liner & 0x3fff
}

static void cpu_liner_write_task(long liner, long data)
{
	long page, offset;
	cpu_page_address_get(liner, &page, &offset);
	hk_cpu_write(0x8000, page); //bank0 set
	flash_cpu_write(offset, data); //bank0 write
}

static void cpu_liner_write_data(long liner, u8 *data, long length)
{
	long page, offset;
	cpu_page_address_get(liner, &page, &offset);
	hk_cpu_write(0x8000, 0); //bank0 set
	while(length != 0){
		flash_cpu_write(offset, *data);
		offset++;
		*data++;
		length--;
	}
}

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
		cpu_liner_write_task(t->addrss, t->data);
		t++;
	}
}

void flash_write(const u8 *data, long length)
{
	task_set(PROTECT_DISABLE);
	pause(10);
	/*
	sequentially load up to 128 byte of page data
	*/
	long address = 0x40000;
	while(length != 0){
		const int banksize = 0x4000;
		cpu_liner_write_data(address, data, banksize);
		address += banksize;
		data += banksize;
		length -= banksize;
	}
	/*
	pause 10ms
	*/
	task_set(PROTECT_ENABLE);
	pause(10);
}


