#ifndef _FILE_H_
#define _FILE_H_
#include "type.h"
int buf_load(uint8_t *buf, const wgChar *file, int size);
void* buf_load_full(const wgChar *file, int *size);
void buf_save(const void *buf, const wgChar *file, int size);
#endif
