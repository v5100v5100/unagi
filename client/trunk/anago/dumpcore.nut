function dump(d, script, mappernum, increase_cpu, increase_ppu)
{
	const mega = 0x20000;
	dofile(script);
	local vram = board.vram_mirrorfind == true ? 1 : 0;
	if(mappernum == -1){
		mappernum = board.mappernum;
	}
	memory_new(d, board.cpu_romsize * increase_cpu, board.ppu_romsize * increase_ppu);
	cpu_dump(d, board.cpu_romsize * increase_cpu / board.cpu_banksize, board.cpu_banksize);
	if(board.ppu_ramfind == true){
		if(ppu_ramfind(d) == true){
			nesfile_save(d, mappernum, vram);
			return;
		}
	}
	if(board.ppu_romsize != 0){
		ppu_dump(d, board.ppu_romsize * increase_ppu / board.ppu_banksize, board.ppu_banksize);
	}
	nesfile_save(d, mappernum, vram);
	return;
}

function workram_rw(d, script, increase_cpu)
{
	dofile(script);
	memory_new(d, board.cpu_ramsize * increase_cpu, 0);
	cpu_ram_access(d, board.cpu_ramsize / board.cpu_banksize, board.cpu_ramsize);
	memory_finalize(d);
}
