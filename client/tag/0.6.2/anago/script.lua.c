#include <stdio.h>
#include <stdbool.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "type.h"
#include "header.h"
#include "reader_master.h"
#include "script.h"

static struct anago_flash_order{
	int program_count, command_change;
	long address, length;
	long c2aaa, c5555, unit;
	struct memory *memory;
	void (*config)(long c2aaa, long c5555, long unit);
	void (*write)(long address, long length, const uint8_t *data);
	void (*read)(long address, long length, u8 *data);
	void (*erase)(long address, bool dowait);
	long (*program)(long address, long length, u8 *data, bool dowait);
}order_cpu, order_ppu;
static inline long long_get(lua_State *t, int index)
{
	lua_Number d = lua_tonumber(t, index);
	return (long) d;
}
static void command_set(lua_State *l, struct anago_flash_order *t)
{
	long command = long_get(l, 1);
	long address = long_get(l, 2);
	long mask = long_get(l, 3);
	long d = command & (mask - 1);
	d |= address;
	switch(command){
	case 0x02aa: case 0x2aaa:
		t->c2aaa = d;
		break;
	case 0x0555: case 0x5555:
		t->c5555 = d;
		break;
	default:
		puts("unknown command address");
		return;
	}
	t->command_change += 1;
}
static int cpu_command(lua_State *l)
{
	command_set(l, &order_cpu);
	return 0;
}
static int ppu_command(lua_State *l)
{
	command_set(l, &order_ppu);
	return 0;
}
static int write(lua_State *l, struct anago_flash_order *t)
{
	long address = long_get(l, 1);
	long data = long_get(l, 2);
	uint8_t d8 = (uint8_t) data;
	t->write(address, 1, &d8);
	return 0;
}
static int cpu_write(lua_State *l)
{
	return write(l, &order_cpu);
}
static int program_regist(lua_State *l, const char *name, struct anago_flash_order *t)
{
	t->address = long_get(l, 1);
	t->length = long_get(l, 2);
	if(t->command_change != 0){
		t->config(t->c2aaa, t->c5555, t->unit);
		t->command_change = 0;
	}
	
	printf("programming %s area 0x%06x...\n", name, t->memory->offset);
	fflush(stdout);
	return lua_yield(l, 0);
}
static void program_execute(struct anago_flash_order *t)
{
	if(t->program_count == 0){
		t->erase(t->c2aaa, false);
		t->program_count += 1;
		printf("erase...\n");
		fflush(stdout);
		return;
	}
	t->program_count += 1;
//	printf("writing %06x\n", t->memory->offset);
//	fflush(stdout);
	const long w = t->program(t->address, t->length, t->memory->data + t->memory->offset, false);
	t->address += w;
	t->length -= w;
	t->memory->offset += w;
}
static int cpu_program(lua_State *l)
{
	return program_regist(l, "program ROM", &order_cpu);
}
static int ppu_program(lua_State *l)
{
	return program_regist(l, "charcter ROM", &order_ppu);
}
static int mmc1_write(lua_State *l)
{
	long address = long_get(l, 1);
	uint8_t data = (uint8_t) long_get(l, 2);
	int i = 5;
	while(i != 0){
		order_cpu.write(address, 1, &data);
		data >>= 1;
		i--;
	}
	return 0;
}

void script_execute(struct config *c)
{
	order_cpu.command_change = 0;
	order_cpu.program_count = 0;
	order_cpu.unit = c->flash_cpu->pagesize;
	order_cpu.memory = &c->rom.cpu_rom;
	order_cpu.config = c->reader->cpu_flash_config;
	order_cpu.write = c->reader->cpu_write_6502;
	order_cpu.read = c->reader->cpu_read;
	order_cpu.erase = c->reader->cpu_flash_erase;
	order_cpu.program = c->reader->cpu_flash_program;
	
	order_ppu.command_change = 0;
	order_ppu.program_count = 0;
	order_ppu.unit = c->flash_ppu->pagesize;
	order_ppu.memory = &c->rom.ppu_rom;
	order_ppu.config = c->reader->ppu_flash_config;
	order_ppu.write = c->reader->ppu_write; //warning ¤ÏÌµ»ë
	order_ppu.read = c->reader->ppu_read;
	order_ppu.erase = c->reader->ppu_flash_erase;
	order_ppu.program = c->reader->ppu_flash_program;
	
	lua_State *const l = lua_open();
	luaL_openlibs(l);
	lua_register(l, "cpu_write", cpu_write);
	lua_register(l, "cpu_program", cpu_program);
	lua_register(l, "cpu_command", cpu_command);
	lua_register(l, "ppu_program", ppu_program);
	lua_register(l, "ppu_command", ppu_command);
	lua_register(l, "mmc1_write", mmc1_write);
	if(luaL_loadfile(l, c->script) == LUA_ERRFILE){
		lua_close(l);
		return;
	}
	if(lua_pcall(l, 0, 0, 0) != 0){
		puts(lua_tostring(l, -1));
		return;
	}
	lua_getfield(l, LUA_GLOBALSINDEX, "initalize");
	lua_getglobal(l, "initalize");
	lua_call(l, 0, 0);
	lua_State *const co_cpu = lua_newthread(l);
	lua_State *const co_ppu = lua_newthread(l);
	lua_getglobal(co_cpu, "program_cpu");
	lua_getglobal(co_ppu, "program_ppu");
	int state_cpu = LUA_YIELD, state_ppu = LUA_YIELD;
	if(order_cpu.memory->size == 0 || c->flash_cpu->id_device == FLASH_ID_DEVICE_DUMMY){
		state_cpu = 0;
	}
	if(order_ppu.memory->size == 0 || c->flash_ppu->id_device == FLASH_ID_DEVICE_DUMMY){
		state_ppu = 0;
	}
	if(state_cpu != 0){
		state_cpu = lua_resume(co_cpu, 0);
	}
	if(state_ppu != 0){
		state_ppu = lua_resume(co_ppu, 0);
	}
	do{
		uint8_t s[2];
		Sleep(2);
		c->reader->flash_status(s);
		if(state_cpu != 0 && s[0] == 0){
			if(order_cpu.length == 0){
				state_cpu = lua_resume(co_cpu, 0);
			}else{
				program_execute(&order_cpu);
			}
		}
		if(state_ppu != 0 && s[1] == 0){
			if(order_ppu.length == 0){
				state_ppu = lua_resume(co_ppu, 0);
			}else{
				program_execute(&order_ppu);
			}
		}
	}while(state_cpu != 0 || state_ppu != 0);
	lua_close(l);
}
