/*
famicom ROM cartridge utility - unagi
パラレルポート共有定義
*/
#ifndef _PARALELL_PORT_INLINE_H_
#define _PARALELL_PORT_INLINE_H_
#include "giveio.h"
#include <windows.h>
#define ASM_ENABLE (0)
enum{
	PORT_DATA = 0x0378,
	PORT_BUSY,
	PORT_CONTROL
};
enum{
	ADDRESS_MASK_A0toA12 = 0x1fff,
	ADDRESS_MASK_A0toA14 = 0x7fff,
	ADDRESS_MASK_A15 = 0x8000
};
enum{ 
	M2_CONTROL_TRUE, M2_CONTROL_FALSE
};

int _inp(int);
#if ASM_ENABLE==0
void _outp(int, int);
#else
static inline void _outp(int address, int data){
	asm(
		" movl %0,%%edx\n"
		" movl %1,%%eax\n"
		" out %%al,%%dx\n"
		:: "q"(address), "q"(data): "%edx", "%eax"
	);
}
#endif

/*
static inline は共有マクロ扱い
*/
static inline int bit_set(int data, const int bit)
{
	data |= 1 << bit;
	return data;
}

static inline int bit_clear(int data, const int bit)
{
	data &= ~(1 << bit);
	return data;
}

static inline void wait(long msec)
{
	if(msec == 0){
		return;
	}
	//const long waittime = 100000;
	Sleep(msec);
}

#endif
