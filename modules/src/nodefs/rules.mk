MODULES+=modules/src/nodefs/nodefs.ko
CLEAN+=modules/src/nodefs/main.o modules/src/nodefs/driver.o

modules/src/nodefs/nodefs.ko: lib/src/memory.o lib/src/string.o modules/src/nodefs/main.o modules/src/nodefs/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
