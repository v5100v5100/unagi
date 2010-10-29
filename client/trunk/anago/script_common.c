#include <assert.h>
#include <string.h>
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include "type.h"
#include "squirrel_wrap.h"
#include "reader_master.h"
#include "widget.h"
#include "script_common.h"

SQInteger script_nop(HSQUIRRELVM v)
{
	return 0;
}

SQInteger range_check(HSQUIRRELVM v, const char *name, long target, const struct range *range)
{
	if((target < range->start) || (target > range->end)){
		SQPRINTFUNCTION f = sq_getprintfunc(v);
		f(v, "%s range must be 0x%06x to 0x%06x", name, (int) range->start, (int) range->end);
		return sq_throwerror(v, "script logical error");
	}
	return 0;
}

SQInteger cpu_write_check(HSQUIRRELVM v)
{
	static const struct range range_address = {0x4000, 0x10000};
	static const struct range range_data = {0x0, 0xff};
	long address, data;
	SQRESULT r = qr_argument_get(v, 2, &address, &data);
	if(SQ_FAILED(r)){
		return r;
	}
	r = range_check(v, "address", address, &range_address);
	if(SQ_FAILED(r)){
		return r;
	}
	return range_check(v, "data", data, &range_data);
}

SQInteger script_require(HSQUIRRELVM v)
{
	if(sq_gettop(v) != 2){
		return sq_throwerror(v, "argument number error");
	}
	if(sq_gettype(v, 2) != OT_STRING){
		return sq_throwerror(v, "argument type error");
	}
	const SQChar *file;
	if(SQ_FAILED(sq_getstring(v, 2, &file))){
		return sq_throwerror(v, "require error");
	}
	if(SQ_FAILED(sqstd_dofile(v, _SC(file), SQFalse, SQTrue))){
		return sq_throwerror(v, "require error");
	}
	return 0;
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
			text->append(text->object, "maybe cartridge connection error\n");
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
