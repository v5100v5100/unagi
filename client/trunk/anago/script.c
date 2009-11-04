#include <stdio.h>
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include "type.h"
#include "header.h"
#include "reader_master.h"
#include "squirrel_wrap.h"
#include "script.h"

static struct anago_flash_order{
	bool command_change;
	long address, length;
	long c000x, c2aaa, c5555, unit;
	struct memory *memory;
	void (*config)(long c000x, long c2aaa, long c5555, long unit);
	void (*device_get)(uint8_t s[2]);
	void (*flash_status)(uint8_t s[2]);
	void (*write)(long address, long length, const uint8_t *data);
	void (*read)(long address, long length, u8 *data);
	void (*erase)(long address, bool dowait);
	long (*program)(long address, long length, const u8 *data, bool dowait);
}order_cpu, order_ppu;

static SQInteger command_set(HSQUIRRELVM v, struct anago_flash_order *t)
{
	long command, address ,mask;
	SQRESULT r = qr_argument_get(v, 3, &command, &address, &mask);
	if(SQ_FAILED(r)){
		return r;
	}
	long d = command & (mask - 1);
	d |= address;
	switch(command){
	case 0x0000:
		t->c000x = d;
		break;
	case 0x02aa: case 0x2aaa:
		t->c2aaa = d;
		break;
	case 0x0555: case 0x5555:
		t->c5555 = d;
		break;
	default:
		return sq_throwerror(v, "unknown command address");
	}
	t->command_change = true;
	return 0;
}
static SQInteger cpu_command(HSQUIRRELVM v)
{
	return command_set(v, &order_cpu);
}
static SQInteger ppu_command(HSQUIRRELVM v)
{
	return command_set(v, &order_ppu);
}
static SQInteger write(HSQUIRRELVM v, struct anago_flash_order *t)
{
	long address, data;
	SQRESULT r = qr_argument_get(v, 2, &address, &data);
	if(SQ_FAILED(r)){
		return r;
	}
	uint8_t d8 = (uint8_t) data;
	t->write(address, 1, &d8);
	return 0;
}
static SQInteger cpu_write(HSQUIRRELVM v)
{
	return write(v, &order_cpu);
}
static SQInteger erase_set(HSQUIRRELVM v, struct anago_flash_order *t, const char *region)
{
	t->config(t->c000x, t->c2aaa, t->c5555, t->unit);
	t->command_change = false;
	t->erase(t->c2aaa, false);
	printf("erasing %s memory...\n", region);
	fflush(stdout);
	return 0; //sq_suspendvm(v);
}
static SQInteger cpu_erase(HSQUIRRELVM v)
{
	return erase_set(v, &order_cpu, "program");
}
static SQInteger ppu_erase(HSQUIRRELVM v)
{
	return erase_set(v, &order_ppu, "charcter");
}
static SQInteger program_regist(HSQUIRRELVM v, const char *name, struct anago_flash_order *t)
{
	SQRESULT r = qr_argument_get(v, 2, &t->address, &t->length);
	if(SQ_FAILED(r)){
		return r;
	}
	if(t->command_change == true){
		t->config(t->c000x, t->c2aaa, t->c5555, t->unit);
		t->command_change = false;
	}
	
	printf("programming %s ROM area 0x%06x...\n", name, t->memory->offset);
	fflush(stdout);
	return sq_suspendvm(v);
}
static void program_execute(struct anago_flash_order *t)
{
/*	printf("writing %06x\n", t->memory->offset);
	fflush(stdout);*/
	const long w = t->program(t->address, t->length, t->memory->data + t->memory->offset, false);
	t->address += w;
	t->length -= w;
	t->memory->offset += w;
}
static SQInteger cpu_program(HSQUIRRELVM v)
{
	return program_regist(v, "program", &order_cpu);
}
static SQInteger ppu_program(HSQUIRRELVM v)
{
	return program_regist(v, "charcter", &order_ppu);
}

