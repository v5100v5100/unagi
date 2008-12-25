/*
famicom ROM cartridge utility - unagi
emuste.net �Ǥ��ʤ��ߤΤ�ΤΥϡ��ɥɥ饤��

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

memo:
* -O0 �ʤ� inline asm �Ǥ�ȿ���Ǥ��뤬��-O2 ����ư���ʤ�
 * ͽ�ۤ�ȿ���� out ��ư������ in �� wait ��ɬ�פߤ���
* gcc �Υ�����֥�� x86 �Ǥ����� src,dst �ν��ȿž���Ƥ���
* http://download.intel.com/jp/developer/jpdoc/IA32_Arh_Dev_Man_Vol2A_i.pdf
 * out,in �Υ��ɥ쥹�� dx ��Ȥ�ʤ��� 8bit ���ɥ쥹�ˤʤ�
 * out,in �Υǡ����ϥ쥸�����ǥǡ��������Ѥ�� al:8bit, ax:16bit, eax:32bit
*/
#include "type.h"
#include "paralellport.h"
#include "hard_onajimi.h"
#include "reader_master.h"
#include "reader_onajimi.h"

static inline void bus_control(int data)
{
#if ASM_ENABLE==0
	_outp(PORT_DATA, data);
#else
	asm(
		" movl %0,%%edx\n"
		" movl %1,%%eax\n"
		" out %%al,%%dx\n"
		:: "i"(PORT_DATA), "q"(data): "%edx", "%eax"
	);
#endif
}

/*
CONTROL bit0 STROBE
�ǡ�����ȿž����
*/
static inline void address_control(int data)
{
	data &= 0x01;
	data ^= 0x01;
#if ASM_ENABLE==0
	_outp(PORT_CONTROL, data);
#else
	asm(
		" movl %0,%%edx\n"
		" movl %1,%%eax\n"
		" out %%al,%%dx\n"
		:: "i"(PORT_CONTROL), "q"(data): "%edx", "%eax"
	);
#endif
}

/* address control data & function */
static long past_address = 0;

static void address_reset(void)
{
	address_control(ADDRESS_RESET);
	address_control(ADDRESS_ENABLE);
	past_address = 0;
}

/*
H->L ��addressincrement
*/
static inline void address_increment(int data)
{
	data = bit_set(data, BITNUM_ADDRESS_INCREMENT);
	bus_control(data);

	data = bit_clear(data, BITNUM_ADDRESS_INCREMENT);
	bus_control(data);
	past_address += 1;
	past_address &= ADDRESS_MASK_A0toA14;
}

static void address_set(long address, int control)
{
	address &= ADDRESS_MASK_A0toA14;
	long increment_count = address - past_address;
	if(increment_count < 0){
		address_reset();
		increment_count = address;
	}
		
	while(increment_count != 0){
		address_increment(control);
		increment_count -= 1;
	}
}

/*
STATUS bit7 BUSY
�ǡ�����ȿž����
*/
static inline int data_bit_get(void)
{
#if ASM_ENABLE==1
	int data;
	asm(
		" xorl %%eax,%%eax\n"
		" movl %1,%%edx\n"
		" in %%dx,%%al\n"
		" movl %%eax,%0"
		:"=q"(data) : "i"(PORT_BUSY) :"%edx", "%eax"
	);
#else
	int data = _inp(PORT_BUSY);
#endif
	data >>= 7;
	data &= 0x01;
	return data ^ 0x01;
}

/* 
L->H ��shift 
*/
static inline void data_shift(int control)
{
	control = bit_clear(control, BITNUM_DATA_SHIFT_RIGHT);
	bus_control(control);
	control = bit_set(control, BITNUM_DATA_SHIFT_RIGHT);
	bus_control(control);
}

/*�Ǿ��bit������֤ˤȤ�*/
static u8 data_get(int control)
{
	int data = 0;
	int i;
	//fcbus 8bit data load, shift count reset
	control = bit_set(control, BITNUM_DATA_DIRECTION);
	data_shift(control);
	//shift mode
	control = bit_clear(control, BITNUM_DATA_DIRECTION);
	bus_control(control);
	for(i = 0; i < 8; i++){
		data |= (data_bit_get() << i);
		data_shift(control);
	}
	return (u8) data;
}

