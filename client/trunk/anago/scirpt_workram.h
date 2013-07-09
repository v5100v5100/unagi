#ifndef _SCRIPT_WORKRAM_H_
#define _SCRIPT_WORKRAM_H_
struct workram_config{
	const wgChar *script, *target;
	const struct reader_handle *handle;
	const struct reader_control *control;
	struct textcontrol log;
	void (*except)(const wgChar *str);
	
	const struct reader_memory_access *access;
	struct memory memory;
	long read_count, increase;
	struct gauge gauge;
};
#endif
