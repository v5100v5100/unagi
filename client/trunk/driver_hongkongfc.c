#include "type.h"
#include "paralellport.h"
#include "hard_hongkongfc.h"
//#include "hongkongfc.h"

enum{
	PORT_CONTROL_MASK_XOR = 0x03, //bit01 invert
	PORT_CONTROL_READ = 0,
	PORT_CONTROL_WRITE
};

static inline int busy_get(void)
{
	int d = _inp(PORT_BUSY);
	d >>= 7;
	return d ^ 1;
}

static inline void port_control_write(int s, int rw)
{
	if(rw == PORT_CONTROL_READ){ 
		//H:BUS->PC
		s = bit_set(s, BITNUM_CONTROL_DIRECTION);
	}else{
		//L:PC->BUS
		s = bit_clear(s, BITNUM_CONTROL_DIRECTION);
	}
	s = bit_clear(s, BITNUM_CONTROL_INTERRUPT);
	s ^= PORT_CONTROL_MASK_XOR;
	_outp(PORT_CONTROL, s);
}

static void data_port_latch(int select, long data)
{
	select <<= BITNUM_CONTROL_DATA_SELECT;
	select = bit_set(select, BITNUM_CONTROL_DATA_LATCH);
	port_control_write(select, PORT_CONTROL_WRITE);
	_outp(PORT_DATA, data);
	select = bit_clear(select, BITNUM_CONTROL_DATA_LATCH);
	port_control_write(select, PORT_CONTROL_WRITE);
}

static void address_set(long address)
{
	data_port_latch(DATA_SELECT_A15toA8, address >> 8);
	data_port_latch(DATA_SELECT_A7toA0, address & 0xff);
}

static u8 data_port_get(long address)
{
	int s = DATA_SELECT_READ << BITNUM_CONTROL_DATA_SELECT;
	s = bit_set(s, BITNUM_CONTROL_DATA_LATCH);
	address_set(address);
	port_control_write(DATA_SELECT_BREAK_DATA << BITNUM_CONTROL_DATA_SELECT, PORT_CONTROL_WRITE);
	port_control_write(s, PORT_CONTROL_READ);
	return (u8) _inp(PORT_DATA);
}

/*
本体 A23-A16 port に latch させたあとに、
fc adapater 内部にさらに latch させる。
*/
static void data_port_set(int c, long data)
{
	data_port_latch(DATA_SELECT_WRITEDATA, data);
	data_port_latch(DATA_SELECT_CONTROL, c);
	c = bit_clear(c, BITNUM_WRITEDATA_LATCH);
	data_port_latch(DATA_SELECT_CONTROL, c);
}

// /A15 だけを使用する
static const int BUS_CONTROL_CPU_READ = (
	(1 << BITNUM_PPU_OUTPUT) |
	(1 << BITNUM_PPU_RW) |
	(1 << BITNUM_PPU_SELECT) |
	(1 << BITNUM_WRITEDATA_OUTPUT) |
	(0 << BITNUM_WRITEDATA_LATCH) |
	(1 << BITNUM_CPU_M2) |
	(1 << BITNUM_CPU_RW)
);
static const int BUS_CONTROL_PPU_READ = (
	(0 << BITNUM_PPU_OUTPUT) |
	(1 << BITNUM_PPU_RW) |
	(0 << BITNUM_PPU_SELECT) |
	(1 << BITNUM_WRITEDATA_OUTPUT) |
	(0 << BITNUM_WRITEDATA_LATCH) |
	(1 << BITNUM_CPU_M2) |
	(1 << BITNUM_CPU_RW)
);
static const int BUS_CONTROL_BUS_WRITE = (
	(1 << BITNUM_PPU_OUTPUT) |
	(1 << BITNUM_PPU_RW) |
	(1 << BITNUM_PPU_SELECT) |
	(1 << BITNUM_WRITEDATA_OUTPUT) |
	(0 << BITNUM_WRITEDATA_LATCH) |
	(1 << BITNUM_CPU_M2) |
	(1 << BITNUM_CPU_RW)
);

void hk_cpu_read(long address, long length, u8 *data)
{
	//fc bus 初期化
	data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_CPU_READ);
	//A15 を反転し、 /ROMCS にしたものを渡す
	address ^= ADDRESS_MASK_A15;
	while(length != 0){
		*data = data_port_get(address);
		address++;
		data++;
		length--;
	}
}

void hk_ppu_read(long address, long length, u8 *data)
{
	data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_PPU_READ);
	address &= ADDRESS_MASK_A0toA12; //PPU charcter data area mask
	address |= ADDRESS_MASK_A15; //CPU area disk
	while(length != 0){
		*data = data_port_get(address);
		address++;
		data++;
		length--;
	}
}

