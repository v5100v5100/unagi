#ifndef _SCRIPT_COMMON_H_
#define _SCRIPT_COMMON_H_
struct range{
	long start, end;
};
SQInteger script_nop(HSQUIRRELVM v);
SQInteger range_check(HSQUIRRELVM v, const wgChar *name, long target, const struct range *range);
SQInteger cpu_write_check(HSQUIRRELVM v);
SQInteger script_require(HSQUIRRELVM v);

struct reader_handle;
struct reader_memory_access;
bool connection_check(const struct reader_handle *h, const struct textcontrol *text, const struct reader_memory_access *cpu, const struct reader_memory_access *ppu);
#endif
