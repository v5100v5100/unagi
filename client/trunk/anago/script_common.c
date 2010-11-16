#include <assert.h>
#include <string.h>
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include "type.h"
#include "memory_manage.h"
#include "squirrel_wrap.h"
#include "reader_master.h"
#include "widget.h"
#include "script_common.h"

SQInteger script_nop(HSQUIRRELVM v)
{
	return 0;
}

SQInteger range_check(HSQUIRRELVM v, const wgChar *name, long target, const struct range *range)
{
	if((target < range->start) || (target > range->end)){
		SQPRINTFUNCTION f = sq_getprintfunc(v);
		f(v, wgT("%s range must be 0x%06x to 0x%06x"), name, (int) range->start, (int) range->end);
		return sq_throwerror(v, wgT("script logical error"));
	}
	return 0;
}

SQInteger cpu_write_check(HSQUIRRELVM v)
{
	static const struct range range_address = {0x4000, 0x10000};
	static const struct range range_data = {0x0, 0xff};
	const SQInteger data_index = 4;
	long address;
	switch(sq_gettype(v, data_index)){
	case OT_INTEGER:{
		long data;
		SQRESULT r = qr_argument_get(v, 2, &address, &data);
		if(SQ_FAILED(r)){
			return r;
		}
		r = range_check(v, wgT("address"), address, &range_address);
		if(SQ_FAILED(r)){
			return r;
		}
		return range_check(v, wgT("data"), data, &range_data);}
	case OT_ARRAY:{
		SQInteger i;
		if(qr_long_get(v, 3, &address) == false){
			return sq_throwerror(v, _SC("argument number error"));
		}
		for(i = 0; i < sq_getsize(v, data_index); i++){
			long data;
			SQRESULT r = range_check(v, wgT("address"), address, &range_address);
			if(SQ_FAILED(r)){
				return r;
			}
			sq_pushinteger(v, i);
			if(SQ_FAILED(sq_get(v, -2))){
				return r;
			}
			if(qr_long_get(v, -1, &data) == false){
				return sq_throwerror(v, wgT("script type error"));
			}
			sq_pop(v, 1);
			r = range_check(v, wgT("data"), data, &range_data);
			if(SQ_FAILED(r)){
				return r;
			}
			address++;
		}
		return 0;}
	default:
		return sq_throwerror(v, wgT("script type error"));
	}
	return 0;
}

void cpu_write_execute(HSQUIRRELVM v, const struct reader_handle *h, const struct reader_memory_access *t)
{
	const SQInteger data_index = 4;
	long address;
	switch(sq_gettype(v, data_index)){
	case OT_INTEGER:{
		long data;
		uint8_t d8;
		SQRESULT r = qr_argument_get(v, 2, &address, &data);
		assert(r == SQ_OK);
		d8 = (uint8_t) (data & 0xff);
		t->memory_write(h, address, 1, &d8);
		}break;
	case OT_ARRAY:{
		long i;
		bool rr = qr_long_get(v, 3, &address);
		assert(rr == true);
		
		const long size = (long) sq_getsize(v, data_index);
		uint8_t *const d8 = Malloc(size * sizeof(uint8_t));
		
		for(i = 0; i < size; i++){
			long data;
			SQRESULT r;
			sq_pushinteger(v, i);
			r = sq_get(v, -2);
			assert(r == SQ_OK);
			rr = qr_long_get(v, -1, &data);
			assert(rr = true);
			sq_pop(v, 1);
			d8[i] = (uint8_t) (data & 0xff);
		}
		t->memory_write(h, address, size, d8);
		Free(d8);
		}break;
	default:
		assert(0);
		break;
	}
	return;
}

static bool connection_check_main(const struct reader_handle *h, const struct textcontrol *text, const struct reader_memory_access *m, long address)
{
	const int size = 0x10;
	uint8_t test1[size], test_m[size];
	int i;
	
	m->memory_read(h, &GAUGE_DUMMY, address, size, test1);
	for(i = 0; i < 3; i++){
		m->memory_read(h, &GAUGE_DUMMY, address, size, test_m);
		if(memcmp(test1, test_m, size) != 0){
			text->append(text->object, wgT("maybe cartridge connection error\n"));
			return false;
		}
	}
	return true;
}
bool connection_check(const struct reader_handle *h, const struct textcontrol *text, const struct reader_memory_access *cpu, const struct reader_memory_access *ppu)
{
	if(connection_check_main(h, text, cpu, 0xf000) == false){
		return false;
	}
	if(connection_check_main(h, text, ppu, 0x0000) == false){
		return false;
	}
	return true;
}
