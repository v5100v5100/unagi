/*
famicom ROM cartridge dump program - unagi
emuste.net でおなじみのもののハードドライバ

todo: 
* battery backup RAM を読めるようにする(namco109 は読めているのか電池が切れてるのかよくわからない、その他は全滅)
* _outp(), _inp() を明確に使えるようにするか、インラインアセンブラにする
* 別のハードウェアに対応した場合は PORT_DATA から wait までをヘッダにまとめる
*/
//#include <dos.h> ?
//#include <windows.h>
#include "type.h"
#include "kairo.h"
#include "paralellport.h"

enum{
	PORT_DATA = 0x0378,
	PORT_STATUS,
	PORT_CONTROL
};
enum{
	ADDRESS_MASK_A0toA14 = 0x7fff,
	ADDRESS_MASK_A15 = 0x8000
};
//mingw だと conio.h に _outp, _inp がないけど、リンクできる...
//自前プロトタイプでいいのか?
void _outp(int, int);
int _inp(int);

static inline int bit_set(int data, const int bit)
{
	data |= 1 << bit;
	return data;
}

static inline int bit_clear(int data, const int bit)
{
	data &= ~(1 << bit);
	return data;
}

static inline void wait(void)
{
	//const long waittime = 100000;
	//SleepEx(20,TRUE);
}

static inline void bus_control(int data)
{
	_outp(PORT_DATA, data);
	wait();
}

/*
CONTROL bit0 STROBE
データが反転する
*/
static inline void address_control(int data)
{
	data &= 0x01;
	data ^= 0x01;
	_outp(PORT_CONTROL, data);
	wait();
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
H->L でaddressincrement
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
データが反転する
*/
static inline int data_bit_get(void)
{
	int data = _inp(PORT_STATUS);
	data >>= 7;
	data &= 0x01;
	return data ^ 0x01;
}

/* 
L->H でshift 
*/
static void data_shift(int control)
{
	control = bit_clear(control, BITNUM_DATA_SHIFT_RIGHT);
	bus_control(control);
	control = bit_set(control, BITNUM_DATA_SHIFT_RIGHT);
	bus_control(control);
}

/*最上位bitから順番にとる*/
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

//ここの data は 0 or 1
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
void reader_init(void)
{
	bus_control(BUS_CONTROL_INIT);
	address_reset();
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

#define M2_SET_READ (0)
static void fc_bus_read(long address, long length, u8 *data, int control)
{
	address_set(address, control);
	bus_control(control);
	if(M2_SET_READ == 1){
		control = bit_clear(control, BITNUM_CPU_M2);
		bus_control(control); //H->L: mapper がアドレスを取ってくる
	}
	while(length != 0){
		if(M2_SET_READ == 1){
			//L->H: mapper が data を出す
			control = bit_set(control, BITNUM_CPU_M2);
		}
		*data = data_get(control);
		if(M2_SET_READ == 1){
			//H->L: おやすみ
			control = bit_clear(control, BITNUM_CPU_M2);
			bus_control(control);
			//L->H: increment
			control = bit_set(control, BITNUM_CPU_M2);
		}
		address_increment(control);

		if(M2_SET_READ == 1){
			//H->L: mapper がアドレスを取ってくる
			control = bit_clear(control, BITNUM_CPU_M2);
			bus_control(control);
		}

		data++;
		length--;
	}
	control = bit_set(control, BITNUM_CPU_M2);
}

void cpu_read(long address, long length, u8 *data)
{
	int control = BUS_CONTROL_CPU_READ;
	if(address & ADDRESS_MASK_A15){
		control = bit_clear(control, BITNUM_CPU_RAMROM_SELECT);
	}
	fc_bus_read(address, length, data, control);
}

void ppu_read(long address, long length, u8 *data)
{
	fc_bus_read(address, length, data, BUS_CONTROL_PPU_READ);
}
/*
6502 write cycle
t   |01234
----+-----
φ2 |HLHLH
/ROM|HHxxH
R/W |HHLLH

0 H bus:addressset
1 H->L bus:data set, mapper:address get
2 L->H bus:data write
3 H->L mapper: data write enable
4 L->H mapper: data get, bus:close

H:1, L:0, x:ROMareaaccess時0, それ以外1
*/
void cpu_write(long address, long data)
{
	int control = BUS_CONTROL_BUS_WRITE;
	//address設定 + 全てのバスを止める
	address_set(address, control);

	control = bit_clear(control, BITNUM_CPU_M2);
	//printf("%02x ", (int) data);
	data_set(control, data);

	control = bit_clear(control, BITNUM_CPU_RW);
	//bus_control(control); //
	control = bit_set(control, BITNUM_CPU_M2);
	//bus_control(control); //
	if(address & ADDRESS_MASK_A15){
		control = bit_clear(control, BITNUM_CPU_RAMROM_SELECT);
	}
	bus_control(control);
	control = bit_clear(control, BITNUM_CPU_M2);
	bus_control(control);
	bus_control(BUS_CONTROL_BUS_WRITE);
}

void ppu_write(long address, long data)
{
	int control = BUS_CONTROL_BUS_WRITE;

	address_set(address, control);
	bus_control(control);
	data_set(control, data);
	control = bit_clear(control, BITNUM_PPU_RW);
	control = bit_clear(control, BITNUM_PPU_SELECT); //mmc1 はなくてもいけるが mmc3 には必要
	bus_control(control);
	bus_control(BUS_CONTROL_BUS_WRITE);
}
