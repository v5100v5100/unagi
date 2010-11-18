#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include "type.h"
#include "memory_manage.h"
#include "squirrel_wrap.h"
#include "flash_device.h"

static void call(HSQUIRRELVM v, const wgChar *devicename)
{
	sq_pushroottable(v);
	sq_pushstring(v, _SC("flash_device_get"), -1);
	if(SQ_SUCCEEDED(sq_get(v,-2))){
		sq_pushroottable(v);
		sq_pushstring(v, devicename, -1);
		sq_call(v, 2, SQTrue, SQTrue);
	}
}
static bool long_get(HSQUIRRELVM v, const wgChar *field, long *ret)
{
	sq_pushstring(v, field, -1);
	SQRESULT r = sq_get(v, -2);
	if(r != SQ_OK){
		return false;
	}
	if(sq_gettype(v, -1) != OT_INTEGER){
		return false;
	}
	SQInteger i;
	r = sq_getinteger(v, -1, &i);
	if(r != SQ_OK){
		return false;
	}
	*ret = (long) i;
	sq_pop(v, 1);
	return true;
}
static bool bool_get(HSQUIRRELVM v, const wgChar *field, bool *ret)
{
	sq_pushstring(v, field, -1);
	SQRESULT r = sq_get(v, -2);
	if(r != SQ_OK){
		return false;
	}
	if(sq_gettype(v, -1) != OT_BOOL){
		return false;
	}
	SQBool i;
	r = sq_getbool(v, -1, &i);
	if(r != SQ_OK){
		return false;
	}
	if(i == SQTrue){
		*ret = true;
	}else{
		*ret = false;
	}
	sq_pop(v, 1);
	return true;
}
bool flash_device_get(const wgChar *name, struct flash_device *t)
{
	HSQUIRRELVM v = qr_open(NULL); 
	if(SQ_FAILED(sqstd_dofile(v, _SC("flashdevice.nut"), SQFalse, SQTrue))){
		qr_close(v);
		return false;
	}
	SQInteger top = sq_gettop(v);
	call(v, name);
	if(sq_gettype(v, -1) != OT_TABLE){
		goto field_error;
	}
	t->name = name;
	if(long_get(v, _SC("capacity"), &t->capacity) == false){
		goto field_error;
	}
	if(long_get(v, _SC("pagesize"), &t->pagesize) == false){
		goto field_error;
	}
	if(long_get(v, _SC("erase_wait"), &t->erase_wait) == false){
		goto field_error;
	}
	if(bool_get(v, _SC("erase_require"), &t->erase_require) == false){
		goto field_error;
	}
	if(bool_get(v, _SC("retry"), &t->retry) == false){
		goto field_error;
	}
	if(long_get(v, _SC("command_mask"), &t->command_mask) == false){
		goto field_error;
	}
	long dd;
	if(long_get(v, _SC("id_manufacurer"), &dd) == false){
		goto field_error;
	}
	t->id_manufacurer = dd;
	if(long_get(v, _SC("id_device"), &dd) == false){
		goto field_error;
	}
	t->id_device = dd;
	sq_settop(v, top);
	qr_close(v);
	return true;

field_error:
//	puts("script field error");
	qr_close(v);
	return false;
}

static int flash_device_number_get(HSQUIRRELVM v)
{
	sq_pushroottable(v);
	sq_pushstring(v, _SC("flash_device_number_get"), -1);
	if(SQ_SUCCEEDED(sq_get(v,-2))){
		sq_pushroottable(v);
		SQRESULT r = sq_call(v, 1, SQTrue, SQTrue);
		assert(r == SQ_OK);
		r++;
	}
	if(sq_gettype(v, -1) != OT_INTEGER){
		return 0;
	}
	SQInteger i;
	if(SQ_FAILED(sq_getinteger(v, -1, &i))){
		return 0;
	}
	return i;
}

static int flash_device_name_get(HSQUIRRELVM v, int index, const wgChar **str)
{
	sq_pushroottable(v);
	sq_pushstring(v, _SC("flash_device_name_get"), -1);
	if(SQ_SUCCEEDED(sq_get(v,-2))){
		sq_pushroottable(v);
		sq_pushinteger(v, index);
		SQRESULT r = sq_call(v, 2, SQTrue, SQTrue);
		assert(r == SQ_OK);
		r++;
	}
	if(sq_gettype(v, -1) != OT_STRING){
		return 0;
	}
	if(SQ_FAILED(sq_getstring(v, -1, str))){
		return 0;
	}
	return 1;
}

#ifdef _UNICODE
  #define STRNCMP wcsncmp
#else
  #define STRNCMP strncmp
#endif
void flash_device_listup(struct flash_listup *t)
{
	const wgChar *str;
	HSQUIRRELVM v = qr_open(NULL);
	SQInteger top = sq_gettop(v);

	if(SQ_FAILED(sqstd_dofile(v, _SC("flashdevice.nut"), SQFalse, SQTrue))){
//		puts("flash device script error");
		qr_close(v);
		return;
	}

	int i;
	const int device_num = flash_device_number_get(v);
	sq_settop(v, top);

	for(i = 0; i < device_num; i++){
		flash_device_name_get(v, i, &str);
		if(STRNCMP(str, _SC("dummy"), 6) != 0){
			t->append(t->obj_cpu, str);
			t->append(t->obj_ppu, str);
		}
		sq_settop(v, top);
	}

	qr_close(v);
	v = NULL;
}
