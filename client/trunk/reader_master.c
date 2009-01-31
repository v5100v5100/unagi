#include <assert.h>
#include <string.h>
#include "giveio.h"
#include "reader_master.h"
#include "reader_onajimi.h"
#include "reader_hongkongfc.h"
#include "reader_dozeu.h"

//����� rodata �ˤ��������� const ���դ�����ʬ�����
static const struct reader_driver *DRIVER_LIST[] = {
	&DRIVER_ONAJIMI, &DRIVER_HONGKONGFC, &DRIVER_DOZEU,
	NULL
};

int paralellport_open_or_close(int oc)
{
	static int giveio_status;
	if(oc == READER_OPEN){
		giveio_status = giveio_start();
		switch(giveio_status){
		case GIVEIO_OPEN:
		case GIVEIO_START:
		case GIVEIO_WIN95:
			break;
		default:
		case GIVEIO_ERROR:
			//printf("%s Can't Access Direct IO %d\n", __FILE__, giveio_status);
			return NG;
		}
	}else if(oc == READER_CLOSE){
		if(giveio_status != GIVEIO_WIN95){
			giveio_stop(GIVEIO_STOP);
		}
	}else{
		assert(0);
	}
	return OK;
}

const struct reader_driver *reader_driver_get(const char *name)
{
	const struct reader_driver **d;
	d = DRIVER_LIST;
	while(*d != NULL){
		if(strcmp(name, (*d)->name) == 0){
			return *d;
		}
		d++;
	}
	return NULL;
}
