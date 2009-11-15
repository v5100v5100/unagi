#ifndef _MEMORY_MANAGE_H_
#define _MEMORY_MANAGE_H_
void mm_init(void);
void *mm_malloc(const char *file, int line, const char *function, int size);
void mm_free(void *addr);
void mm_end(void);
#if DEBUG == 0
#include <stdlib.h>
  #define Malloc(size) malloc(size)
  #define Free(addr) free(addr)
#else
  #define Malloc(size) mm_malloc(__FILE__, __LINE__, __FUNCTION__, size)
  #define Free(addr) mm_free(addr)
#endif
#endif
