#ifndef _SCRIPT_H_
#define _SCRIPT_H_
#if DEBUG==1
#include "reader_master.h"
int ppu_ramtest(const struct reader_driver *d);
#endif
struct st_config;
void script_load(const struct st_config *config);

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

struct st_config{
	//override config
	long mapper;
	int mirror, backupram;
	//target filename
	const char *ramimage_write, *ramimage_read, *romimage;
	const char *script;
	char driver[20];
	//data mode
	int mode;
};
enum{
	CONFIG_OVERRIDE_UNDEF = 4649,
	CONFIG_OVERRIDE_TRUE = 1
};

enum{
	MODE_ROM_DUMP,
	MODE_RAM_READ,
	MODE_RAM_WRITE
};

#endif
