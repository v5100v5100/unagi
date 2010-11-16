function dumpsize_get(m, increase)
{
	local dumpsize = m.size_base * increase;
	if(dumpsize > m.size_max){
		dumpsize = m.size_max;
	}
	return dumpsize;
}

function dump(d, script, mappernum, increase_cpu, increase_ppu)
{
	const mega = 0x20000;
	const INCREASE_AUTO = 11;
	enum memory_type{ROM, RAM};
	dofile(script);

	local vram = board.vram_mirrorfind == true ? 1 : 0;
	local ppuarea_memory;
	if(mappernum == -1){
		mappernum = board.mappernum;
	}
	if(board.ppu_rom.size_base == 0){
		ppuarea_memory = memory_type.RAM;
	}else if(board.ppu_ramfind == true){
		ppuarea_memory = ppu_ramfind(d) == true ? memory_type.RAM : memory_type.ROM;
		if(ppuarea_memory == memory_type.RAM){
			increase_ppu = 0;
		}
	}else{
		ppuarea_memory = memory_type.ROM;
	}
	if(increase_cpu == INCREASE_AUTO){
		if(ppuarea_memory == memory_type.RAM && board.ppu_ramfind == true){
			increase_cpu = 2;
		}else{
			increase_cpu = 1;
		}
	}
	local cpu_dumpsize = dumpsize_get(board.cpu_rom, increase_cpu);
	local ppu_dumpsize = dumpsize_get(board.ppu_rom, increase_ppu);

	memory_new(d, cpu_dumpsize, ppu_dumpsize);
	cpu_dump(d, cpu_dumpsize / board.cpu_rom.banksize, board.cpu_rom.banksize);
	if(ppuarea_memory == memory_type.ROM){
		ppu_dump(d, ppu_dumpsize / board.ppu_rom.banksize, board.ppu_rom.banksize);
	}
	nesfile_save(d, mappernum, vram);
}

function workram_rw(d, script, increase_cpu)
{
	dofile(script);
	local cpu_dumpsize = dumpsize_get(board.cpu_ram, increase_cpu);
	memory_new(d, cpu_dumpsize, 0);
	cpu_ram_access(d, cpu_dumpsize / board.cpu_ram.banksize, board.cpu_ram.banksize);
	memory_finalize(d);
}
