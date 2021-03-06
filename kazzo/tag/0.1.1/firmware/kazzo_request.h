#ifndef _KAZZO_REQUEST_H_
#define _KAZZO_REQUEST_H_
#define USB_CFG_VENDOR_ID       0xc0, 0x16
#define USB_CFG_DEVICE_ID       0xdc, 0x05
#define USB_CFG_DEVICE_VERSION  0x00, 0x01
#define USB_CFG_VENDOR_NAME     'o', 'b', 'd', 'e', 'v', '.', 'a', 't'
#define USB_CFG_VENDOR_NAME_LEN 8
#define USB_CFG_DEVICE_NAME     'k', 'a', 'z', 'z', 'o'
#define USB_CFG_DEVICE_NAME_LEN 5

enum request{
	REQUEST_ECHO = 0,
	REQUEST_PHI2_INIT,
	REQUEST_CPU_READ_6502, REQUEST_CPU_READ, 
	REQUEST_CPU_WRITE_6502, REQUEST_CPU_WRITE_FLASH,
	REQUEST_PPU_READ, REQUEST_PPU_WRITE,
	REQUEST_FLASH_STATUS, REQUEST_FLASH_CONFIG_SET,
	REQUEST_FLASH_PROGRAM, REQUEST_FLASH_ERASE,
	REQUEST_FLASH_DEVICE, //REQUEST_FLASH_BUFFER_GET,
	REQUEST_VRAM_CONNECTION,

	//future expanstion
	REQUEST_DISK_STATUS_GET, REQUEST_DISK_READ, REQUEST_DISK_WRITE,
	
	//bootloader control
	REQUEST_FIRMWARE_VERSION = 0x80, REQUEST_FIRMWARE_PROGRAM,
	REQUEST_FIRMWARE_DOWNLOAD
};
enum index{
	INDEX_IMPLIED = 0, INDEX_CPU, INDEX_PPU, INDEX_BOTH
};
enum {
	VERSION_STRING_SIZE = 0x20,
//	READ_PACKET_SIZE = 0x0100, //use macro to compare SPM_PAGESIZE
	FLASH_PACKET_SIZE = 0x0100
};
#define READ_PACKET_SIZE (0x100)
#endif
