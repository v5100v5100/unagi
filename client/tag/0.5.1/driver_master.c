#include <string.h>
#include "driver_master.h"
#include "driver_onajimi.h"
#include "driver_hongkongfc.h"

//����� rodata �ˤ��������� const ���դ�����ʬ�����
static const struct driver *DRIVER_LIST[] = {
	&DRIVER_ONAJIMI, &DRIVER_HONGKONGFC,
	NULL
};

const struct driver *driver_get(const char *name)
{
	const struct driver **d;
	d = DRIVER_LIST;
	while(*d != NULL){
		if(strcmp(name, (*d)->name) == 0){
			return *d;
		}
		d++;
	}
	return NULL;
}
