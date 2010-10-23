#ifndef _WIDGET_H_
#define _WIDGET_H_
struct gauge{
	void *bar, *label;
	void (*range_set)(void *, int range);
	void (*value_set)(void *, void *, int value);
	void (*value_add)(void *, void *, int value);
	void (*label_set)(void *, const char *str);
};
struct textcontrol{
	void *object;
	void (*append)(void *, const char *);
};
extern const struct gauge GAUGE_DUMMY;
#endif
