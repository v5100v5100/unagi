#ifndef _SCRIPT_H_
#define _SCRIPT_H_
#if DEBUG==1
#include "driver_master.h"
int ppu_ramtest(const struct driver *d);
#endif
void script_load(const char *inmode, const char *scriptfile, const char *targetfile, const int test_only);

struct st_variable{
	int type;
	char variable;
	long value;
};

struct st_expression{
	struct st_variable left, right;
	int operator;
};

struct script{
	int opcode;
	long value[4];
	struct st_expression expression;
	char variable;
};

enum{
	VALUE_EXPRESSION = 0x1000000,
	VALUE_VARIABLE
};
enum{
	EXPRESSION_TYPE_VARIABLE,
	EXPRESSION_TYPE_VALUE
};
#endif
