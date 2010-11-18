#ifndef _SQUIRREL_WRAP_H_
#define _SQUIRREL_WRAP_H_
struct textcontrol;
HSQUIRRELVM qr_open(struct textcontrol *p);
void qr_function_register_global(HSQUIRRELVM v, const wgChar *name, SQFUNCTION f);
bool qr_long_get(HSQUIRRELVM v, SQInteger index, long *d);
SQRESULT qr_call(HSQUIRRELVM v, const SQChar *functionname, SQUserPointer up, const SQChar *scriptfile, int argnum, ...);
void qr_close(HSQUIRRELVM v);
SQRESULT qr_argument_get(HSQUIRRELVM v, SQInteger num, ...);
SQRESULT qr_userpointer_get(HSQUIRRELVM v, SQUserPointer *up);
#endif
