MODULES+=modules/src/arch/x86/uart/uart.ko
CLEAN+=modules/src/arch/x86/uart/main.o modules/src/arch/x86/uart/driver.o modules/src/arch/x86/uart/device.o

modules/src/arch/x86/uart/uart.ko: lib/src/memory.o lib/src/string.o lib/src/arch/x86/io.o modules/src/arch/x86/uart/main.o modules/src/arch/x86/uart/driver.o modules/src/arch/x86/uart/device.o
	$(LD) $(LDFLAGS) -o $@ $^
