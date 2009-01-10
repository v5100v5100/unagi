/*
famicom ROM cartridge utility - unagi
script engine

Copyright (C) 2008 鰻開発協同組合

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

todo: 
* 変数管理のグローバル値を、logical_test(), excute() ローカルにしたい
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "file.h"
#include "reader_master.h"
#include "textutil.h"
#include "config.h"
#include "header.h"
#include "script.h"

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
#include "syntax.h"

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
	.name = '\0', 
	.start = 0, .end = 0, .step = 0,
	.val = 0,
	.Continue = NULL
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

/*
return: error count, ここでは 0 or 1
*/
static int syntax_check_phase(char **word, int word_num, struct script *s, const int mode)
{
	int i = sizeof(SCRIPT_SYNTAX) / sizeof(SCRIPT_SYNTAX[0]);
	const struct script_syntax *syntax;
	syntax = SCRIPT_SYNTAX;
	while(i != 0){
		if(strcmp(syntax->name, word[0]) == 0){
			int j;
			
			s->opcode = syntax->script_opcode;
			if((mode & syntax->permittion) == 0){
				printf("%s opcode %s not allowed on current mode\n", SYNTAX_ERROR_PREFIX, syntax->name);
				return 1;
			};
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
			//opcode found and 入力文字種正常
			return 0;
		}
		syntax++;
		i--;
	}
	printf("%s unknown opcode %s\n", SYNTAX_ERROR_PREFIX, word[0]);
	return 1;
}

