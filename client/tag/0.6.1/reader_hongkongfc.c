/*
famicom ROM cartridge utility - unagi
hongkong FC driver

Copyright (C) 2008 ����ȯ��Ʊ�ȹ�

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

������������:
  * 74138 �� channel select �������ѥ���ݡ��ȤΥǡ������������ѹ�����ȡ�����Υ����ͥ�Υǡ������˲������
    * �˲�����뤬���˲��褬����Ǥ���餷������ˤ���
*/
#include "type.h"
#include "paralellport.h"
#include "reader_master.h"
#include "reader_hongkongfc.h"
#include "hard_hongkongfc.h"

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

static inline void data_port_latch(int select, long data)
{
	select <<= BITNUM_CONTROL_DATA_SELECT;
	select = bit_set(select, BITNUM_CONTROL_DATA_LATCH);
	port_control_write(select, PORT_CONTROL_WRITE);
	_outp(PORT_DATA, data);
	select = bit_clear(select, BITNUM_CONTROL_DATA_LATCH);
	port_control_write(select, PORT_CONTROL_WRITE);
}

enum{ADDRESS_CPU_OPEN, ADDRESS_CPU_CLOSE};
static inline void address_set(long address)
{
	long address_h = address >> 8;
	long address_l = address & 0xff;
	data_port_latch(DATA_SELECT_A15toA8, address_h);
	data_port_latch(DATA_SELECT_A7toA0, address_l);
}

static inline u8 data_port_get(long address, int bus)
{
	address_set(address);
	if(bus != 0){
		data_port_latch(DATA_SELECT_CONTROL, bus);
	}
	port_control_write(DATA_SELECT_BREAK_DATA << BITNUM_CONTROL_DATA_SELECT, PORT_CONTROL_WRITE);
	int s = DATA_SELECT_READ << BITNUM_CONTROL_DATA_SELECT;
	s = bit_set(s, BITNUM_CONTROL_DATA_LATCH);
	port_control_write(s, PORT_CONTROL_READ);
	return (u8) _inp(PORT_DATA);
}

/*
���� A23-A16 port �� latch ���������Ȥˡ�
fc adapater �����ˤ���� latch �����롣
*/
static void data_port_set(int c, long data)
{
	data_port_latch(DATA_SELECT_WRITEDATA, data);
	data_port_latch(DATA_SELECT_CONTROL, c);
	c = bit_set(c, BITNUM_WRITEDATA_LATCH);
	data_port_latch(DATA_SELECT_CONTROL, c);
}

// /A15 ��������Ѥ���
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
//static const int BUS_CONTROL_BUS_STANDBY = BUS_CONTROL_CPU_READ; //���顼�ˤʤ�
#define BUS_CONTROL_BUS_STANDBY BUS_CONTROL_CPU_READ
/*
namcot �� SRAM �ե����ȥ�å��Ϧ�2��0x80��夲��Ƚ��Ϥ����ꤹ��
RAM �����ץ���Ʊ��??
�԰������ CPU �� PPU �� data �����礵��Ƥ���ʪ���ǤƤ��뵤������
*/
static void hk_init(void)
{
	int c = BUS_CONTROL_CPU_READ;
	int i = 0x80;
	while(i != 0){
		c = bit_set(c, BITNUM_CPU_M2);
		data_port_latch(DATA_SELECT_CONTROL, c);
		c = bit_clear(c, BITNUM_CPU_M2);
		data_port_latch(DATA_SELECT_CONTROL, c);
		i--;
	}
}

static void hk_cpu_read(long address, long length, u8 *data)
{
	//fc bus �����
	data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_CPU_READ);
	//A15 ��ȿž���� /ROMCS �ˤ�����Τ��Ϥ�
	address ^= ADDRESS_MASK_A15;
	while(length != 0){
		*data = data_port_get(address, 0);
		address++;
		data++;
		length--;
	}
}

//patch. charcter �ν񤭹��ߤΰ��꤬�������줿��������
int hongkong_flash_patch = 0;
static void hk_ppu_read(long address, long length, u8 *data)
{
	if(hongkong_flash_patch == 0){
		data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_BUS_STANDBY);
	}else{
		data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_PPU_READ);
	}
	
	address &= ADDRESS_MASK_A0toA12; //PPU charcter data area mask
	address |= ADDRESS_MASK_A15; //CPU area disk
	while(length != 0){
		if(hongkong_flash_patch == 0){
			*data = data_port_get(address, BUS_CONTROL_PPU_READ);
		}else{
			*data = data_port_get(address, 0); 
		}
		address++;
		data++;
		length--;
		if(hongkong_flash_patch == 0){
			data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_BUS_STANDBY);
		}
	}
}

static inline void cpu_romcs_set(long address)
{
	data_port_latch(DATA_SELECT_A15toA8, address >> 8);
}

