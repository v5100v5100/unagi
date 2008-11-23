#include <string.h>
#include "driver_master.h"
#include "driver_onajimi.h"
#include "driver_hongkongfc.h"

const struct driver *driver_get(const char *name)
{
	if(strcmp(name, DRIVER_ONAJIMI.name) == 0){
		return &DRIVER_ONAJIMI;
	}else if(strcmp(name, DRIVER_HONGKONGFC.name) == 0){
		return &DRIVER_HONGKONGFC;
	}
	return NULL;
}
