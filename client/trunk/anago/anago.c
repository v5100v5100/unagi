#include <stdio.h>
#include <stdbool.h>
#include "memory_manage.h"
#include "type.h"
#include "flashmemory.h"
#include "header.h"
#include "reader_master.h"
#include "reader_kazzo.h"
#include "reader_dummy.h"
#include "script.h"

static bool transtype_flash_set(char mode, struct memory *t)
{
	switch(mode){
	case 't':
		t->transtype = TRANSTYPE_TOP;
		break;
	case 'e':
		t->transtype = TRANSTYPE_EMPTY;
		break;
	case 'b':
		t->transtype = TRANSTYPE_BOTTOM;
		break;
	case 'f':
		t->transtype = TRANSTYPE_FULL;
		break;
	default:
		return false;
	}
	return true;
}
static bool transtype_set(const char *mode, struct romimage *t)
{
	switch(mode[0]){
	case 'd': case 'f':
		if(mode[1] == '\0'){
			t->cpu_rom.transtype = TRANSTYPE_FULL;
			t->ppu_rom.transtype = TRANSTYPE_FULL;
			return true;
		}
		if(transtype_flash_set(mode[1], &t->cpu_rom) == false){
			return false;
		}
		if(mode[2] == '\0'){
			t->ppu_rom.transtype = TRANSTYPE_FULL;
			return true;
		}
		if(transtype_flash_set(mode[2], &t->ppu_rom) == false){
			return false;
		}
		return true;
	}
	return false;
}
static bool config_parse(const char *romimage, const char *device_cpu, const char *device_ppu, struct config *c)
{
	c->target = romimage;
	if(nesfile_load(__FUNCTION__, romimage, &c->rom) == false){
		return false;
	}
	c->rom.cpu_rom.offset = 0;
	c->rom.ppu_rom.offset = 0;
	c->flash_cpu = flash_driver_get(device_cpu);
	if(device_ppu != NULL){
		c->flash_ppu = flash_driver_get(device_ppu);
	}else{
		c->flash_ppu = flash_driver_get("dummy");
	}
	if(c->flash_cpu == NULL || c->flash_ppu == NULL){
		return false;
	}
	if(c->flash_cpu->id_device == FLASH_ID_DEVICE_DUMMY){
		c->rom.cpu_rom.transtype = TRANSTYPE_EMPTY;
	}
	if(
		(c->flash_ppu->id_device == FLASH_ID_DEVICE_DUMMY) ||
		(c->rom.ppu_rom.size == 0)
	){
		c->rom.ppu_rom.transtype = TRANSTYPE_EMPTY;
	}
	return true;
}
static void anago(int c, char **v)
{
	struct config config;
	config.script = v[2];
	config.flash_ppu = &FLASH_DRIVER_UNDEF;
	config.reader = &DRIVER_KAZZO;
	if(v[1][0] == 'd'){
		config.reader = &DRIVER_DUMMY;
	}	
	if(transtype_set(v[1], &config.rom) == false){
		puts("mode argument error");
		return;
	}
	switch(c){
	case 5: //mode script target cpu_flash_device
		if(config_parse(v[3], v[4], NULL, &config) == false){
			nesbuffer_free(&config.rom, 0);
			return;
		}
		break;
	case 6: //mode script target cpu_flash_device ppu_flash_device
		if(config_parse(v[3], v[4], v[5], &config) == false){
			nesbuffer_free(&config.rom, 0);
			return;
		}
		break;
	default:
		puts("mode script target cpu_flash_device ppu_flash_device");
		return;
	}
	if(config.reader->open_or_close(READER_OPEN) == NG){
		puts("reader open error");
		nesbuffer_free(&config.rom, 0);
		return;
	}
	script_execute(&config);
	nesbuffer_free(&config.rom, 0);
	config.reader->open_or_close(READER_CLOSE);
}
int main(int c, char **v)
{
	mm_init();
	anago(c, v);
	mm_end();
	return 0;
}