void hk_cpu_write(long address, long data)
{
	int c = BUS_CONTROL_BUS_WRITE;
	//全てのバスを止める
	data_port_latch(DATA_SELECT_CONTROL, c);
	// /rom を H にしてバスを止める
	address_set(address | ADDRESS_MASK_A15);
	
	//1 H->L bus:data set, mapper:address get
	c = bit_clear(c, BITNUM_CPU_M2);
	data_port_set(c, data); //latchはこの関数内部で行う
	c = bit_clear(c, BITNUM_WRITEDATA_OUTPUT);
	data_port_latch(DATA_SELECT_CONTROL, c);
	//2 L->H bus:data write
	//ROM 領域の場合はこのタイミングで /rom を落とす
	if(address & ADDRESS_MASK_A15){
		address_set(address & ADDRESS_MASK_A15);
	}
	c = bit_clear(c, BITNUM_CPU_RW);
	c = bit_set(c, BITNUM_CPU_M2);
	data_port_latch(DATA_SELECT_CONTROL, c);
	//3 H->L mapper: data write enable
	c = bit_clear(c, BITNUM_CPU_M2);
	data_port_latch(DATA_SELECT_CONTROL, c);
	//4 L->H mapper: data get, bus:close
	if(address & ADDRESS_MASK_A15){
		address_set(address | ADDRESS_MASK_A15);
	}
	data_port_latch(DATA_SELECT_CONTROL, BITNUM_WRITEDATA_OUTPUT);
}

void hk_ppu_write(long address, long data)
{
	int c = BUS_CONTROL_BUS_WRITE;
	c = bit_clear(c, BITNUM_CPU_M2); //たぶんいる
	data_port_latch(DATA_SELECT_CONTROL, c);
	//cpu rom を止めたアドレスを渡す
	address_set((address & ADDRESS_MASK_A0toA12) | ADDRESS_MASK_A15);
	data_port_set(c, data); 
	c = bit_clear(c, BITNUM_PPU_RW);
	c = bit_clear(c, BITNUM_PPU_SELECT);
	c = bit_clear(c, BITNUM_WRITEDATA_OUTPUT);
	data_port_latch(DATA_SELECT_CONTROL, c);
	data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_BUS_WRITE);
}

#include "giveio.h"
#include <stdio.h>
#include <stdlib.h>
static void data_show(u8 *d, int length)
{
	int sum = 0;
	int offset = 0;
	while(length != 0){
		char safix;
		switch(offset & 0xf){
		default:
			safix = ' ';
			break;
		case 7:
			safix = '-';
			break;
		case 0xf:
			safix = '\n';
			break;
		}
		printf("%02x%c", *d, safix);
		sum += *d;
		d++;
		offset++;
		length--;
	}
	printf("sum 0x%06x", sum);
}

int main(int c, char **v)
{
//	giveio_stop(GIVEIO_REMOVE);
	const int gg = giveio_start();
	switch(gg){
	case GIVEIO_OPEN:
	case GIVEIO_START:
	case GIVEIO_WIN95:
		break;
	default:
	case GIVEIO_ERROR:
		printf("Can't Access Direct IO %d\n", gg);
		return 0;
	}

	long d = strtol(v[2], NULL, 0x10);

	switch(v[1][0]){
	case 'w':
		data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_BUS_WRITE);
		port_control_write(DATA_SELECT_WRITE << BITNUM_CONTROL_DATA_SELECT, PORT_CONTROL_WRITE);
		_outp(PORT_DATA, d);
		break;
	case 'c':
		data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_CPU_READ);
		address_set(d ^ ADDRESS_MASK_A15);
		port_control_write(DATA_SELECT_BREAK_DATA << BITNUM_CONTROL_DATA_SELECT, PORT_CONTROL_WRITE);
		port_control_write(DATA_SELECT_READ << BITNUM_CONTROL_DATA_SELECT, PORT_CONTROL_READ);
		printf("%02x\n", _inp(PORT_DATA));
		break;
	case 'C':{
		const long length = 0x1000;
		u8 data[length];
		hk_cpu_read(d, length, data);
		data_show(data, length);
		}
		break;
	case 'p':
		data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_PPU_READ);
		address_set(d | ADDRESS_MASK_A15);
		port_control_write(DATA_SELECT_READ << BITNUM_CONTROL_DATA_SELECT, PORT_CONTROL_READ);
		printf("%02x\n", _inp(PORT_DATA));
		break;
	case 'P':{
		const long length = 0x200;
		u8 data[length];
		hk_ppu_read(d, length, data);
		data_show(data, length);
		}
		break;
	}
	
	if(gg != GIVEIO_WIN95){
		giveio_stop(GIVEIO_STOP);
	}
	return 0;
}
