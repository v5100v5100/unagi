MCU_164P = MCU=atmega164p TARGET=kazzo_mega164p
MCU_16 = MCU=atmega16 TARGET=kazzo_mega16
SOURCE_ROOT = Makefile COPYING kazzo_test.exe kazzo_schematics.png readme.txt usbrequest.txt kazzo_mega16.hex kazzo_mega164p.hex
SOURCE_FIRMWARE = \
	avr_main.c bus_access.c disk_access.c flashmemory.c \
	bus_access.h disk_access.h flashmemory.h kazzo_request.h kazzo_task.h type.h usbconfig.h \
	firmware.mak usbdrv/*
SOURCE_ECHO = Makefile hostecho.c opendevice.c opendevice.h
SOURCE_MODULE = reader_kazzo.c usb_device.c reader_kazzo.h reader_master.h usb_device.h
WINDOWS_DRIVER = libusb0.dll kazzo.inf libusb0.sys

all:
	(cd firmware;make -f firmware.mak $(MCU_164P))
	(cd firmware;make -f firmware.mak $(MCU_16))
	(cd hostecho;make)
clean:
	(cd firmware;make -f firmware.mak $(MCU_164P) clean)
	(cd firmware;make -f firmware.mak $(MCU_16) clean)
	(cd hostecho;make clean)
p4p:
	(cd firmware;make -f firmware.mak $(MCU_164P) program)
p4f:
	(cd firmware;make -f firmware.mak $(MCU_164P) fuse)
package:
	(cd ..; 7za a kazzo_xxx.zip \
		$(addprefix kazzo/,$(SOURCE_ROOT)) \
		$(addprefix kazzo/firmware/,$(SOURCE_FIRMWARE)) \
		$(addprefix kazzo/hostecho/,$(SOURCE_ECHO)) \
		$(addprefix kazzo/hostmodule/,$(SOURCE_MODULE)) \
		$(addprefix kazzo/windows_driver/,$(WINDOWS_DRIVER)) \
	)
