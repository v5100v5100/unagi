#ifndef _SCRIPT_DUMP_H_
#define _SCRIPT_DUMP_H_
enum dump_mode{
	MODE_ROM_DUMP, MODE_RAM_READ, MODE_RAM_WRITE
};
enum {
	INCREASE_AUTO = 11
};
struct dump_config{
	enum dump_mode mode;
	const wgChar *script;
	const wgChar *target;
	const struct reader_handle *handle;
	const struct reader_control *control;
	struct dump_memory_driver{
		const struct reader_memory_access *access;
		struct memory memory;
		long read_count, increase;
		struct gauge gauge;
	}cpu, ppu;
	long mappernum;
	bool progress;
	bool battery;
	unsigned long crc;
	struct textcontrol log;
	void (*except)(const wgChar *str);
};
bool script_dump_execute(struct dump_config *c);
bool script_workram_execute(struct dump_config *d);
#endif
