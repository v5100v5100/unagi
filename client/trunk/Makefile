OBJ_HK = giveio.o reader_hongkongfc.o
OBJ_HD = head/nesheader.o head/header.o file.o
TARGET_DIR = debug
TARGET_MAK = debug.mak
ifeq ($(PROFILE),1)
	TARGET_DIR = profile
	TARGET_MAK = profile.mak
endif
ifeq ($(RELEASE),1)
	TARGET_DIR = release
	TARGET_MAK = release.mak
endif

all:
	cd $(TARGET_DIR); make -f $(TARGET_MAK)
	cp $(TARGET_DIR)/unagi.exe .
clean:
	rm -f unagi.exe \
		debug/*.o debug/*.exe debug/*.d \
		profile/*.o profile/*.exe profile/*.d \
		release/*.o release/*.exe release/*.d

head/nesheader.o: nesheader.c
	$(CC) $(CFLAGS) -DHEADEROUT -I. -c -o $@ $<
head/header.o: header.c
	$(CC) $(CFLAGS) -DHEADEROUT -I. -c -o $@ $<
hk.exe: $(OBJ_HK)
	$(CC) -o $@ $(OBJ_HK)
iodel.exe: iodel.o giveio.o
	$(CC) -o $@ iodel.o giveio.o
