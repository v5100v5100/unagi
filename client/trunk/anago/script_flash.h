#ifndef _SCRIPT_PROGRAM_H_
#define _SCRIPT_PROGRAM_H_
enum{
	PROGRAM_SCRIPT_STR_LENGTH = 20,
	PROGRAM_TARGET_STR_LENGTH = 80
};
struct program_config{
	char script[PROGRAM_SCRIPT_STR_LENGTH];
	char target[PROGRAM_TARGET_STR_LENGTH];
	struct reader_handle handle;
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
};
void script_program_execute(struct program_config *c);
#endif
