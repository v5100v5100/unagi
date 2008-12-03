/*
famicom ROM cartridge utility - unagi
command line interface, config file pharser

Copyright (C) 2008  sato_tiff

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
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "reader_master.h"
#include "giveio.h"
#include "file.h"
#include "script.h"
#include "header.h"
#include "textutil.h"
#include "config.h"
#include "flashmemory.h"
#include "client_test.h"

static int flag_get(const char *flag, struct st_config *c)
{
	while(*flag != '\0'){
		switch(*flag){
		case 'S': case 's':
			c->backupram = CONFIG_OVERRIDE_TRUE;
			break;
		case 'H': case 'h':
			c->mirror = MIRROR_HORIZONAL;
			break;
		case 'V': case 'v':
			c->mirror = MIRROR_VERTICAL;
			break;
		case '_':
			break;
		default:
			return NG;
		}
		flag++;
	}
	return OK;
}

static const char PREFIX_CONFIG_ERROR[] = "config error:";
static int config_file_load(struct st_config *c)
{
	char *buf;
	int size = 0;
	c->reader = NULL;
	buf = buf_load_full("unagi.cfg", &size);
	if(buf == NULL){
		printf("%s config file open error\n", PREFIX_CONFIG_ERROR);
		return NG;
	}
	char **text;
	text = malloc(sizeof(char*) * TEXT_MAXLINE);
	const int text_num = text_load(buf, size, text);
	if(text_num == 0){
		printf("%s script line too much\n", PREFIX_CONFIG_ERROR);
		free(buf);
		free(text);
		return NG;
	}
	int i;
	for(i=0; i<text_num; i++){
		char *word[TEXT_MAXWORD];
		word_load(text[i], word);
		if(word[0][0] == '#'){
			continue;
		}
		if(strcmp("DRIVER", word[0]) == 0){
			c->reader = reader_driver_get(word[1]);
			break;
		}else{
			printf("%s unknown config title %s", PREFIX_CONFIG_ERROR, word[1]);
			free(buf);
			free(text);
			return NG;
		}
	}
	
	free(text);
	if(c->reader == NULL){
		printf("%s hardware not selected or not found\n", PREFIX_CONFIG_ERROR);
		free(buf);
		return NG;
	}
	free(buf);
	return OK;
}

static int flash_pointer_init(const char *device, const struct flash_driver **f)
{
	*f = flash_driver_get(device);
	if(*f == NULL){
		printf("%s unknown flash device %s\n", PREFIX_CONFIG_ERROR, device);
		return NG;
	}
	return OK;
}
enum{
	ARGC_MODE = 1,
	ARGC_SCRIPTFILE,
	ARGC_DUMP_OUT_NESFILE,
	ARGC_DUMP_FLAG,
	ARGC_DUMP_MAPPER,
	ARGC_READ_OUT_RAMFILE = ARGC_DUMP_OUT_NESFILE,
	ARGC_WRITE_IN_RAMFILE = ARGC_DUMP_OUT_NESFILE,
	ARGC_PROGRAM_IN_NESFILE = ARGC_DUMP_OUT_NESFILE,
	ARGC_PROGRAM_CPU_DEVICE,
	ARGC_PROGRAM_PPU_DEVICE
};

static int config_init(const int argc, const char **argv, struct st_config *c)
{
	c->script = argv[ARGC_SCRIPTFILE];
	c->romimage = NULL;
	c->ramimage_read = NULL;
	c->ramimage_write = NULL;
	c->mapper = CONFIG_OVERRIDE_UNDEF;
	c->mirror = CONFIG_OVERRIDE_UNDEF;
	c->backupram = CONFIG_OVERRIDE_UNDEF;
	c->mapper = CONFIG_OVERRIDE_UNDEF;
	//mode 別 target file 初期化
	switch(argv[ARGC_MODE][0]){
	case 'd':
		c->mode = MODE_ROM_DUMP;
		c->romimage = argv[ARGC_DUMP_OUT_NESFILE];
		break;
	case 'r':
		c->mode = MODE_RAM_READ;
		c->ramimage_read = argv[ARGC_READ_OUT_RAMFILE];
		break;
	case 'w':
		c->mode = MODE_RAM_WRITE;
		c->ramimage_write = argv[ARGC_WRITE_IN_RAMFILE];
		break;
	case 'f':
		c->mode = MODE_ROM_PROGRAM;
		c->romimage = argv[ARGC_PROGRAM_IN_NESFILE];
		break;
	default:
		printf("%s unkown mode %s\n", PREFIX_CONFIG_ERROR, argv[ARGC_MODE]);
		return NG;
	};
	//mode 別 argc check. ここに来る argc は 4.5.6 が保証されている
	switch(c->mode){
	case MODE_ROM_DUMP:{
		int flag_error = OK, mapper_error = OK;
		switch(argc){
		case 5:
			flag_error = flag_get(argv[ARGC_DUMP_FLAG], c);
			break;
		case 6:
			flag_error = flag_get(argv[ARGC_DUMP_FLAG], c);
			mapper_error = value_get(argv[ARGC_DUMP_MAPPER], &c->mapper);
			break;
		}
		if(flag_error != OK){
			printf("%s unknown flag %s\n", PREFIX_CONFIG_ERROR, argv[ARGC_DUMP_FLAG]);
			return NG;
		}
		if(mapper_error != OK){
			printf("%s unknown mapper %s\n", PREFIX_CONFIG_ERROR, argv[ARGC_DUMP_MAPPER]);
			return NG;
		}
		}break;
	case MODE_RAM_READ:
	case MODE_RAM_WRITE:
		if(argc != 4){
			printf("%s too many argument\n", PREFIX_CONFIG_ERROR);
			return NG;
		}
		break;
	case MODE_ROM_PROGRAM:
		switch(argc){
		case 4:
			printf("%s few argument\n", PREFIX_CONFIG_ERROR);
			return NG;
		case 5:
			if(flash_pointer_init(argv[ARGC_PROGRAM_CPU_DEVICE], &(c->cpu_flash_driver)) == NG){
				return NG;
			}
			c->ppu_flash_driver = NULL;
			break;
		case 6:
			if(flash_pointer_init(argv[ARGC_PROGRAM_CPU_DEVICE], &(c->cpu_flash_driver)) == NG){
				return NG;
			}
			if(flash_pointer_init(argv[ARGC_PROGRAM_PPU_DEVICE], &(c->ppu_flash_driver)) == NG){
				return NG;
			}
			break;
		}
		break;
	}

	if(config_file_load(c) == NG){
		return NG;
	}
	return OK;
}

int main(int c, char **v)
{
	struct st_config config;
	int config_result;
	switch(c){
	case 3:
		if(DEBUG==1){
			test(v[1], v[2]);
		}else{
			goto usage;
		}
		return 0;
	case 4: //mode script target
	case 5:
		//mode script target flag
		//mode script target cpu_flash_device
	case 6:
		//mode script target flag mapper
		//mode script target cpu_flash_device ppu_flash_device
		config_result = config_init(c, (const char **) v, &config);
		break;
	usage:
	default:
		printf("famicom ROM cartridge utility - unagi version 0.5.2\n");
		printf("%s [mode] [mapper script] [target file] [flag]\n", v[0]);
		printf("mode - [d]ump ROM / [r]ead RAM/ [w]rite RAM\n");
		return 0;
	}
	if(config_result == OK){
		script_load(&config);
	}
	return 0;
}
