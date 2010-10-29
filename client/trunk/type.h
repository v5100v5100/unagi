#ifndef _TYPE_H_
#define _TYPE_H_
#include <stdint.h>
#ifndef __cplusplus 
  #include <stdbool.h>
#endif

enum{
	OK = 0, NG
};
#ifdef _UNICODE
  #include <wchar.h>
  typedef wchar_t wgChar;
  #define wgT(x) L##x
#else
  typedef char wgChar;
  #define wgT(x) x
#endif
#endif
