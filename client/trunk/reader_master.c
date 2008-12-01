#include <string.h>
#include "driver_master.h"
#include "driver_onajimi.h"
#include "driver_hongkongfc.h"

//これを rodata にしたいけど const の付け方が分からん
static const struct reader_driver *DRIVER_LIST[] = {
	&DRIVER_ONAJIMI, &DRIVER_HONGKONGFC,
	NULL
};

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
