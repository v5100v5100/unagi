all: $(TARGET) unagi.d
unagi.d:
	$(CC) -MM $(CFLAGS) ../*.c > $@
nesheader.exe: $(OBJ_HD)
	$(CC) -o $@ $(OBJ_HD)
clean: 
	rm -f $(OBJ) $(TARGET) unagi.d
unagi.res.o: unagi.rc unagi.ico
	windres -I.. -i $< -o $@
all: $(TARGET) unagi.d