static void hk_cpu_write_6502(long address, long length, const uint8_t *data)
{
	while(length != 0){
		int c = BUS_CONTROL_BUS_STANDBY;
		//���ƤΥХ���ߤ��
		data_port_latch(DATA_SELECT_CONTROL, c);
		// /rom �� H �ˤ��ƥХ���ߤ��
		address_set(address | ADDRESS_MASK_A15);
		
		//��2 = L, R/W=L, address set, data set
		c = bit_clear(c, BITNUM_CPU_M2);
		data_port_set(c, *data); //latch�Ϥ��δؿ������ǹԤ�
		if(address & ADDRESS_MASK_A15){
			cpu_romcs_set(address & ADDRESS_MASK_A0toA14);
		}
		c = bit_clear(c, BITNUM_CPU_RW);
		data_port_latch(DATA_SELECT_CONTROL, c);
		//wait(wait_msec);
		//��2 = H, data out
		c = bit_set(c, BITNUM_CPU_M2);
		c = bit_clear(c, BITNUM_WRITEDATA_OUTPUT);
		data_port_latch(DATA_SELECT_CONTROL, c);
		//wait(wait_msec);
		//��2 = L, H �ˤ���ޤ� R/W, address, Data ��ͭ�����֤ˤ���
		c = bit_clear(c, BITNUM_CPU_M2);
		data_port_latch(DATA_SELECT_CONTROL, c);
		//wait(wait_msec);
		//��2 = H, R/W = H, address disable, data out disable
		if(address & ADDRESS_MASK_A15){
			//address & ADDRESS_MASK_A15 ��ư���Ƥ�..?
			cpu_romcs_set(address | ADDRESS_MASK_A15);
		}
		data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_BUS_STANDBY);
		address += 1;
		data += 1;
		length -= 1;
	}
}

//onajimi ���� /CS �� /OE ��Ʊ���ˤʤäƤ��뤬��hongkong���Ȼߤ���롣�񤭹��߻��� output enable �� H �Ǥ���٤���
static void hk_ppu_write(long address, long length, const uint8_t *data)
{
	while(address != 0){
		int c = BUS_CONTROL_BUS_STANDBY;
		c = bit_clear(c, BITNUM_CPU_M2); //���֤󤤤�
		//c = bit_clear(c, BITNUM_CPU_RW);
		data_port_latch(DATA_SELECT_CONTROL, c);
		//cpu rom ��ߤ᤿���ɥ쥹���Ϥ�
		address_set((address & ADDRESS_MASK_A0toA12) | ADDRESS_MASK_A15);
		data_port_set(c, *data); 
	//	data_port_latch(DATA_SELECT_CONTROL, c);
		//CS down
		c = bit_clear(c, BITNUM_PPU_SELECT);
		data_port_latch(DATA_SELECT_CONTROL, c);
		//WE down
		c = bit_clear(c, BITNUM_WRITEDATA_OUTPUT);
		c = bit_clear(c, BITNUM_PPU_RW);
		data_port_latch(DATA_SELECT_CONTROL, c);
		//WE up
		c = bit_set(c, BITNUM_PPU_RW);
		data_port_latch(DATA_SELECT_CONTROL, c);
		//CS up
	//	c = bit_set(c, BITNUM_WRITEDATA_OUTPUT);
		data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_BUS_STANDBY);
		
		address += 1;
		data += 1;
		length -= 1;
	}
}

#if 0
static const int FLASH_CPU_WRITE = (
	(1 << BITNUM_PPU_OUTPUT) |
	(1 << BITNUM_PPU_RW) |
	(1 << BITNUM_PPU_SELECT) |
	(1 << BITNUM_WRITEDATA_OUTPUT) |
	(0 << BITNUM_WRITEDATA_LATCH) |
	(0 << BITNUM_CPU_M2) |
	(1 << BITNUM_CPU_RW)
);

static void hk_cpu_flash_write(long address, long data)
{
	int c = FLASH_CPU_WRITE;
	//���ƤΥХ���ߤ��
	data_port_latch(DATA_SELECT_CONTROL, c);
	address_set(address | ADDRESS_MASK_A15);
	data_port_set(c, data);
/*
W29C020
During the byte-load cycle, the addresses are latched by the falling 
edge of either CE or WE,whichever occurs last. The data are latched 
by the rising edge of either CE or WE, whicheveroccurs first.
W49F002
#CS or #WE ���ߤꤿ�Ȥ��� address latch
#CS or #WE ���夬�ä��Ȥ��� data latch

hongkong �Ϥ� address �� /ROMCS ��Ʊ���Х��Ȥǡ� /CS ����ˤ����
hongkong �ǡ����˲�+���ɥ쥹�԰���ˤʤ�Τǡ�/WE ����ˤ��ʤ���ư���ʤ���
*/
	//CS down
	cpu_romcs_set(address & ADDRESS_MASK_A0toA14);
	//WE down
	c = bit_clear(c, BITNUM_WRITEDATA_OUTPUT);
	c = bit_clear(c, BITNUM_CPU_RW);
//	c = bit_clear(c, BITNUM_CPU_M2);
	data_port_latch(DATA_SELECT_CONTROL, c);
	//WE up
	data_port_latch(DATA_SELECT_CONTROL, FLASH_CPU_WRITE);
	//CS up
	cpu_romcs_set(address | ADDRESS_MASK_A15);
}
#endif

const struct reader_driver DRIVER_HONGKONGFC = {
	.name = "hongkongfc",
	.open_or_close = paralellport_open_or_close,
	.init = hk_init,
	.cpu_read = hk_cpu_read,
	.ppu_read = hk_ppu_read,
	.cpu_write_6502 = hk_cpu_write_6502,
	.ppu_write = hk_ppu_write,
	.flash_support = NG,
	.cpu_flash_config = NULL, .cpu_flash_erase = NULL, .cpu_flash_program = NULL,
	.ppu_flash_config = NULL, .ppu_flash_erase = NULL, .ppu_flash_program = NULL
};

#ifdef TEST
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
		data_port_latch(DATA_SELECT_CONTROL, BUS_CONTROL_BUS_STANDBY);
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
		address_set(d | ADDRESS_MASK_A15, ADDRESS_RESET);
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
#endif