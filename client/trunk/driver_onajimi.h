#ifndef _DRIVER_ONAJIMI_H_
#define _DRIVER_ONAJIMI_H_

void reader_init(void);
void cpu_read(long address, long length, u8 *data);
void ppu_read(long address, long length, u8 *data);
void cpu_write(long address, long data);
void ppu_write(long address, long data);
#endif
