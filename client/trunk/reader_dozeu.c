#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include "type.h"
#include "reader_master.h"
#include "dozeu/cusb.h"
#include "dozeu/fx2fw.h"
#include "dozeu/fx2fw_prog.h"
#include "waveform_dozeu.h"
#include "hard_dozeu.h"
#include "reader_dozeu.h"

static HANDLE handle_usb;

static int usb_open_or_close(int oc)
{
	int ret;
	handle_usb = NULL;
	switch(oc){
	case READER_OPEN:
		ret = cusb_init(
			0, &handle_usb,
			fw_bin, 
			"F2FW", "V100"
		);
		break;
	case READER_CLOSE:
		ret = usb_close(&handle_usb);
		break;
	default:
		ret = NG;
		assert(0);
		break;
	}
	return ret;
}

static const u8 DIRECTION_PORT_A = (
	(DIRECTION_OUTPUT << 0) |
	(DIRECTION_OUTPUT << 1) |
	(DIRECTION_INPUT << 2) |
	(DIRECTION_INPUT << 3) |
	(DIRECTION_INPUT << 4) |
	(DIRECTION_OUTPUT << 5) |
	(DIRECTION_OUTPUT << 6) |
	(DIRECTION_OUTPUT << 7)
);
//Port B は databus なので最初に初期化せず、都度方向を決める
static const u8 DIRECTION_PORT_C = DIRECTION_OUTPUT_8BIT;
static const u8 DIRECTION_PORT_D = DIRECTION_OUTPUT_8BIT;
static const u8 DIRECTION_PORT_E = DIRECTION_OUTPUT_8BIT;

static void dz_init(void)
{
	u8 cmd[0x40];
	int i = 0;
	cmd[i++] = CMD_MODE;
	cmd[i++] = MODE_PIO;
	cmd[i++] = CMD_OEA;
	cmd[i++] = DIRECTION_PORT_A;
	cmd[i++] = CMD_OEC;
	cmd[i++] = DIRECTION_PORT_C;
	cmd[i++] = CMD_OED;
	cmd[i++] = DIRECTION_PORT_D;
	usb_bulk_write(&handle_usb, CPIPE, cmd, i);
	
	//φ2の上げ下げって CTL 端子だけどどーやるんだ??

	i = 0;
	cmd[i++] = CMD_MODE;
	cmd[i++] = MODE_GPIF | MODE_8BIT | MODE_ADDR | MODE_NOFLOW;
	cmd[i++] = CMD_GPIF;
	memcpy(cmd + i, WAVEFORM_INIT, INIT_SIZE);
	i += INIT_SIZE;
	assert(i < 0x40);
	usb_bulk_write(&handle_usb, CPIPE, cmd, i);
}

static void address_set(long address)
{
	u8 cmd[0x40];
	int i = 0;
	cmd[i++] = CMD_ADSET;
	cmd[i++] = address & 0xff;
	cmd[i++] = 0; //上位アドレスは GPIF 制御しないのでとりあえず0
	cmd[i++] = CMD_OUTE;
	cmd[i++] = address >> 8;
	assert(i < 0x40);
	usb_bulk_write(&handle_usb, CPIPE, cmd, i);
}

static void waveform_set(int num)
{
	static int waveform_bank = WAVEFORM_BANK_UNDEF;
	if(num != waveform_bank){
		u8 cmd[0x200], *t;
		const struct gpif_data *w;
		//pointer initialize
		t = cmd;
		w = &WAVEFORM_DATA[num];
		//flow data の転送は farm とツールの設計が破綻している
		//1度に4つ分送る必要があるがデータが連続してない
		int i = 0, j;
		*t = CMD_FLOW;
		t++;
		i++;
		for(j = 0; j < WAVE_NUM; j++){
			memcpy(t, w->flow[j], FLOW_SIZE);
			t += FLOW_SIZE;
			i += FLOW_SIZE;
		}
		//wave data は個別に設定できるが、連続していないので意味がない
		*t = CMD_WAVE;
		t++;
		i++;
		for(j = 0; j < WAVE_NUM; j++){
			memcpy(t, w->wave[j], WAVE_SIZE);
			t += WAVE_SIZE;
			i += WAVE_SIZE;
		}
		assert(i < 0x200);
		usb_bulk_write(&handle_usb, CPIPE, cmd, i);
	}
	waveform_bank = num;
}

static void cpu_waveform_set(long address)
{
	int wavebank = WAVEFORM_BANK_CPU_6502_HIGH;
	if((address & ADDRESS_MASK_A15) == 0){
		wavebank = WAVEFORM_BANK_CPU_6502_LOW;
	}
	waveform_set(wavebank);
}

static void bus_read(long address, long length, u8 *data)
{
	const u8 CMD = CMD_SREAD;
	while(length != 0){
		address_set(address);
		usb_bulk_write(&handle_usb, CPIPE, &CMD, 1);
		usb_bulk_read(&handle_usb, 2, data, 1);
		address++;
		data++;
		length--;
	}
}

static void bus_write(long address, long data)
{
	address_set(address);
	const u8 CMD[2] = {CMD_SWRITE, data};
	usb_bulk_write(&handle_usb, CPIPE, CMD, 2);
}

static void dz_cpu_read(long address, long length, u8 *data)
{
	cpu_waveform_set(address);
	bus_read(address, length, data);
}

static void dz_cpu_6502_write(long address, long data, long wait_msec)
{
	cpu_waveform_set(address);
	bus_write(address, data);
}

static void dz_ppu_read(long address, long length, u8 *data)
{
	waveform_set(WAVEFORM_BANK_PPU);
	bus_read(address, length, data);
}

static void dz_ppu_write(long address, long data)
{
	waveform_set(WAVEFORM_BANK_PPU);
	bus_write(address, data);
}

static void dz_cpu_flash_write(long address, long data)
{
	waveform_set(WAVEFORM_BANK_CPU_FLASH_HIGH);
	bus_write(address, data);
}

const struct reader_driver DRIVER_DOZEU = {
	.name = "dozeu",
	.open_or_close = usb_open_or_close,
	.init = dz_init,
	.cpu_read = dz_cpu_read,
	.ppu_read = dz_ppu_read,
	.cpu_6502_write = dz_cpu_6502_write,
	.cpu_flash_write = dz_cpu_flash_write,
	.ppu_write = dz_ppu_write
};
