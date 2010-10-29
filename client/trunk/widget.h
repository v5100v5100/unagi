#ifndef _WIDGET_H_
#define _WIDGET_H_
#ifndef __cplusplus 
  #include <stdarg.h>
#else
  #include <cstdarg>
#endif

struct gauge{
	void *bar, *label;
	void (*range_set)(void *, int range);
	void (*value_set)(void *, void *, int value);
	void (*value_add)(void *, void *, int value);
	void (*label_set)(void *, const wgChar *str, ...);
};
struct textcontrol{
	void *object;
	void (*append)(void *, const wgChar *, ...);
	void (*append_va)(void *, const wgChar *, va_list);
};
extern const struct gauge GAUGE_DUMMY;
#endif