static SQInteger erase_wait(HSQUIRRELVM v)
{
	uint8_t s[2];
	do{
		Sleep(2);
		order_cpu.flash_status(s);
	}while((s[0] != 0) && (s[1] != 0));
	return 0;
}
static void execute_main(HSQUIRRELVM v, struct config *c)
{
	if(SQ_FAILED(sqstd_dofile(v, _SC("flashmode.nut"), SQFalse, SQTrue)))
	{
		return;
	}
	if(SQ_FAILED(sqstd_dofile(v, _SC(c->script), SQFalse, SQTrue)))
	{
		return;
	}
	qr_call(
		v, "program_init", true, 5, c->rom.mappernum, 
		order_cpu.memory->transtype, order_cpu.memory->size, 
		order_ppu.memory->transtype, order_ppu.memory->size
	);
}
#if 0
	HSQUIRRELVM co_cpu = sq_newthread(v, 0x400);
	HSQUIRRELVM co_ppu = sq_newthread(v, 0x400);
	SQInteger state_cpu, state_ppu;
	qr_call(v, "initalize", true);
	if(order_cpu.memory->size == 0 || c->flash_cpu->id_device == FLASH_ID_DEVICE_DUMMY){
		state_cpu = SQ_VMSTATE_IDLE;
	}else{
		qr_call(co_cpu, "program_cpu", false);
		state_cpu = sq_getvmstate(co_cpu);
/*	config.reader->ppu_flash_device_get(s);
	printf("ppu device %02x %02x\n", s[0], s[1]);*/
	}
	if(order_ppu.memory->size == 0 || c->flash_ppu->id_device == FLASH_ID_DEVICE_DUMMY){
		state_ppu = SQ_VMSTATE_IDLE;
	}else{
		qr_call(co_ppu, "program_ppu", false);
		state_ppu = sq_getvmstate(co_ppu);
	}
#endif
static SQInteger program_main(HSQUIRRELVM v)
{
	if(sq_gettop(v) != (2 + 1)){
		return sq_throwerror(v, "argument number error");
	}
	HSQUIRRELVM co_cpu, co_ppu;
	if(SQ_FAILED(sq_getthread(v, 2, &co_cpu))){
		return sq_throwerror(v, "thread error");
	}
	if(SQ_FAILED(sq_getthread(v, 3, &co_ppu))){
		return sq_throwerror(v, "thread error");
	}
	SQInteger state_cpu = sq_getvmstate(co_cpu);
	SQInteger state_ppu = sq_getvmstate(co_ppu);
	while(state_cpu != SQ_VMSTATE_IDLE || state_ppu != SQ_VMSTATE_IDLE){
		uint8_t s[2];
		Sleep(2);
		order_cpu.flash_status(s);
		if(state_cpu != SQ_VMSTATE_IDLE && s[0] == 0){
			if(order_cpu.length == 0){
				sq_wakeupvm(co_cpu, SQFalse, SQFalse, SQTrue/*, SQTrue*/);
				state_cpu = sq_getvmstate(co_cpu);
			}else{
				program_execute(&order_cpu);
			}
		}
		if(state_ppu != SQ_VMSTATE_IDLE && s[1] == 0){
			if(order_ppu.length == 0){
				sq_wakeupvm(co_ppu, SQFalse, SQFalse, SQTrue/*, SQTrue*/);
				state_ppu = sq_getvmstate(co_ppu);
			}else{
				program_execute(&order_ppu);
			}
		}
	}
	return 0;
}
void script_execute(struct config *c)
{
	order_cpu.command_change = true;
	order_cpu.unit = c->flash_cpu->pagesize;
	order_cpu.memory = &c->rom.cpu_rom;
	order_cpu.config = c->reader->cpu_flash_config;
	order_cpu.device_get = c->reader->cpu_flash_device_get;
	order_cpu.write = c->reader->cpu_write_6502;
	order_cpu.read = c->reader->cpu_read;
	order_cpu.erase = c->reader->cpu_flash_erase;
	order_cpu.program = c->reader->cpu_flash_program;
	order_cpu.flash_status = c->reader->flash_status;
	
	order_ppu.command_change = true;
	order_ppu.unit = c->flash_ppu->pagesize;
	order_ppu.memory = &c->rom.ppu_rom;
	order_ppu.config = c->reader->ppu_flash_config;
	order_ppu.device_get = c->reader->ppu_flash_device_get;
	order_ppu.write = c->reader->ppu_write; //warning ¤ÏÌµ»ë
	order_ppu.read = c->reader->ppu_read;
	order_ppu.erase = c->reader->ppu_flash_erase;
	order_ppu.program = c->reader->ppu_flash_program;
	order_ppu.flash_status = c->reader->flash_status;
	
	HSQUIRRELVM v = qr_open(); 
	qr_function_register_global(v, "cpu_write", cpu_write);
	qr_function_register_global(v, "cpu_erase", cpu_erase);
	qr_function_register_global(v, "cpu_program", cpu_program);
	qr_function_register_global(v, "cpu_command", cpu_command);
	qr_function_register_global(v, "ppu_erase", ppu_erase);
	qr_function_register_global(v, "ppu_program", ppu_program);
	qr_function_register_global(v, "ppu_command", ppu_command);
	qr_function_register_global(v, "program_main", program_main);
	qr_function_register_global(v, "erase_wait", erase_wait);
	
	execute_main(v, c);

	qr_close(v);
}
