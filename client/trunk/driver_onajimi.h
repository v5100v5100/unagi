#ifndef _PARALELLPORT_H
#define _PARALELLPORT_H

void reader_init(void);
void cpu_read(long address, long length, u8 *data);
void ppu_read(long address, long length, u8 *data);
void cpu_write(long address, long data);
void ppu_write(long address, long data);
#endif
