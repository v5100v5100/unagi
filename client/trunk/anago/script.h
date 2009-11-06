#ifndef _SCRIPT_H_
#define _SCRIPT_H_
struct config{
	const char *script, *target;
	struct flash_device flash_cpu, flash_ppu;
	const struct reader_driver *reader;
	struct romimage rom;
};
void script_execute(struct config *c);
#endif
