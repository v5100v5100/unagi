/*
famicom ROM cartridge dump program - unagi
script engine

todo: 
* 別の読み出しハードに対応したときは cpu_read などを関数ポインタにまとめた struct を用意して実行する
* 変数管理のグローバル値を、logical_test(), excute() ローカルにしたい
* RAM アクセスができ次第、RAM 読み出しスクリプトも設計する
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "file.h"
#include "paralellport.h"
#include "giveio.h"
#include "textutil.h"
#include "header.h"
#include "script.h"

#define OP_PPU_WRITE_ENABLE (0)
/*
MAPPER num
MIRROR [HV]
CPU_ROMSIZE num
CPU_RAMSIZE num
PPU_ROMSIZE num
DUMP_START
CPU_READ address size
CPU_WRITE address data -> 変数展開+演算子使用可能
PPU_READ address size
STEP_START variable start end step -> for(i=start;i<end;i+=step)
STEP_END
DUMP_END
*/
struct script_syntax{
	const char *name;
	int script_opcode;
	int argc, compare;
	int argv_type[4];
};
enum{
	SYNTAX_ARGVTYPE_NULL,
	SYNTAX_ARGVTYPE_VALUE,
	SYNTAX_ARGVTYPE_HV,
	SYNTAX_ARGVTYPE_EXPRESSION,
	SYNTAX_ARGVTYPE_VARIABLE
};
enum{
	SYNTAX_COMPARE_EQ,
	SYNTAX_COMPARE_GT
};
enum{
	SCRIPT_OPCODE_MAPPER,
	SCRIPT_OPCODE_MIRROR,
	SCRIPT_OPCODE_CPU_ROMSIZE,
	SCRIPT_OPCODE_CPU_RAMSIZE,
	SCRIPT_OPCODE_PPU_ROMSIZE,
	SCRIPT_OPCODE_DUMP_START,
	SCRIPT_OPCODE_CPU_READ,
	SCRIPT_OPCODE_CPU_WRITE,
	SCRIPT_OPCODE_CPU_RAMRW,
	SCRIPT_OPCODE_PPU_RAMTEST,
	SCRIPT_OPCODE_PPU_READ,
	SCRIPT_OPCODE_PPU_WRITE,
	SCRIPT_OPCODE_STEP_START,
	SCRIPT_OPCODE_STEP_END,
	SCRIPT_OPCODE_DUMP_END,
	SCRIPT_OPCODE_COMMENT,
	SCRIPT_OPCODE_NUM
};
static const char OPSTR_CPU_ROMSIZE[] = "CPU_ROMSIZE";
static const char OPSTR_CPU_RAMSIZE[] = "CPU_RAMSIZE";
static const char OPSTR_PPU_ROMSIZE[] = "PPU_ROMSIZE";
static const char OPSTR_CPU_RAMRW[] = "CPU_RAMRW";
static const struct script_syntax SCRIPT_SYNTAX[] = {
	{"MAPPER", SCRIPT_OPCODE_MAPPER, 1, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{"MIRROR", SCRIPT_OPCODE_MIRROR, 1, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_HV, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{OPSTR_CPU_ROMSIZE, SCRIPT_OPCODE_CPU_ROMSIZE, 1, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{OPSTR_CPU_RAMSIZE, SCRIPT_OPCODE_CPU_RAMSIZE, 1, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{OPSTR_PPU_ROMSIZE, SCRIPT_OPCODE_PPU_ROMSIZE, 1, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{"DUMP_START", SCRIPT_OPCODE_DUMP_START, 0, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{"CPU_READ", SCRIPT_OPCODE_CPU_READ, 2, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{"CPU_WRITE", SCRIPT_OPCODE_CPU_WRITE, 2, SYNTAX_COMPARE_GT, {SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_EXPRESSION, SYNTAX_ARGVTYPE_EXPRESSION, SYNTAX_ARGVTYPE_EXPRESSION}},
	{OPSTR_CPU_RAMRW, SCRIPT_OPCODE_CPU_RAMRW, 2, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{"PPU_RAMTEST", SCRIPT_OPCODE_PPU_RAMTEST, 0, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{"PPU_READ", SCRIPT_OPCODE_PPU_READ, 2, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
#if OP_PPU_WRITE_ENABLE==1
	{"PPU_WRITE", SCRIPT_OPCODE_PPU_WRITE, 2, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
#endif
	{"STEP_START", SCRIPT_OPCODE_STEP_START, 4, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_VARIABLE, SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_VALUE, SYNTAX_ARGVTYPE_VALUE}},
	{"STEP_END", SCRIPT_OPCODE_STEP_END, 0, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}},
	{"DUMP_END", SCRIPT_OPCODE_DUMP_END, 0, SYNTAX_COMPARE_EQ, {SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL, SYNTAX_ARGVTYPE_NULL}}
};

//変数管理
struct variable_manage{
	char name;
	long start,end,step;
	long val;
	const struct script *Continue;
};

enum{
	STEP_MAX = 2,
	VARIABLE_MAX = STEP_MAX
};

static const struct variable_manage VARIABLE_INIT = {
	'\0', 0, 0, 0, 0,
	NULL
};
static struct variable_manage variable_bank[VARIABLE_MAX];
static int variable_num = 0;

static void variable_init_single(int num)
{
	memcpy(&variable_bank[num], &VARIABLE_INIT, sizeof(struct variable_manage));
}

static void variable_init_all(void)
{
	int i;
	variable_num = 0;
	for(i = 0; i < VARIABLE_MAX; i++){
		variable_init_single(i);
	}
}

static int variable_get(char name, long *val)
{
	int i;
	struct variable_manage *v;
	v = variable_bank;
	for(i = 0; i < variable_num; i++){
		if(v->name == name){
			*val = v->val;
			return OK;
		}
		v++;
	}
	return NG;
}

//変数展開
static int expression_calc(const struct st_expression *e, long *val)
{
	long left, right;
	if(e->left.type == EXPRESSION_TYPE_VARIABLE){
		if(variable_get(e->left.variable, &left) == NG){
			return NG;
		}
	}else{
		left = e->left.value;
	}
	if(e->operator == OPERATOR_NONE){
		*val = left;
		return OK;
	}
	if(e->right.type == EXPRESSION_TYPE_VARIABLE){
		if(variable_get(e->right.variable, &right) == NG){
			return NG;
		}
	}else{
		right = e->right.value;
	}
	switch(e->operator){
	case OPERATOR_PLUS:
		*val = left + right;
		break;
	case OPERATOR_SHIFT_LEFT:
		*val = left >> right;
		//*val &= 1;
		break;
	case OPERATOR_SHIFT_RIGHT:
		*val = left << right;
		break;
	case OPERATOR_AND:
		*val = left & right;
		break;
	case OPERATOR_OR:
		*val = left | right;
		break;
	case OPERATOR_XOR:
		*val = left ^ right;
		break;
	}
	
	return OK;
}

static int step_new(char name, long start, long end, long step, const struct script *Continue)
{
	if(variable_num >= VARIABLE_MAX){
		return NG; //変数定義が多すぎ
	}
	struct variable_manage *v;
	int i;
	v = variable_bank;
	for(i = 0; i < variable_num; i++){
		if(v->name == name){
			return NG; //変数名重複
		}
		v++;
	}
	v = variable_bank;
	v += variable_num;
	v->name = name;
	v->start = start;
	v->end = end;
	v->step = step;
	v->val = start;
	v->Continue = Continue;
	variable_num++;
	return OK;
}

static const struct script *step_end(const struct script *Break)
{
	//現在のループの対象変数を得る
	struct variable_manage *v;
	v = variable_bank;
	v += (variable_num - 1);
	//変数更新
	v->val += v->step;
	if(v->val < v->end){
		return v->Continue;
	}
	//ループが終わったのでその変数を破棄する
	variable_init_single(variable_num - 1);
	variable_num--;
	return Break;
}

static int syntax_check_expression(char **word, int word_num, struct st_expression *e)
{
	if(word_num == 0){
		return NG;
	}
	//left
	if(value_get(word[0], &(e->left.value)) == OK){
		e->left.type = EXPRESSION_TYPE_VALUE;
	}else{
		e->left.type = EXPRESSION_TYPE_VARIABLE;
		e->left.variable = word[0][0];
	}
	word_num--;
	if(word_num == 0){
		e->operator = OPERATOR_NONE;
		return OK;
	}
	//operator
	e->operator = operator_get(word[1]);
	if(e->operator == OPERATOR_ERROR){
		return NG;
	}
	word_num--;
	if(word_num == 0){
		return NG;
	}
	//right
	if(value_get(word[2], &(e->right.value)) == OK){
		e->right.type = EXPRESSION_TYPE_VALUE;
	}else{
		e->right.type = EXPRESSION_TYPE_VARIABLE;
		e->right.variable = word[2][0];
	}
	return OK;
}

static const char SYNTAX_ERROR_PREFIX[] = "syntax error:";

static int syntax_check_phase(char **word, int word_num, struct script *s)
{
	int i = sizeof(SCRIPT_SYNTAX) / sizeof(SCRIPT_SYNTAX[0]);
	const struct script_syntax *syntax;
	syntax = SCRIPT_SYNTAX;
	while(i != 0){
		if(strcmp(syntax->name, word[0]) == 0){
			int j;
			
			s->opcode = syntax->script_opcode;
			{
				int compare = 0;
				switch(syntax->compare){
				case SYNTAX_COMPARE_EQ:
					compare = (syntax->argc == (word_num - 1));
					break;
				case SYNTAX_COMPARE_GT:
					compare = (syntax->argc <= (word_num - 1));
					break;
				}
				if(!compare){
					printf("%s parameter number not much %s\n", SYNTAX_ERROR_PREFIX, word[0]);
					return 1;
				}
			}
			for(j = 0; j < syntax->argc; j++){
				switch(syntax->argv_type[j]){
				case SYNTAX_ARGVTYPE_NULL:
					printf("%s ARGV_NULL select\n", SYNTAX_ERROR_PREFIX);
					return 1;
				case SYNTAX_ARGVTYPE_VALUE:
					if(value_get(word[j + 1], &(s->value[j])) == NG){
						printf("%s value error %s %s\n", SYNTAX_ERROR_PREFIX, word[0], word[j+1]);
						return 1;
					}
					break;
				case SYNTAX_ARGVTYPE_HV:
					switch(word[j + 1][0]){
					case 'H':
					case 'h':
						s->value[j] = MIRROR_HORIZONAL;
						break;
					case 'V':
					case 'v':
						s->value[j] = MIRROR_VERTICAL;
						break;
					case 'A':
					case 'a':
						s->value[j] = MIRROR_PROGRAMABLE;
						break;
					default:
						printf("%s unknown scroll mirroring type %s\n", SYNTAX_ERROR_PREFIX, word[j+1]);
						return 1;
					}
					break;
				case SYNTAX_ARGVTYPE_EXPRESSION:
					s->value[j] = VALUE_EXPRESSION;
					//命令名の単語と単語数を除外して渡す
					if(syntax_check_expression(&word[j+1], word_num - 2, &s->expression) == NG){
						printf("%s expression error\n", SYNTAX_ERROR_PREFIX);
						return 1;
					}
					//可変に引数を取るのでここで終わり
					return 0;
				case SYNTAX_ARGVTYPE_VARIABLE:{
					const char v = word[j+1][0];
					if((v >= 'a' && v <= 'z') || (v >= 'A' && v <= 'Z')){
						s->value[j] = VALUE_VARIABLE;
						s->variable = v;
					}else{
						printf("%s variable must use [A-Za-z] %s\n", SYNTAX_ERROR_PREFIX, word[j+1]);
						return 1;
					}
					}break;
				}
			}
			return 0;
		}
		syntax++;
		i--;
	}
	printf("%s unknown opcode %s\n", SYNTAX_ERROR_PREFIX, word[0]);
	return 1;
}

static int syntax_check(char **text, int text_num, struct script *s)
{
	int error = 0;
	int i;
	variable_init_all();
	for(i = 0; i < text_num; i++){
		char *word[TEXT_MAXWORD];
		const int n = word_load(text[i], word);
		if(word[0][0] == '#'){
			s->opcode = SCRIPT_OPCODE_COMMENT;
		}else{
			error += syntax_check_phase(word, n, s);
		}
		s++;
	}
	return error;
}

/*
logical_check() 用サブ関数とデータ
*/
static const char LOGICAL_ERROR_PREFIX[] = "logical error:";

static inline void logical_print_illgalarea(const char *area, long address)
{
	printf("%s illgal %s area $%06x\n", LOGICAL_ERROR_PREFIX, area, (int) address);
}

static inline void logical_print_illgallength(const char *area, long length)
{
	printf("%s illgal %s length $%04x\n", LOGICAL_ERROR_PREFIX, area, (int) length);
}

static inline void logical_print_overdump(const char *area, long start, long end)
{
	printf("%s %s area over dump $%06x-$%06x\n", LOGICAL_ERROR_PREFIX, area, (int)start ,(int)end);
}

static inline void logical_print_access(const char *area, const char *rw, long addr, long len)
{
	printf("%s %s $%04x $%02x\n", area, rw, (int) addr, (int) len);
}

static inline void logical_print_byteerror(const char *area, long data)
{
	printf("%s write data byte range over, %s $%x\n", LOGICAL_ERROR_PREFIX, area, (int) data);
}

static int dump_length_conform(const char *name, long logicallength, long configlength)
{
	if(configlength != logicallength){
		printf("%s %s dump length error\n", LOGICAL_ERROR_PREFIX, name);
		printf("%s: 0x%06x, dump length: 0x%06x\n", name, (int) configlength, (int) logicallength);
		return 1;
	}
	return 0;
}
static inline int is_region_cpurom(long address)
{
	return (address >= 0x8000) && (address < 0x10000);
}

static inline int is_region_cpuram(long address)
{
	return (address >= 0x6000) && (address < 0x8000);
}

static inline int is_region_ppurom(long address)
{
	return (address >= 0) && (address < 0x2000);
}

static inline int is_data_byte(long data)
{
	return (data >= 0) && (data < 0x100);
}

//これだけ is 系で <= 演算子を使用しているので注意
static inline int is_range(long data, long start, long end)
{
	return (data >= start) && (data <= end);
}
static const char STR_REGION_CPU[] = "cpu";
static const char STR_REGION_PPU[] = "ppu";
static const char STR_ACCESS_READ[] = "read";
static const char STR_ACCESS_WRITE[] = "write";

enum{
	SETTING, DUMP, END
};
static int logical_check(const struct script *s, struct romimage *r, const int test)
{
	long cpu_romsize = 0, cpu_ramsize = 0, ppu_romsize = 0;
	int setting = SETTING, error = 0;
	//romimage init
	variable_init_all();
	while(s->opcode != SCRIPT_OPCODE_DUMP_END){
		//printf("opcode exec %s\n", SCRIPT_SYNTAX[s->opcode].name);
		if((setting == DUMP) && (s->opcode < SCRIPT_OPCODE_DUMP_START)){
			printf("%s config script include DUMPSTART area\n", LOGICAL_ERROR_PREFIX);
			error += 1;
		}
		switch(s->opcode){
		case SCRIPT_OPCODE_COMMENT:
			break;
		case SCRIPT_OPCODE_MAPPER:
			r->mappernum = s->value[0];
			break;
		case SCRIPT_OPCODE_MIRROR:
			r->mirror = s->value[0];
			break;
		case SCRIPT_OPCODE_CPU_ROMSIZE:
			r->cpu_rom.size = s->value[0];
			break;
		case SCRIPT_OPCODE_CPU_RAMSIZE:
			r->cpu_ram_read.size = s->value[0];
			r->cpu_ram_write.size = s->value[0];
			break;
		case SCRIPT_OPCODE_PPU_ROMSIZE:
			r->ppu_rom.size = s->value[0];
			break;
		case SCRIPT_OPCODE_DUMP_START:
			setting = DUMP;
			break;
		case SCRIPT_OPCODE_CPU_READ:{
			const long address = s->value[0];
			const long length = s->value[1];
			const long end = address + length - 1;
			//length filter. 0 はだめ
			if(!is_range(length, 1, 0x4000)){
				logical_print_illgallength(STR_REGION_CPU, length);
				error += 1;
			}
			//address filter
			else if(address < 0x6000 || address >= 0x10000){
				logical_print_illgalarea(STR_REGION_CPU, address);
				error += 1;
			}else if(end >= 0x10000){
				logical_print_overdump(STR_REGION_CPU, address, end);
				error += 1;
			}
			//$7fff-$8000を連続でまたがせない
			else if((address <= 0x7fff) && (end >= 0x8000)){
				printf("%s address cannot over $7fff-$8000. divide CPU_READ.\n", LOGICAL_ERROR_PREFIX);
				error += 1;
			}
			//dump length update
			if(is_region_cpuram(address)){
				cpu_ramsize += length;
			}else if(is_region_cpurom(address)){
				cpu_romsize += length;
			}
			if(test == 1){
				logical_print_access(STR_REGION_CPU, STR_ACCESS_READ, address, length);
			}
			setting = DUMP;
			}
			break;
		case SCRIPT_OPCODE_CPU_WRITE:{
			const long address = s->value[0];
			long data;
			if(expression_calc(&s->expression, &data) == NG){
				printf("%s expression calc error\n", LOGICAL_ERROR_PREFIX);
				error += 1;
			}
			if(address < 0x5000 || address >= 0x10000){
				logical_print_illgalarea(STR_REGION_CPU, address);
				error += 1;
			}else if(!is_data_byte(data)){
				logical_print_byteerror(STR_REGION_CPU, data);
				error += 1;
			}
			if(test == 1){
				logical_print_access(STR_REGION_CPU, STR_ACCESS_WRITE, address, data);
			}
			setting = DUMP;
			}
			break;
		case SCRIPT_OPCODE_CPU_RAMRW:{
			if(r->mode == MODE_ROM_DUMP){
				printf("%s cannot use %s on ROMDUMP mode\n", LOGICAL_ERROR_PREFIX, OPSTR_CPU_RAMRW);
				error += 1;
			}
			const long address = s->value[0];
			const long length = s->value[1];
			const long end = address + length - 1;
			//length filter. 0 はだめ
			if(!is_range(length, 1, 0x2000)){
				logical_print_illgallength(STR_REGION_CPU, length);
				error += 1;
			}
			//address filter
			else if(address < 0x6000 || address >= 0x8000){
				logical_print_illgalarea(STR_REGION_CPU, address);
				error += 1;
			}else if(end >= 0x8000){
				logical_print_overdump(STR_REGION_CPU, address, end);
				error += 1;
			}
			cpu_ramsize += length;
			if(r->mode == MODE_RAM_WRITE){
				r->cpu_ram_write.data = buf_load_full(r->ramfile, &(r->cpu_ram_write.size));
				if(r->cpu_ram_write.data == NULL){
					printf("%s RAM image open error\n.", LOGICAL_ERROR_PREFIX);
					error += 1;
				}else if(r->cpu_ram_read.size != r->cpu_ram_write.size){
					printf("%s RAM image size is not same.\n", LOGICAL_ERROR_PREFIX);
					free(r->cpu_ram_write.data);
					r->cpu_ram_write.data = NULL;
					error += 1;
				}
			}
			setting = DUMP;
			}
			break;
		case SCRIPT_OPCODE_PPU_RAMTEST:
			//logical check ではなにもしない
			break;
		case SCRIPT_OPCODE_PPU_READ:{
			const long address = s->value[0];
			const long length = s->value[1];
			const long end = address + length - 1;
			//length filter. 0 を容認する
			if(!is_range(length, 0, 0x2000)){
				logical_print_illgallength(STR_REGION_PPU, length);
				error += 1;
			}
			//address filter
			else if(!is_region_ppurom(address)){
				logical_print_illgalarea(STR_REGION_PPU, address);
				error += 1;
			}else if (end >= 0x2000){
				logical_print_overdump(STR_REGION_PPU, address, end);
				error += 1;
			}
			//dump length update
			if(is_region_ppurom(address)){
				ppu_romsize += length;
			}
			if(test == 1){
				logical_print_access(STR_REGION_PPU, STR_ACCESS_READ, address, length);
			}
			setting = DUMP;
			}
			break;
		case SCRIPT_OPCODE_PPU_WRITE:{
			if(OP_PPU_WRITE_ENABLE==0){
				break;
			}
			const long address = s->value[0];
			const long data = s->value[1];
			setting = DUMP;
			if(!is_region_ppurom(address)){
				logical_print_illgalarea(STR_REGION_PPU, address);
				error += 1;
			}else if(!is_data_byte(data)){
				logical_print_byteerror(STR_REGION_PPU, data);
				error += 1;
			}
			if(test == 1){
				logical_print_access(STR_REGION_PPU, STR_ACCESS_WRITE, address, data);
			}
			}
			break;
		case SCRIPT_OPCODE_STEP_START:{
			int i;
			{
				const int v = s->value[1];
				if((v < 0) || (v > 0xff)){
					printf("%s step start must 0-0xff 0x%x\n", LOGICAL_ERROR_PREFIX, v);
					error += 1;
				}
			}
			for(i = 2; i <4; i++){
				const int v = s->value[i];
				if((v < 1) || (v > 0x100)){
					printf("%s end or next must 1-0x100 0x%x\n", LOGICAL_ERROR_PREFIX, v);
					error += 1;
				}
			}
			//ループの戻り先はこの命令の次なので s[1]
			if(step_new(s->variable, s->value[1], s->value[2], s->value[3], &s[1]) == NG){
				printf("%s step loop too much\n", LOGICAL_ERROR_PREFIX);
				error += 1;
				return error;
			}
			setting = DUMP;
			}break;
		case SCRIPT_OPCODE_DUMP_END:
			setting = END;
			break;
		}
		if(setting == END){
			break;
		}
		if(s->opcode == SCRIPT_OPCODE_STEP_END){
			if(variable_num == 0){
				printf("%s loop closed, missing STEP_START\n", LOGICAL_ERROR_PREFIX);
				return error + 1;
			}
			s = step_end(&s[1]);
			setting = DUMP;
		}else{
			s++;
		}
	}
	
	//loop open conform
	if(variable_num != 0){
		printf("%s loop opened, missing STEP_END\n", LOGICAL_ERROR_PREFIX);
		error += 1;
	}
	//dump length conform
	error += dump_length_conform(OPSTR_CPU_ROMSIZE, cpu_romsize, r->cpu_rom.size);
	error += dump_length_conform(OPSTR_CPU_RAMSIZE, cpu_ramsize, r->cpu_ram_read.size);
	error += dump_length_conform(OPSTR_PPU_ROMSIZE, ppu_romsize, r->ppu_rom.size);
	return error;
}

/*
execute() 用サブ関数とデータ
*/
enum {PPU_TEST_RAM, PPU_TEST_ROM};
const u8 PPU_TEST_DATA[] = "PPU_TEST_DATA";
/*static*/ int ppu_ramtest(void)
{
	const int length = sizeof(PPU_TEST_DATA);
	const long testaddr = 123;
	//ppu ram data fill 0
	{
		int i = length;
		long address = testaddr;
		while(i != 0){
			ppu_write(address++, 0);
			i--;
		}
	}
	
	//ppu test data write
	{
		const u8 *d;
		int i = length;
		long address = testaddr;
		d = PPU_TEST_DATA;
		while(i != 0){
			ppu_write(address++, (long) *d);
			d++;
			i--;
		}
	}

	u8 writedata[length];
	ppu_read(testaddr, length, writedata);
	if(memcmp(writedata, PPU_TEST_DATA, length) == 0){
		return PPU_TEST_RAM;
	}
	return PPU_TEST_ROM;
}

static void readbuffer_print(const struct memory *m, long length)
{
	if(length >= 0x10){
		length = 0x10;
	}
	printf("%s 0x%05x:", m->name, m->offset);
	int offset = 0;
	const u8 *data;
	data = m->data;
	while(length != 0){
		char safix;
		switch(offset & 0xf){
		default:
			safix = ' ';
			break;
		case 0x7:
			safix = '-';
			break;
		case 0xf:
			safix = ';';
			break;
		}
		printf("%02x%c", (int) *data, safix);
		data++;
		offset++;
		length--;
	}
}

static void checksum_print(const u8 *data, long length)
{
	int sum = 0;
	while(length != 0){
		sum += (int) *data;
		data++;
		length--;
	}
	printf(" 0x%06x\n", sum);
	fflush(stdout);
}

static void read_result_print(const struct memory *m, long length)
{
	readbuffer_print(m, length);
	checksum_print(m->data, length);
}

static void execute_cpu_ramrw(const struct memory *w, struct memory *r, int mode, long address, long length)
{
	if(mode == MODE_RAM_WRITE){
		const u8 *writedata;
		long a = address;
		long l = length;
		writedata = w->data;
		while(l != 0){
			cpu_write(a++, *writedata);
			writedata += 1;
			l--;
		}
	}
	cpu_read(address, length, r->data);
	if(mode == MODE_RAM_DUMP){
		return;
	}
	if(memcmp(r->data, w->data, length) == 0){
		printf("RAM data write success.\n");
	}else{
		printf("RAM data write failed.\n");
	}
}

static int execute(const struct script *s, struct romimage *r)
{
	const int gg = giveio_start();
	switch(gg){
	case GIVEIO_OPEN:
	case GIVEIO_START:
	case GIVEIO_WIN95:
		reader_init();
		break;
	default:
	case GIVEIO_ERROR:
		printf("execute error: Can't Access Direct IO %d\n", gg);
		return NG;
	}
	struct memory cpu_rom, ppu_rom, cpu_ram_read, cpu_ram_write;
	cpu_rom = r->cpu_rom;
	ppu_rom = r->ppu_rom;
	cpu_ram_read = r->cpu_ram_read;
	cpu_ram_write = r->cpu_ram_write;
	
	variable_init_all();
	while(s->opcode != SCRIPT_OPCODE_DUMP_END){
		int end = 1;
		switch(s->opcode){
		case SCRIPT_OPCODE_CPU_READ:{
			struct memory *m;
			const long addr = s->value[0];
			const long length = s->value[1];
			m = &cpu_rom;
			if(is_region_cpuram(addr)){
				m = &cpu_ram_read;
			}
			cpu_read(addr, length, m->data);
			read_result_print(m, length);
			m->data += length;
			m->offset += length;
			}break;
		case SCRIPT_OPCODE_CPU_WRITE:{
			long data;
			expression_calc(&s->expression, &data);
			cpu_write(s->value[0], data);
			}
			break;
		case SCRIPT_OPCODE_CPU_RAMRW:{
			const long length = s->value[1];
			execute_cpu_ramrw(&cpu_ram_write, &cpu_ram_read, r->mode, s->value[0], length);
			read_result_print(&cpu_ram_read, length);
			cpu_ram_read.data += length;
			cpu_ram_read.offset += length;
			if(r->mode == MODE_RAM_WRITE){
				cpu_ram_write.data += length;
				cpu_ram_write.offset += length;
			}
			}
			break;
		case SCRIPT_OPCODE_PPU_RAMTEST:
			if(ppu_ramtest() == PPU_TEST_RAM){
				printf("PPU_RAMTEST: charcter RAM found\n");
				r->ppu_rom.size = 0;
				end = 0;
			}
			break;
		case SCRIPT_OPCODE_PPU_READ:{
			const long address = s->value[0];
			const long length = s->value[1];
			if(length == 0){
				/*for mmc2,4 protect.
				このときは1byte読み込んで、その内容はバッファにいれない*/
				u8 dummy;
				ppu_read(address, 1, &dummy);
			}else{
				ppu_read(address, length, ppu_rom.data);
				read_result_print(&ppu_rom, length);
			}
			ppu_rom.data += length;
			ppu_rom.offset += length;
			}
			break;
		case SCRIPT_OPCODE_PPU_WRITE:
			if(OP_PPU_WRITE_ENABLE == 1){
				ppu_write(s->value[0], s->value[1]);
			}
			break;
		case SCRIPT_OPCODE_STEP_START:
			//ループの戻り先はこの命令の次なので &s[1]
			step_new(s->variable, s->value[1], s->value[2], s->value[3], &s[1]);
			break;
		case SCRIPT_OPCODE_DUMP_END:
			end = 0;
			break;
		}
		if(end == 0){
			break;
		}
		if(s->opcode == SCRIPT_OPCODE_STEP_END){
			s = step_end(++s);
		}else{
			s++;
		}
	}
	if(gg != GIVEIO_WIN95){
		giveio_stop(GIVEIO_STOP);
	}
	return OK;
}

void script_load(const char *inmode, const char *scriptfile, const char *targetfile, const int test_only)
{
	int mode;
	switch(inmode[0]){
	case 'd':
		mode = MODE_ROM_DUMP;
		break;
	case 'r':
		mode = MODE_RAM_DUMP;
		break;
	case 'w':
		mode = MODE_RAM_WRITE;
		break;
	default:
		printf("unknown mode %s\n", inmode);
		return;
	}
	
	struct script *s;
	{
		int scriptsize = 0;
		char *buf;
		
		buf = buf_load_full(scriptfile, &scriptsize);
		if(buf == NULL){
			printf("scriptfile open error\n");
			return;
		}
		char *text[TEXT_MAXLINE];
		const int text_num = text_load(buf, scriptsize, text);
		if(text_num == 0){
			printf("script line too much\n");
			free(buf);
			return;
		}
		s = malloc(sizeof(struct script) * (text_num + 1));
		//logical_check, execute 共に s->opcode が DUMP_END になるまで続ける。DUMP_END の入れ忘れ用に末尾のscriptに必ず DUMP_END をいれる
		{
			struct script *k;
			k = s;
			k += text_num;
			k->opcode = SCRIPT_OPCODE_DUMP_END;
		}
		const int error = syntax_check(text, text_num, s);
		free(buf);
		if(error != 0){
			free(s);
			return;
		}
	}
	struct romimage r = {
		cpu_rom: {
			size: 0, offset: 0,
			data: NULL,
			name: STR_REGION_CPU
		},
		ppu_rom: {
			size: 0, offset: 0,
			data: NULL,
			name: STR_REGION_PPU
		},
		cpu_ram_read: {
			size: 0, offset: 0,
			data: NULL,
			name: STR_REGION_CPU
		},
		cpu_ram_write: {
			size: 0, offset: 0,
			data: NULL,
			name: STR_REGION_CPU
		},
		neshead: NULL,
		mappernum: 0,
		mirror: MIRROR_PROGRAMABLE,
		mode: mode,
		ramfile: NULL
	};
	if(mode == MODE_RAM_WRITE){
		r.ramfile = targetfile;
	}
	if((logical_check(s, &r, test_only) == 0) && (test_only == 0)){
		//dump RAM 領域取得
		if(nesbuffer_malloc(&r) == NG){
			free(s);
			if(r.cpu_ram_write.data != NULL){
				free(r.cpu_ram_write.data);
			}
			return;
		}
		//dump
		if(execute(s, &r) == OK){
			//成果出力
			switch(r.mode){
			case MODE_ROM_DUMP:
				nesfile_create(&r, targetfile);
				break;
			case MODE_RAM_DUMP:
				backupram_create(&(r.cpu_ram_read), targetfile);
				break;
			}
		}
		//dump RAM 領域解放
		nesbuffer_free(&r);
		if(r.cpu_ram_write.data != NULL){
			free(r.cpu_ram_write.data);
		}
	}
	free(s);
}
