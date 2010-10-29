#ifndef _SCRIPT_PROGRAM_H_
#define _SCRIPT_PROGRAM_H_
struct program_config{
	const wgChar *script;
	const wgChar *target;
	const struct reader_handle *handle;
	const struct reader_control *control;
	struct flash_memory_driver{
		const struct reader_memory_access *access;
		struct flash_device flash;
		struct memory memory;
		struct {
			long address, length, count, offset;
		}programming, compare;
		bool command_change;
		long c000x, c2aaa, c5555;
		struct gauge gauge;
	}cpu, ppu;
	long mappernum;
	bool compare, testrun;
	struct textcontrol log;
	void (*except)(const wgChar *str);
};
void script_program_execute(struct program_config *c);
#endif