//������ data �� 0 or 1
static inline int writedata_set(long data)
{
	data &= 1;
	return data << BITNUM_DATA_WRITE_DATA;
}

static void data_set(int control, long data)
{
	int i;
	for(i = 0; i < 8; i++){
		control = bit_clear(control, BITNUM_DATA_WRITE_DATA);
		control |= writedata_set(data >> i);
		bus_control(control);
		data_shift(control);
	}
}

static const int BUS_CONTROL_INIT = (
	ADDRESS_NOP | DATA_SHIFT_NOP |
	(DATA_DIRECTION_READ << BITNUM_DATA_DIRECTION) |
	(PPU_DISABLE << BITNUM_PPU_SELECT) |
	(PPU_WRITE__CPU_DISABLE << BITNUM_CPU_M2) |
	(CPU_RAM_SELECT << BITNUM_CPU_RAMROM_SELECT) |
	(CPU_READ << BITNUM_CPU_RW)
);
static void reader_init(void)
{
	int c = BUS_CONTROL_INIT;
	int i = 0x80;
	bus_control(c);
	address_reset();
	/*
	namcot bus �������
	driver_onajimi ����
	*/
	while(i != 0){
		c = bit_set(c, BITNUM_CPU_M2);
		bus_control(c);
		c = bit_clear(c, BITNUM_CPU_M2);
		bus_control(c);
		i--;
	}
}

static const int BUS_CONTROL_CPU_READ = (
	ADDRESS_NOP | DATA_SHIFT_NOP |
	(DATA_DIRECTION_READ << BITNUM_DATA_DIRECTION) |
	(PPU_DISABLE << BITNUM_PPU_SELECT) |
	(PPU_READ__CPU_ENABLE << BITNUM_CPU_M2) | //H
	(CPU_RAM_SELECT << BITNUM_CPU_RAMROM_SELECT) |
	(CPU_READ << BITNUM_CPU_RW)
);

static const int BUS_CONTROL_PPU_READ = (
	ADDRESS_NOP | DATA_SHIFT_NOP |
	(DATA_DIRECTION_READ << BITNUM_DATA_DIRECTION) |
	(PPU_READ__CPU_ENABLE << BITNUM_CPU_M2) |
	(PPU_ENABLE << BITNUM_PPU_SELECT) |
	(CPU_RAM_SELECT << BITNUM_CPU_RAMROM_SELECT) |
	(CPU_READ << BITNUM_CPU_RW)
);

static const int BUS_CONTROL_BUS_WRITE = (
	ADDRESS_NOP | DATA_SHIFT_NOP |
	(DATA_DIRECTION_WRITE << BITNUM_DATA_DIRECTION) |
	(PPU_READ__CPU_ENABLE << BITNUM_CPU_M2) |
	(PPU_DISABLE << BITNUM_PPU_SELECT) |
	(CPU_RAM_SELECT << BITNUM_CPU_RAMROM_SELECT) |
	(CPU_READ << BITNUM_CPU_RW)
);

static void fc_bus_read(long address, long length, u8 *data, int control, int m2_control)
{
	address_set(address, control);
	if(m2_control == M2_CONTROL_TRUE){
		control = bit_clear(control, BITNUM_CPU_M2);
		bus_control(control); //H->L: mapper �����ɥ쥹���äƤ���
	}
	while(length != 0){
		if(m2_control == M2_CONTROL_TRUE){
			//L->H: mapper �� data ��Ф�
			control = bit_set(control, BITNUM_CPU_M2);
		}
		*data = data_get(control);
		if(m2_control == M2_CONTROL_TRUE){
			//H->L: ���䤹��
			control = bit_clear(control, BITNUM_CPU_M2);
			bus_control(control);
			//L->H: increment
			control = bit_set(control, BITNUM_CPU_M2);
		}
		address_increment(control);

		if(m2_control == M2_CONTROL_TRUE){
			//H->L: mapper �����ɥ쥹���äƤ���
			control = bit_clear(control, BITNUM_CPU_M2);
			bus_control(control);
		}

		data++;
		length--;
	}
	control = bit_set(control, BITNUM_CPU_M2);
}

