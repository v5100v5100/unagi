#include <stddef.h>
#include "type.h"
#include "widget.h"

static void gauge_range_nothing(void *a, int b)
{
}
static void gauge_value_nothing(void *a, void *b, int c)
{
}
static void label_nothing(void *a, const wgChar *str, ...)
{
}
const struct gauge GAUGE_DUMMY = {
	.bar = NULL, .label = NULL,
	.range_set = gauge_range_nothing,
	.value_set = gauge_value_nothing,
	.value_add = gauge_value_nothing,
	.label_set = label_nothing
};


