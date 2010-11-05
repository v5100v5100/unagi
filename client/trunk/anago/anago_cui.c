#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "memory_manage.h"
#include "type.h"
#include "widget.h"
#include "cui_gauge.h"
#include "romimage.h"
#include "reader_master.h"
#include "reader_kazzo.h"
#include "script_dump.h"
#include "flash_device.h"
#include "script_program.h"

#ifdef _UNICODE
  #define PUTS _putws
  #define PRINTF wprintf
#else
  #define PUTS puts
  #define PRINTF printf
#endif

static void text_append_va(void *obj, const wgChar *format, va_list list)
{
#ifdef _UNICODE
	vwprintf(format, list);
#else
	vprintf(format, list);
#endif
}

static void text_append(void *obj, const wgChar *format, ...)
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

static void except(const wgChar *str)
{
	PUTS(str);
	exit(0);
}

static bool program_rom_set(const wgChar *device, wgChar trans, struct memory *m, struct flash_device *f)
{
	m->offset = 0;
	if(flash_device_get(device, f) == false){
		PRINTF(wgT("unknown flash memory device %s\n"), device);
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

static void program(int c, wgChar **v, const struct reader_driver *r)
{
	struct program_config config;
	config.cpu.memory.data = NULL;
	config.ppu.memory.data = NULL;
	config.script = v[2];
	config.target = v[3];
	config.control = &r->control;
	config.cpu.access = &r->cpu;
	config.ppu.access = &r->ppu;
	config.compare = false;
	config.testrun = false;
	switch(v[1][0]){
	case 'a':
		config.testrun = true;
		break;
	case 'F':
		config.compare = true;
		break;
	}

	switch(c){
	case 5: {//mode script target cpu_flash_device
		wgChar trans = wgT('f');
		if(v[1][1] != wgT('\0')){
			trans = v[1][1];
		}
		if(program_rom_set(v[4], trans, &config.cpu.memory, &config.cpu.flash) == false){
			return;
		}
		if(program_rom_set(wgT("dummy"), wgT('e'), &config.ppu.memory, &config.ppu.flash) == false){
			assert(0);
			return;
		}
		}break;
	case 6: { //mode script target cpu_flash_device ppu_flash_device
		wgChar trans = wgT('f');
		if(v[1][1] != wgT('\0')){
			trans = v[1][1];
		}
		if(program_rom_set(v[4], trans, &config.cpu.memory, &config.cpu.flash) == false){
			return;
		}
		trans = wgT('f');
		if(v[1][1] != wgT('\0') && v[1][2] != wgT('\0')){
			trans = v[1][2];
		}
		if(program_rom_set(v[5], trans, &config.ppu.memory, &config.ppu.flash) == false){
			return;
		}
		}break;
	default:
		PUTS(wgT("mode script target cpu_flash_device ppu_flash_device"));
		return;
	}
	log_set(&config.log);
	cui_gauge_new(&config.cpu.gauge, wgT("Program  Flash"), 2, -2);
	cui_gauge_new(&config.ppu.gauge, wgT("Charcter Flash"), 1, -1);
	config.except = except;
	script_program_execute(&config);
	cui_gauge_destory(&config.cpu.gauge);
	cui_gauge_destory(&config.ppu.gauge);
}

static void dump(int c, wgChar **v, const struct reader_driver *r)
{
	struct dump_config config;
	if(c < 4){
		PUTS(wgT("argument error"));
		return;
	}
	config.cpu.increase = 1;
	config.ppu.increase = 1;
	config.progress = true;
	switch(v[1][0]){
	case wgT('d'):
		config.mode = MODE_ROM_DUMP;
		break;
	case wgT('D'):
		config.mode = MODE_ROM_DUMP;
		config.progress = false;
		break;
	case wgT('r'): case wgT('R'):
		config.mode = MODE_RAM_READ;
		break;
	case wgT('w'): case wgT('W'):
		config.mode = MODE_RAM_WRITE;
		break;
	}
	switch(v[1][1]){
	case wgT('2'):
		config.cpu.increase = 2;
		break;
	case wgT('4'):
		config.cpu.increase = 4;
		break;
	}
	if(v[1][1] != wgT('\0')){
		switch(v[1][2]){
		case wgT('2'):
			config.ppu.increase = 2;
			break;
		case wgT('4'):
			config.ppu.increase = 4;
			break;
		}
	}
	config.script = v[2];
	config.target = v[3];
	config.control = &r->control;
	config.cpu.access = &r->cpu;
	config.ppu.access = &r->ppu;
	cui_gauge_new(&config.cpu.gauge, wgT("Program  ROM"), 2, -2);
	cui_gauge_new(&config.ppu.gauge, wgT("Charcter ROM"), 1, -1);
	config.except = except;
	config.mappernum = -1;
	if(c == 5){
#ifdef _UNICODE
		config.mappernum = _wtoi(v[4]);
#else
		config.mappernum = atoi(v[4]);
#endif
	}
	config.battery = false;
	log_set(&config.log);
	if(config.mode == MODE_ROM_DUMP){
		script_dump_execute(&config);
	}else{
		script_workram_execute(&config);
	}
	cui_gauge_destory(&config.cpu.gauge);
	cui_gauge_destory(&config.ppu.gauge);
}

static void usage(const wgChar *v)
{
	PUTS(wgT("famicom bus simluator 'anago'"));
	PRINTF(wgT("%s [mode] [script] [target] ....\n"), v);
}

#if DEBUG==1
extern const struct reader_driver DRIVER_DUMMY;
#endif

#ifdef WIN32
int main(int c, char **vv)
#else
int anago_cui(int c, wgChar **v)
#endif
{
	mm_init();
	if(c >= 2){
		const struct reader_driver *r = &DRIVER_KAZZO;
#ifdef _UNICODE
		int i;
		wchar_t **v;
		v = Malloc(sizeof(wchar_t *) * c);
		for(i = 0; i < c; i++){
			size_t len = strlen(vv[i]) + 1;
			v[i] = Malloc(sizeof(wchar_t) * len);
			mbstowcs(v[i], vv[i], len);
		}
#endif
		switch(v[1][0]){
#if DEBUG==1
		case wgT('x'):
			r = &DRIVER_DUMMY; //though down
#endif
		case wgT('f'): case wgT('F'):
			program(c, v, r);
			break;
#if DEBUG==1
		case wgT('z'): case wgT('R'): case wgT('W'): 
			r = &DRIVER_DUMMY; //though down
#endif
		case wgT('d'): case wgT('D'):
		case wgT('r'): case wgT('w'):
			dump(c,v, r);
			break;
		default:
			usage(v[0]);
			PUTS(wgT("mode are d, D, f, g"));
			break;
		}
#ifdef _UNICODE
		for(i = 0; i < c; i++){
			Free(v[i]);
		}
		Free(v);
#endif
	}else{ //usage
#ifdef _UNICODE
		size_t len = strlen(vv[0]) + 1;
		wchar_t *t = Malloc(sizeof(wchar_t) * len);
		mbstowcs(t, vv[0], len);
		usage(t);
		Free(t);
#else
		usage(v[0]);
#endif
	}
	mm_end();
	return 0;
}
