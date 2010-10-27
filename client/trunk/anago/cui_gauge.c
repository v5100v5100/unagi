#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef WIN32
  #include <windows.h>
#endif
#include "memory_manage.h"
#include "widget.h"

struct cui_gauge{
	const char *name;
	int value, range;
	int lineback, lineforward;
};

static void range_set(void *obj, int value)
{
	struct cui_gauge *t = obj;
	t->range = value;
}

static void console_move(int line)
{
	if(line == 0){
		return;
	}
#ifdef WIN32
	HANDLE c;
	CONSOLE_SCREEN_BUFFER_INFO info;
	c = GetStdHandle(STD_OUTPUT_HANDLE);
	if(GetConsoleScreenBufferInfo(c, &info) == 0){
		//command.com, cygwin shell, mingw shell
		if(line < 0){
			printf("\x1b[%dA", -line);
		}else if(line == 1){
			printf("\n");
			fflush(stdout);
		}else{
			printf("\n");
			fflush(stdout);
			printf("\x1b[%dB", line - 1);
//			printf("\x1b[%dB", line - 1);
//			fflush(stdout);
		}
	}else{
		//cmd.exe
		info.dwCursorPosition.X = 0;
		info.dwCursorPosition.Y += line;
		SetConsoleCursorPosition(c, info.dwCursorPosition);
	}
#else
	if(line < 0){
		printf("\x1b[%dA\x1b[35D", -line);
	}else{
		printf("\x1b[%dB\x1b[35D", line);
	}
#endif
//	fflush(stdout);
}

static void draw(const struct cui_gauge *t)
{
	const int barnum = 16;
	const int unit = t->range / barnum;
	int igeta = t->value / unit;
	char bar[barnum + 3 + 1];
	char *text = bar;
	int i;
	assert(igeta <= barnum);
	printf("%s 0x%06x/0x%06x ", t->name, (int) t->value, (int) t->range);
	*text++ = '|';
	for(i = 0; i < igeta; i++){
		if(i == barnum / 2){
			*text++ = '|';
		}
		*text++ = '#';
	}
	for(; i < barnum; i++){
		if(i == barnum / 2){
			*text++ = '|';
		}
		*text++ = ' ';
	}
	*text++ = '|';
	*text = '\0';
	printf(bar);
//	fflush(stdout);
}


static void value_set(void *obj, void *d, int value)
{
	struct cui_gauge *t = (struct cui_gauge *) obj;
	t->value = value;
	if(t->range != 0){
		draw(t);
	}
	console_move(1);
}

static void value_add(void *obj, void *d, int value)
{
	struct cui_gauge *t = (struct cui_gauge *) obj;
	t->value += value;
	console_move(t->lineback);
	draw(t);
	console_move(t->lineforward);
}

static void name_set(void *obj, const char *name, int lineforward, int lineback)
{
	struct cui_gauge *t = (struct cui_gauge *) obj;
	t->name = name;
	t->lineforward = lineforward;
	t->lineback = lineback;
}

static void label_set(void *obj, const char *format, ...)
{
	va_list list;
	const struct cui_gauge *t = (const struct cui_gauge *) obj;

	va_start(list, format);
	console_move(t->lineback);
	printf("%s ", t->name);
	vprintf(format, list);
	console_move(t->lineforward);
	va_end(list);
}

void cui_gauge_new(struct gauge *t, const char *name, int lineforward, int lineback)
{
	t->bar =  Malloc(sizeof(struct cui_gauge));
	t->label = t->bar;
	name_set(t->bar, name, lineforward, lineback);
	t->range_set = range_set;
	t->value_set = value_set;
	t->value_add = value_add;
	t->label_set = label_set;
}

void cui_gauge_destory(struct gauge *t)
{
	Free(t->bar);
	t->bar = NULL;
	t->label = NULL;
	t->range_set = NULL;
	t->value_set = NULL;
	t->value_add = NULL;
	t->label_set = NULL;
}
