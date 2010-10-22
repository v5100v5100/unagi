#ifndef _SCRIPT_DUMP_H_
#define _SCRIPT_DUMP_H_
enum{
	DUMP_SCRIPT_STR_LENGTH = 20,
	DUMP_TARGET_STR_LENGTH = 50
};
struct config_dump{
	char script[DUMP_SCRIPT_STR_LENGTH];
	char target[DUMP_TARGET_STR_LENGTH];
	const struct reader_driver *reader;
	long mappernum;
	//struct romimage rom;
	struct {long cpu, ppu;} increase;
	bool progress;
	struct gauge gauge_cpu, gauge_ppu;
	struct textcontrol log;
};
void script_dump_execute(struct config_dump *c);
#endif
