#ifndef _SCRIPT_FLASH_H_
#define _SCRIPT_FLASH_H_
enum{
	PROGRAM_SCRIPT_STR_LENGTH = 20,
	PROGRAM_TARGET_STR_LENGTH = 50
};
struct config_flash{
	char script[PROGRAM_SCRIPT_STR_LENGTH];
	char target[PROGRAM_TARGET_STR_LENGTH];
	struct flash_device flash_cpu, flash_ppu;
	const struct reader_driver *reader;
	struct romimage rom;
	bool compare, testrun;
	struct gauge gauge_cpu, gauge_ppu;
	struct textcontrol log;
};
void script_flash_execute(struct config_flash *c);
#endif
