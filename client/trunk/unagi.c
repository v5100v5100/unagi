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
#include "driver_master.h"
#include "flashmemory.h"
#include "giveio.h"
#include "file.h"
#include "script.h"
#include "header.h"
#include "textutil.h"

static void test(const char *drivername, const char *file)
{
	const struct reader_driver *d;
	d = reader_driver_get(drivername);
	if(d == NULL){
		printf("%s: reader driver not found\n", __FUNCTION__);
		return;
	}
	const int gg = giveio_start();
	switch(gg){
	case GIVEIO_OPEN:
	case GIVEIO_START:
	case GIVEIO_WIN95:
		d->init();
		break;
	default:
	case GIVEIO_ERROR:
		printf("Can't Access Direct IO %d\n", gg);
		return;
	}

	switch(file[0]){
	case 'p':
		//printf("%d\n", ppu_ramtest(d));
		break;
	case 'b':{
		const int testbufsize = 0x100;
		u8 testbuf[testbufsize];
		int i;
		d->cpu_read(0x6000, testbufsize, testbuf);
		for(i=0;i<0x10;i++){
			printf("%02x ", testbuf[i]);
		}
		}break;
	case 'f':{
		
		}
		break;
	case 'e':{
		const struct flash_driver *f;
		const long address_2aaa = 0x8000 + 0x2aaa;
		const long address_5555 = 0x8000 + 0x5000;
		f = flash_driver_get("W49F002");
		if(f == NULL){
			printf("%s: flash driver not found\n", __FUNCTION__);
			break;
		}
		d->cpu_6502_write(0x8000, 0);
		d->cpu_6502_write(0xc000, 2);
		if(f->productid_check(d, f, 0, address_2aaa, address_5555) == NG){
			printf("product id error\n");
			break;
		}
		f->erase(d, address_2aaa, address_5555);
		}
		break;
	}
	
	if(gg != GIVEIO_WIN95){
		giveio_stop(GIVEIO_STOP);
	}
	return;
}

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
	c->driver[0] = '\0';
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
			strncpy(c->driver, word[1], 20);
		}else{
			printf("%s unknown config title %s", PREFIX_CONFIG_ERROR, word[1]);
			free(buf);
			free(text);
			return NG;
		}
	}
	free(text);
	if(c->driver[0] == '\0'){
		printf("%s hardware not selected\n", PREFIX_CONFIG_ERROR);
		free(buf);
		return NG;
	}
	free(buf);
	return OK;
}

static int config_init(int argc, const char *mode, const char *script, const char *file, const char *flag, const char *mapper, struct st_config *c)
{
	c->romimage = NULL;
	c->ramimage_read = NULL;
	c->ramimage_write = NULL;
	switch(mode[0]){
	case 'd':
		c->mode = MODE_ROM_DUMP;
		c->romimage = file;
		break;
	case 'r':
		c->mode = MODE_RAM_READ;
		c->ramimage_read = file;
		break;
	case 'w':
		c->mode = MODE_RAM_WRITE;
		c->ramimage_write = file;
		break;
	default:
		printf("%s unkown mode %s\n", PREFIX_CONFIG_ERROR, mode);
		return NG;
	};
	switch(c->mode){
	case MODE_RAM_READ:
	case MODE_RAM_WRITE:
		if(argc != 4){
			printf("%s too many argument\n", PREFIX_CONFIG_ERROR);
			return NG;
		}
	}
	
	c->script = script;
	c->mapper = CONFIG_OVERRIDE_UNDEF;
	c->mirror = CONFIG_OVERRIDE_UNDEF;
	c->backupram = CONFIG_OVERRIDE_UNDEF;
	c->mapper = CONFIG_OVERRIDE_UNDEF;
	{
		int flag_error = OK, mapper_error = OK;
		switch(argc){
		case 5:
			flag_error = flag_get(flag, c);
			break;
		case 6:
			flag_error = flag_get(flag, c);
			mapper_error = value_get(mapper, &c->mapper);
			break;
		}
		if(flag_error != OK){
			printf("%s unknown flag %s\n", PREFIX_CONFIG_ERROR, flag);
			return NG;
		}
		if(mapper_error != OK){
			printf("%s unknown mapper %s\n", PREFIX_CONFIG_ERROR, flag);
			return NG;
		}
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
		config_result = config_init(c, v[1], v[2], v[3], NULL, NULL, &config);
		break;
	case 5: //mode script target flag
		config_result = config_init(c, v[1], v[2], v[3], v[4], NULL, &config);
		break;
	case 6: //mode script target flag mapper
		config_result = config_init(c, v[1], v[2], v[3], v[4], v[5], &config);
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
