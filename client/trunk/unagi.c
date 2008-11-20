/*
famicom ROM cartridge dump program - unagi
command line interface

todo:
* test 用の関数などのコマンドライン系統を統一する
* mirror, battery, mapper number をコマンドラインからも指定できるようにする
*/
#include <stdio.h>
#include <stdlib.h>
#include "type.h"
#include "paralellport.h"
#include "giveio.h"
#include "file.h"
#include "script.h"

#if DEBUG==1
static void backupram_test(const char *file)
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
		printf("Can't Access Direct IO %d\n", gg);
		return;
	}

	switch(file[0]){
	case 'p':
		printf("%d\n", ppu_ramtest());
		break;
	case 'b':{
		const int testbufsize = 0x100;
		u8 testbuf[testbufsize];
		int i;
		cpu_read(0x6000, testbufsize, testbuf);
		for(i=0;i<0x10;i++){
			printf("%02x ", testbuf[i]);
		}
		}break;
	}
	
	if(gg != GIVEIO_WIN95){
		giveio_stop(GIVEIO_STOP);
	}
	return;
}
#endif

int main(int c, char **v)
{
	switch(c){
#if DEBUG==1
	case 2:
		backupram_test(v[1]);
		break;
#endif
	case 4:
		script_load(v[1], v[2], v[3], 0);
		break;
	case 5:
		script_load(v[1], v[2], v[3], 1);
		break;
	default:
		printf("%s [mode] [mapper script] [target file]\n", v[0]);
		printf("mode - [d]ump ROM / [r]ead RAM/ [w]rite RAM\n");
	}
	return 0;
}
