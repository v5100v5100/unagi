function flash_device_get(name)
{
	local mega = 0x20000;
	local device = {
		["dummy"] = {
			capacity = 16 * mega, pagesize = 1,
			erase_wait = 0, erase_require = false,
			id_manufacurer = 0xf1, id_device = 0xf1
		},
		["W29C020"] = {
			capacity = 2 * mega, pagesize = 0x80,
			erase_wait = 50, erase_require = false,
			id_manufacurer = 0xda, id_device = 0x45
		},
		["W29C040"] = {
			capacity = 4 * mega, pagesize = 0x100,
			erase_wait = 50, erase_require = false,
			id_manufacurer = 0xda, id_device = 0x46
		},
		["W49F002"] = {
			capacity = 2 * mega, pagesize = 1,
			erase_wait = 100, erase_require = true,
			id_manufacurer = 0xda, id_device = 0xae
		},
		["EN29F002T"] = {
			capacity = 2 * mega, pagesize = 1,
			erase_wait = 2000, erase_require = true,
			id_manufacurer = 0x1c, id_device = 0x92
		},
		["AM29F040B"] = {
			capacity = 4 * mega, pagesize = 1,
			erase_wait = 8000, erase_require = true,
			id_manufacurer = 0x01, id_device = 0xa4
		},
		//chip erase time is not written in datasheet!!
		["MBM29F080A"] = {
			capacity = 8 * mega, pagesize = 1,
			erase_wait = 8000, erase_require = true,
			id_manufacurer = 0x04, id_device = 0xd5
		}
	};
	return device[name];
}