/*
return: error count
*/
static int syntax_check(char **text, int text_num, struct script *s, int mode)
{
	int error = 0;
	int i;
	mode = 1<< mode; //permittion は bitflag なのでここで変換する
	variable_init_all();
	for(i = 0; i < text_num; i++){
		char *word[TEXT_MAXWORD];
		const int n = word_load(text[i], word);
		if(word[0][0] == '#'){
			s->opcode = SCRIPT_OPCODE_COMMENT;
		}else{
			error += syntax_check_phase(word, n, s, mode);
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
static int command_mask(const int region, const long address, const long offset, long size, struct flash_order *f)
{
	const char *str_region = STR_REGION_CPU;
	if(region == MEMORY_AREA_PPU){
		str_region = STR_REGION_PPU;
	}
	switch(region){
	case MEMORY_AREA_CPU_ROM:
		switch(offset){
		case 0x8000: case 0xa000: case 0xc000:
			break;
		default:
			printf("%s %s_COMMAND area offset error\n", LOGICAL_ERROR_PREFIX, str_region);
			return NG;
		}
		switch(size){
		case 0x2000: case 0x4000: case 0x8000:
			break;
		default:
			printf("%s %s_COMMAND area mask error\n", LOGICAL_ERROR_PREFIX, str_region);
			return NG;
		}
		break;
	case MEMORY_AREA_PPU:
		switch(offset){
		case 0x0000: case 0x0400: case 0x0800: case 0x0c00:
		case 0x1000: case 0x1400: case 0x1800: case 0x1c00:
			break;
		default:
			printf("%s %s_COMMAND area offset error\n", LOGICAL_ERROR_PREFIX, str_region);
			return NG;
		}
		switch(size){
		case 0x0400: case 0x0800: case 0x1000: case 0x2000: 
			break;
		default:
			printf("%s %s_COMMAND area mask error\n", LOGICAL_ERROR_PREFIX, str_region);
			return NG;
		}
		break;
	default:
		assert(0); //unknown memory area
	}

	const long mask = size - 1;
	const long data = (address & mask) | offset;
	switch(address){
	case 0:
		f->command_0000 = data;
		break;
	case 0x2aaa: case 0x02aa: 
		f->command_2aaa = data;
		break;
	case 0x5555: case 0x0555:
		f->command_5555 = data;
		break;
	default:
		printf("%s %s_COMMAND unknown commnand address\n", LOGICAL_ERROR_PREFIX, str_region);
		return NG;
	}
	return OK;
}

static int logical_check(const struct script *s, const struct st_config *c, struct romimage *r)
{
	long cpu_romsize = 0, cpu_ramsize = 0, ppu_romsize = 0;
	int imagesize = 0; //for write or program mode
	int setting = SETTING;
	int error = 0;
	
	variable_init_all();
	while(s->opcode != SCRIPT_OPCODE_DUMP_END){
		//printf("opcode exec %s\n", SCRIPT_SYNTAX[s->opcode].name);
		if((setting == DUMP) && (s->opcode < SCRIPT_OPCODE_DUMP_START)){
			printf("%s config script include DUMPSTART area\n", LOGICAL_ERROR_PREFIX);
			error += 1;
		}

		//romimage open for write or program mode
		if((imagesize == 0) && (setting == DUMP)){
			switch(c->mode){
			case MODE_RAM_WRITE:
				assert(r->cpu_ram.attribute == MEMORY_ATTR_READ);
				r->cpu_ram.data = buf_load_full(c->ramimage, &imagesize);
				if(r->cpu_ram.data == NULL){
					printf("%s RAM image open error\n", LOGICAL_ERROR_PREFIX);
					imagesize = -1;
					error += 1;
				}else if(r->cpu_ram.size != imagesize){
					printf("%s RAM image size is not same\n", LOGICAL_ERROR_PREFIX);
					free(r->cpu_ram.data);
					r->cpu_ram.data = NULL;
					imagesize = -1;
					error += 1;
				}
				break;
			case MODE_ROM_PROGRAM:
				assert(c->cpu_flash_driver->write != NULL);
				assert(r->cpu_rom.attribute == MEMORY_ATTR_READ);
				assert(r->ppu_rom.attribute == MEMORY_ATTR_READ);
				if(nesfile_load(LOGICAL_ERROR_PREFIX, c->romimage, r)== NG){
					error += 1;
				}
				imagesize = -1;
				break;
			default: 
				imagesize = -1;
				break;
			}
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
		case SCRIPT_OPCODE_CPU_ROMSIZE:{
			const long size = s->value[0];
			r->cpu_rom.size = size;
			if(memorysize_check(size, MEMORY_AREA_CPU_ROM)){
				printf("%s %s length error\n", LOGICAL_ERROR_PREFIX, OPSTR_CPU_ROMSIZE);
				error += 1;
			}
			//flash memory capacity check
			//いまのところ == にして小さい容量もそのうち対応
			else if((c->mode == MODE_ROM_PROGRAM) && (size > c->cpu_flash_driver->capacity)){
				printf("%s flash memory capacity error\n", LOGICAL_ERROR_PREFIX);
				error += 1;
			}
			}break;
		case SCRIPT_OPCODE_CPU_RAMSIZE:
			//memory size は未確定要素が多いので check を抜く
			r->cpu_ram.size = s->value[0];
			break;
		case SCRIPT_OPCODE_CPU_COMMAND:
			if(command_mask(MEMORY_AREA_CPU_ROM, s->value[0], s->value[1], s->value[2], &(r->cpu_flash)) == NG){
				error += 1;
			}
			break;
		case SCRIPT_OPCODE_PPU_ROMSIZE:{
			const long size = s->value[0];
			r->ppu_rom.size = size;
			if(memorysize_check(size, MEMORY_AREA_PPU)){
				printf("%s %s length error\n", LOGICAL_ERROR_PREFIX, OPSTR_PPU_ROMSIZE);
				error += 1;
			}
			else if((c->mode == MODE_ROM_PROGRAM) && (size > c->ppu_flash_driver->capacity)){
				printf("%s flash memory capacity error\n", LOGICAL_ERROR_PREFIX);
				error += 1;
			}
			}
			break;
		case SCRIPT_OPCODE_PPU_COMMAND:
			if(command_mask(MEMORY_AREA_PPU, s->value[0], s->value[1], s->value[2], &(r->ppu_flash)) == NG){
				error += 1;
			}
			break;
		case SCRIPT_OPCODE_DUMP_START:
			setting = DUMP;
			break;
		case SCRIPT_OPCODE_CPU_READ:{
			const long address = s->value[0];
			const long length = s->value[1];
			const long end = address + length - 1;
			
			assert(r->cpu_rom.attribute == MEMORY_ATTR_WRITE);
			//length filter. 0 はだめ
			if(!is_range(length, 1, 0x4000)){
				logical_print_illgallength(STR_REGION_CPU, length);
				error += 1;
			}
			//address filter
			else if(!is_region_cpurom(address)){
				logical_print_illgalarea(STR_REGION_CPU, address);
				error += 1;
			}else if(end >= 0x10000){
				logical_print_overdump(STR_REGION_CPU, address, end);
				error += 1;
			}
			cpu_romsize += length;
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
			setting = DUMP;
			}
			break;
		case SCRIPT_OPCODE_CPU_RAMRW:{
			const long address = s->value[0];
			const long length = s->value[1];
			const long end = address + length - 1;
			switch(c->mode){
			case MODE_RAM_READ:
				assert(r->cpu_ram.attribute == MEMORY_ATTR_WRITE);
				break;
			case MODE_RAM_WRITE:
				assert(r->cpu_ram.attribute = MEMORY_ATTR_READ);
				break;
			}
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
			setting = DUMP;
			}
			break;
		case SCRIPT_OPCODE_CPU_PROGRAM:{
			const long address = s->value[0];
			const long length = s->value[1];
			const long end = address + length - 1;
			
			assert(r->cpu_rom.attribute == MEMORY_ATTR_READ);
			assert(r->ppu_rom.attribute == MEMORY_ATTR_READ);
			//length filter.
			if(!is_range(length, 0x80, 0x2000)){
				logical_print_illgallength(STR_REGION_CPU, length);
				error += 1;
			}
			//address filter
			else if(!is_region_cpurom(address)){
				logical_print_illgalarea(STR_REGION_CPU, address);
				error += 1;
			}else if(end >= 0x10000){
				logical_print_overdump(STR_REGION_CPU, address, end);
				error += 1;
			}
			cpu_romsize += length;
			setting = DUMP;
			}
			break;
		case SCRIPT_OPCODE_PPU_RAMFIND:
			//ループ内部に入ってたらエラー
			if(variable_num != 0){
				printf("%s PPU_RAMTEST must use outside loop\n", LOGICAL_ERROR_PREFIX);
				error += 1;
			}
			break;
		case SCRIPT_OPCODE_PPU_SRAMTEST:
		case SCRIPT_OPCODE_PPU_READ:{
			const long address = s->value[0];
			const long length = s->value[1];
			const long end = address + length - 1;
			assert(r->ppu_rom.attribute == MEMORY_ATTR_WRITE);
			//length filter. 0 を容認する
			long min = 0;
			if(s->opcode == SCRIPT_OPCODE_PPU_SRAMTEST){
				min = 1;
			}
			if(!is_range(length, min, 0x2000)){
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
			if((s->opcode == SCRIPT_OPCODE_PPU_READ) && is_region_ppurom(address)){
				ppu_romsize += length;
			}
			setting = DUMP;
			}
			break;
		case SCRIPT_OPCODE_PPU_WRITE:{
			if(DEBUG==0){
				break;
			}
			const long address = s->value[0];
			long data;
			if(expression_calc(&s->expression, &data) == NG){
				printf("%s expression calc error\n", LOGICAL_ERROR_PREFIX);
				error += 1;
			}
			setting = DUMP;
			if(!is_region_ppurom(address)){
				logical_print_illgalarea(STR_REGION_PPU, address);
				error += 1;
			}else if(!is_data_byte(data)){
				logical_print_byteerror(STR_REGION_PPU, data);
				error += 1;
			}
			setting = DUMP;
			}
			break;
		case SCRIPT_OPCODE_PPU_PROGRAM:{
			const long address = s->value[0];
			const long length = s->value[1];
			const long end = address + length - 1;
			
			assert(r->ppu_rom.attribute == MEMORY_ATTR_READ);
			//length filter.
			if(!is_range(length, 0x80, 0x1000)){
				logical_print_illgallength(STR_REGION_PPU, length);
				error += 1;
			}
			//address filter
			else if(!is_region_ppurom(address)){
				logical_print_illgalarea(STR_REGION_PPU, address);
				error += 1;
			}else if(end >= 0x2000){
				logical_print_overdump(STR_REGION_PPU, address, end);
				error += 1;
			}
			ppu_romsize += length;
			setting = DUMP;
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
	error += dump_length_conform(OPSTR_CPU_RAMSIZE, cpu_ramsize, r->cpu_ram.size);
	error += dump_length_conform(OPSTR_PPU_ROMSIZE, ppu_romsize, r->ppu_rom.size);
	
	//command line config override
	if(c->mirror != CONFIG_OVERRIDE_UNDEF){
		r->mirror = c->mirror;
	}
	if(c->backupram != CONFIG_OVERRIDE_UNDEF){
		r->backupram = 1;
	}
	if(c->mapper != CONFIG_OVERRIDE_UNDEF){
		//program mode で mapper 変更を防ぐ
		assert(c->mode == MODE_ROM_DUMP);
		r->mappernum = c->mapper;
	}
	if(c->syntaxtest == 1){
		if(error == 0){
			printf("syntax ok!\n");
		}
		error += 1;
	}
	return error;
}

/*
execute() 用サブ関数とデータ
*/
static int execute_connection_check(const struct reader_driver *d)
{
	int ret = OK;
	const int testsize = 0x80;
	int testcount = 3;
	u8 *master, *reload;
	master = malloc(testsize);
	reload = malloc(testsize);

	d->cpu_read(0xfee0, testsize, master);
	
	while(testcount != 0){
		d->cpu_read(0xfee0, testsize, reload);
		if(memcmp(master, reload, testsize) != 0){
			ret = NG;
			break;
		}
		testcount--;
	}
	
	free(master);
	free(reload);
	return ret;
}

enum {PPU_TEST_RAM, PPU_TEST_ROM};
const u8 PPU_TEST_DATA[] = "PPU_TEST_DATA";
static int ppu_ramfind(const struct reader_driver *d)
{
	const int length = sizeof(PPU_TEST_DATA);
	const long testaddr = 123;
	//ppu ram data fill 0
	{
		int i = length;
		long address = testaddr;
		while(i != 0){
			d->ppu_write(address++, 0);
			i--;
		}
	}
	
	//ppu test data write
	{
		const u8 *data;
		int i = length;
		long address = testaddr;
		data = PPU_TEST_DATA;
		while(i != 0){
			d->ppu_write(address++, (long) *data);
			data++;
			i--;
		}
	}

	u8 writedata[length];
	d->ppu_read(testaddr, length, writedata);
	if(memcmp(writedata, PPU_TEST_DATA, length) == 0){
		return PPU_TEST_RAM;
	}
	return PPU_TEST_ROM;
}

static int ramtest(const int region, const struct reader_driver *d, long address, long length, u8 *writedata, u8 *testdata, const long filldata)
{
	long i = length;
	long a = address;
	while(i != 0){
		switch(region){
		case MEMORY_AREA_CPU_RAM:
			d->cpu_6502_write(a, filldata, 0);
			break;
		case MEMORY_AREA_PPU:
			d->ppu_write(a, filldata);
			break;
		default:
			assert(0);
		}
		a++;
		i--;
	}
	switch(region){
	case MEMORY_AREA_CPU_RAM:
		d->cpu_read(address, length, testdata);
		break;
	case MEMORY_AREA_PPU:
		d->ppu_read(address, length, testdata);
		break;
	default:
		assert(0);
	}
	memset(writedata, filldata, length);
	if(memcmp(writedata, testdata, length) == 0){
		return 0;
	}
	return 1;
}

static const long SRAMTESTDATA[] = {0xff, 0xaa, 0x55, 0x00};
static int sramtest(const int region, const struct reader_driver *d, long address, long length)
{
	u8 *writedata, *testdata;
	int error = 0;
	int i;
	testdata = malloc(length);
	writedata = malloc(length);
	for(i = 0; i < sizeof(SRAMTESTDATA) / sizeof(long); i++){
		const long filldata = SRAMTESTDATA[i];
		error += ramtest(region, d, address, length, testdata, writedata, filldata);
	}
	free(testdata);
	free(writedata);
	return error;
}

static void readbuffer_print(const struct memory *m, long length)
{
	if(length >= 0x10){
		length = 0x10;
	}
	printf("%s ROM 0x%05x:", m->name, m->offset);
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
}

static void read_result_print(const struct memory *m, long length)
{
	readbuffer_print(m, length);
	checksum_print(m->data, length);
	fflush(stdout);
}

static void execute_program_begin(const struct memory *m)
{
	if(0){ //DEBUG==1){
		return;
	}
	printf("writing %s area 0x%06x ... ", m->name, m->offset);
	fflush(stdout);
}

static const char STR_OK[] = "OK";
static const char STR_NG[] = "NG";

//memcmp の戻り値が入るので 0 が正常
static void execute_program_finish(int result)
{
	const char *str;
	str = STR_NG;
	if(result == 0){
		str = STR_OK;
	}
	printf("%s\n", str);
	fflush(stdout);
}
static const char EXECUTE_ERROR_PREFIX[] = "execute error:";
static const char EXECUTE_PROGRAM_PREPARE[] = "%s device initialize ... ";
static const char EXECUTE_PROGRAM_DONE[] = "done\n";
static void execute_cpu_ramrw(const struct reader_driver *d, const struct memory *ram, int mode, long address, long length, long wait)
{
	if(mode == MODE_RAM_WRITE){
		const u8 *writedata;
		long a = address;
		long l = length;
		writedata = ram->data;
		while(l != 0){
			d->cpu_6502_write(a++, *writedata, wait);
			writedata += 1;
			l--;
		}
		u8 *compare;
		compare = malloc(length);
		d->cpu_read(address, length, compare);
		if(memcmp(ram->data, compare, length) == 0){
			printf("RAM data write success\n");
		}else{
			printf("RAM data write failed\n");
		}
		free(compare);
	}else{
		d->cpu_read(address, length, ram->data);
	}
}

static int execute(const struct script *s, const struct st_config *c, struct romimage *r)
{
	const struct reader_driver *const d = c->reader;
	switch(d->open_or_close(READER_OPEN)){
	case OK:
		d->init();
		break;
	case NG:
		printf("%s driver open error\n", EXECUTE_ERROR_PREFIX);
		return NG;
	default:
		assert(0);
	}
	if(execute_connection_check(d) == NG){
		printf("%s maybe connection error\n", EXECUTE_ERROR_PREFIX);
		d->open_or_close(READER_CLOSE);
		return NG;
	}
	u8 *program_compare;
	program_compare = NULL;
	if(c->mode == MODE_ROM_PROGRAM){
		printf("flashmemory/SRAM program mode. To abort programming, press Ctrl+C\n");
		if(r->ppu_rom.size != 0){
			c->ppu_flash_driver->init(&(r->ppu_flash));
		}
		int size = r->cpu_rom.size;
		if(size < r->ppu_rom.size){
			size = r->ppu_rom.size;
		}
		program_compare = malloc(size);
	}
	struct memory cpu_rom, ppu_rom, cpu_ram;
	cpu_rom = r->cpu_rom;
	ppu_rom = r->ppu_rom;
	cpu_ram = r->cpu_ram;
	
	int programcount_cpu = 0, programcount_ppu = 0;
	variable_init_all();
	while(s->opcode != SCRIPT_OPCODE_DUMP_END){
		int end = 1;
		switch(s->opcode){
		case SCRIPT_OPCODE_CPU_READ:{
			struct memory *m;
			const long address = s->value[0];
			const long length = s->value[1];
			m = &cpu_rom;
			d->cpu_read(address, length, m->data);
			read_result_print(m, length);
			m->data += length;
			m->offset += length;
			}break;
		case SCRIPT_OPCODE_CPU_WRITE:{
			long data;
			expression_calc(&s->expression, &data);
			d->cpu_6502_write(s->value[0], data, c->write_wait);
			}
			break;
		case SCRIPT_OPCODE_CPU_RAMRW:{
			const long address = s->value[0];
			const long length = s->value[1];
			if(c->mode == MODE_RAM_WRITE){
				if(sramtest(MEMORY_AREA_CPU_RAM, d, address, length) != 0){
					printf("SRAM test NG\n");
					end = 0;
					break;
				}
			}
			execute_cpu_ramrw(d, &cpu_ram, c->mode, address, length, c->write_wait);
			read_result_print(&cpu_ram, length);
			cpu_ram.data += length;
			cpu_ram.offset += length;
			}
			break;
		case SCRIPT_OPCODE_CPU_PROGRAM:{
			if(c->cpu_flash_driver->id_device == FLASH_ID_DEVICE_DUMMY){
				break;
			}
			if(programcount_cpu++ == 0){
				printf(EXECUTE_PROGRAM_PREPARE, cpu_rom.name);
				fflush(stdout);
				//device によっては erase
				c->cpu_flash_driver->init(&(r->cpu_flash));
				printf(EXECUTE_PROGRAM_DONE);
				fflush(stdout);
			}
			const long address = s->value[0];
			const long length = s->value[1];
			execute_program_begin(&cpu_rom);
			c->cpu_flash_driver->write(
				&(r->cpu_flash),
				address, length,
				&cpu_rom
			);
			d->cpu_read(address, length, program_compare);
			const int result = memcmp(program_compare, cpu_rom.data, length);
			execute_program_finish(result);
			cpu_rom.data += length;
			cpu_rom.offset += length;
			
			if((DEBUG==0) && (result != 0)){
				end = 0;
			}
			}
			break;
		case SCRIPT_OPCODE_PPU_RAMFIND:
			if(ppu_ramfind(d) == PPU_TEST_RAM){
				printf("PPU_RAMFIND: charcter RAM found\n");
				r->ppu_rom.size = 0;
				end = 0;
			}
			break;
		case SCRIPT_OPCODE_PPU_SRAMTEST:{
			const long address = s->value[0];
			const long length = s->value[1];
			printf("PPU_SRAMTEST: 0x%06x-0x%06x ", (int)ppu_rom.offset, (int) (ppu_rom.offset + length) - 1);
			if(sramtest(MEMORY_AREA_PPU, d, address, length) == 0){
				printf("%s\n", STR_OK);
			}else{
				printf("%s\n", STR_NG);
				//end = 0;
			}
			}break;
		case SCRIPT_OPCODE_PPU_READ:{
			const long address = s->value[0];
			const long length = s->value[1];
			if(length == 0){
				/*for mmc2,4 protect.
				このときは1byte読み込んで、その内容はバッファにいれない*/
				u8 dummy;
				d->ppu_read(address, 1, &dummy);
			}else{
				d->ppu_read(address, length, ppu_rom.data);
				read_result_print(&ppu_rom, length);
			}
			ppu_rom.data += length;
			ppu_rom.offset += length;
			}
			break;
		case SCRIPT_OPCODE_PPU_WRITE:
			if(DEBUG == 1){
				long data;
				expression_calc(&s->expression, &data);
				d->ppu_write(s->value[0], data);
			}
			break;
		case SCRIPT_OPCODE_PPU_PROGRAM:{
			if(c->ppu_flash_driver->id_device == FLASH_ID_DEVICE_DUMMY){
				break;
			}
			if(programcount_ppu++ == 0){
				printf(EXECUTE_PROGRAM_PREPARE, ppu_rom.name);
				fflush(stdout);
				c->ppu_flash_driver->init(&(r->ppu_flash));
				printf(EXECUTE_PROGRAM_DONE);
				fflush(stdout);
			}
			const long address = s->value[0];
			const long length = s->value[1];
			execute_program_begin(&ppu_rom);
			c->ppu_flash_driver->write(
				&(r->ppu_flash),
				address, length,
				&ppu_rom
			);
			d->ppu_read(address, length, program_compare);
			const int result = memcmp(program_compare, ppu_rom.data, length);
			execute_program_finish(result);
			ppu_rom.data += length;
			ppu_rom.offset += length;
			
			if((DEBUG==0) && (result != 0)){
				end = 0;
			}
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
	d->open_or_close(READER_CLOSE);
	if(program_compare != NULL){
		free(program_compare);
	}
	return OK;
}

void script_load(const struct st_config *c)
{
	struct script *s;
	{
		int scriptsize = 0;
		char *buf;
		
		buf = buf_load_full(c->script, &scriptsize);
		if(buf == NULL){
			printf("scriptfile open error\n");
			return;
		}
		char **text;
		text = malloc(sizeof(char*) * TEXT_MAXLINE);
		const int text_num = text_load(buf, scriptsize, text);
		if(text_num == 0){
			printf("script line too much\n");
			free(buf);
			free(text);
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
		const int error = syntax_check(text, text_num, s, c->mode);
		free(buf);
		free(text);
		if(error != 0){
			free(s);
			return;
		}
	}
	struct romimage r = {
		.cpu_rom = {
			.size = 0, .offset = 0,
			.data = NULL,
			.attribute = MEMORY_ATTR_NOTUSE,
			.name = "program"
		},
		.ppu_rom = {
			.size = 0, .offset = 0,
			.data = NULL,
			.attribute = MEMORY_ATTR_NOTUSE,
			.name = "charcter"
		},
		.cpu_ram = {
			.size = 0, .offset = 0,
			.data = NULL,
			.attribute = MEMORY_ATTR_NOTUSE,
			.name = STR_REGION_CPU
		},
		//device に応じた関数ポインタを flash_order に渡す
		.cpu_flash = {
			.command_0000 = 0,
			.command_2aaa = 0,
			.command_5555 = 0,
			.pagesize = c->cpu_flash_driver->pagesize,
			.erase_wait = c->cpu_flash_driver->erase_wait,
			.command_mask = c->cpu_flash_driver->command_mask,
			.flash_write = c->reader->cpu_flash_write,
			.read = c->reader->cpu_read
		},
		.ppu_flash = {
			.command_0000 = 0,
			.command_2aaa = 0,
			.command_5555 = 0,
			.pagesize = c->ppu_flash_driver->pagesize,
			.erase_wait = c->ppu_flash_driver->erase_wait,
			.command_mask = c->ppu_flash_driver->command_mask,
			.flash_write = c->reader->ppu_write,
			.read = c->reader->ppu_read
		},
		.mappernum = 0,
		.mirror = MIRROR_PROGRAMABLE
	};
	//attribute はその struct data に対しての RW なので要注意
	switch(c->mode){
	case MODE_ROM_DUMP:
		r.cpu_rom.attribute = MEMORY_ATTR_WRITE;
		r.ppu_rom.attribute = MEMORY_ATTR_WRITE;
		break;
	case MODE_RAM_READ:
		r.cpu_ram.attribute = MEMORY_ATTR_WRITE;
		break;
	case MODE_RAM_WRITE:
		r.cpu_ram.attribute = MEMORY_ATTR_READ;
		break;
	case MODE_ROM_PROGRAM:
		r.cpu_rom.attribute = MEMORY_ATTR_READ;
		r.ppu_rom.attribute = MEMORY_ATTR_READ;
		break;
	default:
		assert(0);
	}
	
	if(logical_check(s, c, &r) == 0){
		//dump RAM 領域取得
		if(nesbuffer_malloc(&r, c->mode) == NG){
			free(s);
			if((c->mode == MODE_RAM_WRITE) && (r.cpu_ram.data != NULL)){
				free(r.cpu_ram.data);
			}
			return;
		}
		//script execute!!
		if(execute(s, c, &r) == OK){
			//成果出力
			switch(c->mode){
			case MODE_ROM_DUMP:
				nesfile_create(&r, c->romimage);
				break;
			case MODE_RAM_READ:
				backupram_create(&(r.cpu_ram), c->ramimage);
				break;
			}
		}
		//dump RAM 領域解放
		nesbuffer_free(&r, c->mode);
		if((c->mode == MODE_RAM_WRITE) && (r.cpu_ram.data != NULL)){
			free(r.cpu_ram.data);
		}
	}
	free(s);
}
