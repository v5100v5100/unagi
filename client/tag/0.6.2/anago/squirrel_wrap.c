#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include "type.h"
#include "widget.h"

#ifdef SQUNICODE 
#define scvprintf vwprintf 
#else 
#define scvprintf vprintf 
#endif 
static void print_stdout(HSQUIRRELVM v, const SQChar *s, ...) 
{
	va_list arglist;
	va_start(arglist, s);
	scvprintf(s, arglist);
	va_end(arglist);
}

static void print_other(HSQUIRRELVM v, const SQChar *s, ...)
{
	va_list arglist;
	const struct textcontrol *p = (const struct textcontrol *) sq_getforeignptr(v);
	va_start(arglist, s);

	p->append_va(p->object, s, arglist);
	va_end(arglist);
}

HSQUIRRELVM qr_open(const struct textcontrol *p)
{
	HSQUIRRELVM v = sq_open(0x400);
	if(p == NULL){
		sq_setprintfunc(v, print_stdout);
	}else{
		sq_setforeignptr(v, (SQUserPointer) p);
		sq_setprintfunc(v, print_other);
	}
	sq_pushroottable(v);
	sqstd_seterrorhandlers(v);
	sqstd_register_iolib(v);
	return v;
}

//SQInteger 
void qr_function_register_global(HSQUIRRELVM v, const SQChar *name, SQFUNCTION f)
{
	sq_pushroottable(v);
	sq_pushstring(v, name, -1);
	sq_newclosure(v, f, 0);
	sq_createslot(v, -3); 
	sq_pop(v, 1);
}

SQRESULT qr_call(HSQUIRRELVM v, const SQChar *functionname, SQUserPointer up, const SQChar *scriptfile, int argnum, ...)
{
	SQRESULT r = SQ_ERROR;
	SQInteger top = sq_gettop(v);
	sq_pushroottable(v);
	sq_pushstring(v, functionname, -1);
	if(SQ_SUCCEEDED(sq_get(v,-2))){
		va_list ap;
		int i;
		
		sq_pushroottable(v);
		sq_pushuserpointer(v, up);
		sq_pushstring(v, scriptfile, -1);
		va_start(ap, argnum);
		for(i = 0; i < argnum; i++){
			sq_pushinteger(v, va_arg(ap, long));
		}
		r = sq_call(v, 3 + argnum, SQFalse, SQTrue); //calls the function 
		va_end(ap);
	}
	sq_settop(v, top); //restores the original stack size
	return r;
}

void qr_close(HSQUIRRELVM v)
{
	sq_pop(v, 1);
	sq_close(v); 
}

bool qr_long_get(HSQUIRRELVM v, SQInteger index, long *d)
{
	if(sq_gettype(v, index) != OT_INTEGER){
		return false;
	}
	SQInteger i;
	if(SQ_FAILED(sq_getinteger(v, index, &i))){
		return false;
	}
	*d = (long) i;
	return true;
}

SQRESULT qr_argument_get(HSQUIRRELVM v, SQInteger num, ...)
{
	va_list ap;
	if(sq_gettop(v) != (num + 2)){ //roottable, up, arguments...
		return sq_throwerror(v, _SC("argument number error"));
	}
	va_start(ap, num);
	SQInteger i;
	for(i = 0; i < num; i++){
		if(qr_long_get(v, i + 3, va_arg(ap, long *)) == false){
			return sq_throwerror(v, _SC("argument type error"));
		}
	}
	return SQ_OK;
}

SQRESULT qr_userpointer_get(HSQUIRRELVM v, SQUserPointer *up)
{
	SQRESULT r;
	assert(sq_gettype(v, 2) == OT_USERPOINTER);
	r = sq_getuserpointer(v, 2, up);
	if(SQ_FAILED(r)){
		return sq_throwerror(v, _SC("1st argument must be d (userpointer)"));
	}
	return r;
}

void qr_version_print(const struct textcontrol *l)
{
	l->append(l->object, SQUIRREL_VERSION _SC(" "));
	l->append(l->object, SQUIRREL_COPYRIGHT _SC("\n"));
}
