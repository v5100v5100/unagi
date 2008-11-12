#include <stdio.h>
#include <stdlib.h>
#include "type.h"
#include "paralellport.h"
#include "giveio.h"
#include "file.h"
#include "script.h"

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
		const int testbufsize = 0x2000;
		u8 testbuf[testbufsize];
		int i;
		/*cpu_write(0x8000, 0x80);
		cpu_write(0xe000, 0);
		cpu_write(0xe000, 0);
		cpu_write(0xe000, 0);
		cpu_write(0xe000, 0);
		cpu_write(0xe000, 0);*/
		for(i=0;i<0x10;i++){
			cpu_write(0x6000 + i, i);
		}
		cpu_read(0x6000, testbufsize, testbuf);
		buf_save(testbuf, file, testbufsize);
		}break;
	}
	
	if(gg != GIVEIO_WIN95){
		giveio_stop(GIVEIO_STOP);
	}
	return;
}

int main(int c, char **v)
{
	switch(c){
	case 2:
		backupram_test(v[1]);
		break;
	case 3:
		script_load(v[1], v[2], 0);
		break;
	case 4:
		script_load(v[1], v[2], 1);
		break;
	default:
		printf("%s [mapper script] [dump file]", v[0]);
	}
	return 0;
}