static void cpu_read(long address, long length, u8 *data)
{
	int control = BUS_CONTROL_CPU_READ;
	if(address & ADDRESS_MASK_A15){
		control = bit_clear(control, BITNUM_CPU_RAMROM_SELECT);
	}
	fc_bus_read(address, length, data, control, M2_CONTROL_TRUE);
}

static void ppu_read(long address, long length, u8 *data)
{
	fc_bus_read(address, length, data, BUS_CONTROL_PPU_READ, M2_CONTROL_FALSE);
}
/*
6502 write cycle
t   |01234
----+-----
��2 |HLHLH
/ROM|HHxxH
R/W |HHLLH

0 H bus:addressset
1 H->L bus:data set, mapper:address get
2 L->H bus:data write
3 H->L mapper: data write enable
4 L->H mapper: data get, bus:close

H:1, L:0, x:ROMareaaccess��0, ����ʳ�1
*/
static void cpu_6502_write(long address, long data, long wait_msec)
{
	int control = BUS_CONTROL_BUS_WRITE;
	//address���� + ���ƤΥХ���ߤ��
	address_set(address, control);

	//��2 = L, R/W=L, data set, dataout
	control = bit_clear(control, BITNUM_CPU_M2);
	data_set(control, data);
	control = bit_clear(control, BITNUM_CPU_RW);
	bus_control(control);
	if(address & ADDRESS_MASK_A15){
		control = bit_clear(control, BITNUM_CPU_RAMROM_SELECT);
	}
	wait(wait_msec);
	
	//��2 = H, data out
	control = bit_set(control, BITNUM_CPU_M2);
	bus_control(control);
	wait(wait_msec);
	//��2 = L, H �ˤ���ޤ� R/W, address, Data ��ͭ�����֤ˤ���
	control = bit_clear(control, BITNUM_CPU_M2);
	bus_control(control);
	wait(wait_msec);
	//��2 = H, R/W = H, address disable, data out disable
	bus_control(BUS_CONTROL_BUS_WRITE);
}

static void ppu_write(long address, long data)
{
	int control = BUS_CONTROL_BUS_WRITE;

	address_set(address, control);
	bus_control(control);
	data_set(control, data);
	control = bit_clear(control, BITNUM_PPU_RW);
	control = bit_clear(control, BITNUM_PPU_SELECT);
	bus_control(control);
	bus_control(BUS_CONTROL_BUS_WRITE);
}

static const int BUS_CONTROL_CPU_FLASH_WRITE = (
	ADDRESS_NOP | DATA_SHIFT_NOP |
	(DATA_DIRECTION_READ << BITNUM_DATA_DIRECTION) |
	(PPU_DISABLE << BITNUM_PPU_SELECT) |
	(PPU_READ__CPU_ENABLE << BITNUM_CPU_M2) | //H
	(CPU_RAM_SELECT << BITNUM_CPU_RAMROM_SELECT) |
	(CPU_READ << BITNUM_CPU_RW)
);

static void cpu_flash_write(long address, long data)
{
	int control = BUS_CONTROL_BUS_WRITE;
	control = bit_clear(control, BITNUM_CPU_M2);
	address_set(address, control);
	
	//WE down
	control = bit_clear(control, BITNUM_CPU_RW);
	data_set(control, data);
	//CS down
	control = bit_clear(control, BITNUM_CPU_RAMROM_SELECT);
	bus_control(control);
	//CS up
	control = bit_set(control, BITNUM_CPU_RAMROM_SELECT);
	bus_control(control);
	//WE up
	control = bit_set(control, BITNUM_CPU_RW);
	bus_control(control);
}

const struct reader_driver DRIVER_ONAJIMI = {
	.name = "onajimi",
	.open_or_close = paralellport_open_or_close,
	.init = reader_init,
	.cpu_read = cpu_read,
	.ppu_read = ppu_read,
	.cpu_6502_write = cpu_6502_write,
	.cpu_flash_write = cpu_flash_write,
	.ppu_write = ppu_write
};
