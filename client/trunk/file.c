#include <stdio.h>
#include "type.h"
#include "memory_manage.h"
#include "file.h"

#ifdef _UNICODE
  #define FOPEN(name, mode) _wfopen(name, L##mode)
#else
  #define FOPEN(name, mode) _fopen(name, mode)
#endif

int buf_load(uint8_t *buf, const wgChar *file, int size)
{
	FILE *fp;

	fp = FOPEN(file, "rb");
	if(fp == NULL){
		return NG;
	}
	fseek(fp, 0, SEEK_SET);
	fread(buf, sizeof(uint8_t), size, fp);
	fclose(fp);
	return OK;
}

void* buf_load_full(const wgChar *file, int *size)
{
	FILE *fp;
	uint8_t *buf;

	*size = 0;
	fp = FOPEN(file, "rb");
	if(fp == NULL){
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	*size = ftell(fp);
	if(*size == 0){
		fclose(fp);
		return NULL;
	}
	fseek(fp, 0, SEEK_SET);
	buf = Malloc(*size);
	fread(buf, sizeof(uint8_t), *size, fp);
	fclose(fp);
	return buf;
}

void buf_save(const void *buf, const wgChar *file, int size)
{
	FILE *fp;

	fp = FOPEN(file, "wb");
	fseek(fp, 0, SEEK_SET);
	fwrite(buf, sizeof(uint8_t), size, fp);
	fclose(fp);
}

