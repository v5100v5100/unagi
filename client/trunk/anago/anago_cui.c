#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "memory_manage.h"
#include "type.h"
#include "widget.h"
#include "cui_gauge.h"
#include "header.h"
#include "reader_master.h"
#include "reader_kazzo.h"
#include "script_dump.h"
#include "flash_device.h"
#include "script_program.h"

static void text_append_va(void *obj, const char *format, va_list list)
{
	vprintf(format, list);
}

static void text_append(void *obj, const char *format, ...)
{
	va_list list;
	va_start(list, format);
	text_append_va(obj, format, list);
	va_end(list);
}

static void log_set(struct textcontrol *log)
{
	log->object = NULL;
	log->append = text_append;
	log->append_va = text_append_va;
}

static bool program_rom_set(const char *device, char trans, struct memory *m, struct flash_device *f)
{
	m->offset = 0;
	if(flash_device_get(device, f) == false){
		printf("unknown flash memory device %s\n", device);
		return false;
	}
	switch(trans){
	case 'f':
		m->transtype = TRANSTYPE_FULL;
		break;
	case 't':
		m->transtype = TRANSTYPE_TOP;
		break;
	case 'b':
		m->transtype = TRANSTYPE_BOTTOM;
		break;
	case 'e':
	default: 
		m->transtype = TRANSTYPE_EMPTY;
		break;
	}
	return true;
}

static void program(int c, char **v)
{
	struct program_config config;
	config.cpu.memory.data = NULL;
	config.ppu.memory.data = NULL;
	config.script = v[2];
	config.target = v[3];
	config.control = &DRIVER_KAZZO.control;
	config.cpu.access = &DRIVER_KAZZO.cpu;
	config.ppu.access = &DRIVER_KAZZO.ppu;
	config.compare = false;
	config.testrun = false;
	switch(v[1][0]){
	case 'a':
//		config.reader = &DRIVER_DUMMY;
		config.testrun = true;
		break;
	case 'F':
		config.compare = true;
		break;
	}

	switch(c){
	case 5: {//mode script target cpu_flash_device
		char trans = 'f';
		if(v[1][1] != '\0'){
			trans = v[1][1];
		}
		if(program_rom_set(v[4], trans, &config.cpu.memory, &config.cpu.flash) == false){
			return;
		}
		if(program_rom_set("dummy", 'e', &config.ppu.memory, &config.ppu.flash) == false){
			assert(0);
			return;
		}
		}break;
	case 6: { //mode script target cpu_flash_device ppu_flash_device
		char trans = 'f';
		if(v[1][1] != '\0'){
			trans = v[1][1];
		}
		if(program_rom_set(v[4], trans, &config.cpu.memory, &config.cpu.flash) == false){
			return;
		}
		trans = 'f';
		if(v[1][1] != '\0' && v[1][2] != '\0'){
			trans = v[1][2];
		}
		if(program_rom_set(v[5], trans, &config.ppu.memory, &config.ppu.flash) == false){
			return;
		}
		}break;
	default:
		puts("mode script target cpu_flash_device ppu_flash_device");
		return;
	}
	log_set(&config.log);
	cui_gauge_new(&config.cpu.gauge, "Program  Flash", 2, -2);
	cui_gauge_new(&config.ppu.gauge, "Charcter Flash", 1, -1);
	script_program_execute(&config);
	cui_gauge_destory(&config.cpu.gauge);
	cui_gauge_destory(&config.ppu.gauge);
}

static void dump(int c, char **v)
{
	struct dump_config config;
	if(c < 4){
		puts("argument error");
		return;
	}
	config.cpu.increase = 1;
	config.ppu.increase = 1;
	config.progress = true;
	switch(v[1][0]){
	case 'D':
		config.progress = false;
		break;
	}
	switch(v[1][1]){
	case '2':
		config.cpu.increase = 2;
		break;
	case '4':
		config.cpu.increase = 4;
		break;
	}
	if(v[1][1] != '\0'){
		switch(v[1][2]){
		case '2':
			config.ppu.increase = 2;
			break;
		case '4':
			config.ppu.increase = 4;
			break;
		}
	}
	config.script = v[2];
	config.target = v[3];
	config.control = &DRIVER_KAZZO.control;
	config.cpu.access = &DRIVER_KAZZO.cpu;
	config.ppu.access = &DRIVER_KAZZO.ppu;
	cui_gauge_new(&config.cpu.gauge, "Program  ROM", 2, -2);
	cui_gauge_new(&config.ppu.gauge, "Charcter ROM", 1, -1);
	config.mappernum = -1;
	if(c == 5){
		config.mappernum = atoi(v[4]);
	}
	config.battery = false;
	log_set(&config.log);
	script_dump_execute(&config);
	cui_gauge_destory(&config.cpu.gauge);
	cui_gauge_destory(&config.ppu.gauge);
}

static void usage(const char *v)
{
	puts("famicom bus simluator 'anago'");
	printf("%s [mode] [script] [target] ....\n", v);
}

#ifdef WIN32
int main(int c, char **v)
#else
int anago_cui(int c, char **v)
#endif
{
	mm_init();
	if(c >= 2){
		switch(v[1][0]){
		case 'a': case 'f': case 'F':
			program(c, v);
			break;
		case 'd': case 'D':
			dump(c,v);
			break;
		default:
			usage(v[0]);
			puts("mode are d, D, f, g");
			break;
		}
	}else{
		usage(v[0]);
	}
	mm_end();
	return 0;
}
