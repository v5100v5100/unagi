#ifndef _SCRIPT_DUMP_H_
#define _SCRIPT_DUMP_H_
enum{
	DUMP_SCRIPT_STR_LENGTH = 20,
	DUMP_TARGET_STR_LENGTH = 50
};
struct dump_config{
	char script[DUMP_SCRIPT_STR_LENGTH];
	char target[DUMP_TARGET_STR_LENGTH];
	struct reader_handle handle;
	const struct reader_control *control;
	struct dump_memory_driver{
		const struct reader_memory_access *access;
		struct memory memory;
		long read_count, increase;
		struct gauge gauge;
	}cpu, ppu;
	long mappernum;
	//struct romimage rom;
	bool progress;
	struct textcontrol log;
};
void script_dump_execute(struct dump_config *c);
#endif
