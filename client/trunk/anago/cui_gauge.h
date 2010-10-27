#ifndef _CUI_GAUGE_H_
#define _CUI_GAUGE_H_
struct gauge;
void cui_gauge_new(struct gauge *t, const char *name, int lineforward, int lineback);
void cui_gauge_destory(struct gauge *t);
#endif
