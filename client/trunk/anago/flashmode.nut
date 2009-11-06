mega <- 0x20000;
trans_empty <- 0;
function cpu_banksize_get(capacity)
{
	return capacity * mega / cpu_banksize;
}
function ppu_banksize_get(capacity)
{
	return capacity * mega / ppu_banksize;
}
function loopsize_get(flashsize, trans, size)
{
	local trans_full = 3, trans_top = 1, trans_bottom = 2; //header.h enum transtype
	local loop;
	switch(trans){
	case trans_full:
		loop = flashsize.full;
		break;
	case trans_top:
		loop = flashsize.top[size];
		break;
	case trans_bottom:
		loop = flashsize.bottom[size];
		break;
	default:
		loop = {start = 0, end = 0};
		break;
	}
	return loop;
}
function program(d, mapper, cpu_trans, cpu_size, ppu_trans, ppu_size)
{
	if(board.mapper != mapper){
		print("mapper number not connected");
		return;
	}
	local cpu_loop = loopsize_get(board.cpu_flashsize, cpu_trans, cpu_size);
	local ppu_loop = loopsize_get(board.ppu_flashsize, ppu_trans, ppu_size);
	local co_cpu = newthread(program_cpu);
	local co_ppu = newthread(program_ppu);
	initalize(d);
	if(cpu_trans != trans_empty){
		cpu_erase(d);
	}
	if(ppu_trans != trans_empty){
		ppu_erase(d);
	}
	erase_wait(d);
	if(cpu_trans != 0){
		co_cpu.call(d, cpu_loop);
	}
	if(ppu_trans != trans_empty){
		co_ppu.call(d, ppu_loop);
	}
	program_main(d, co_cpu, co_ppu)
}

