NPROC=$(shell nproc)
SHELL:=/bin/bash

MIPS=/opt/mips32-mti-elf/2019.09-03-2/bin/mips-mti-elf
LIB_DIR=/opt/mips32-mti-elf/2019.09-03-2/mips-mti-elf/lib/mips32/el/sof

CFLAGS=-EL -Os -G0 -mips2 -msoft-float -mno-abicalls -fno-pic -ffreestanding
LDFLAGS=-EL -nostdlib -z max-page-size=32

LDFLAGS+=-L/opt/mips32-mti-elf/2019.09-03-2/lib/gcc/mips-mti-elf/7.4.0/mips2/el/sof/
LIBS+=-lgcc
 # -lc -lm

# CORE_OBJS=core.o lib.o debug.o
CORE_OBJS=core_api.o lib.o debug.o
LOADER_OBJS=init.o main.o debug.o

CORE=cores/gpsp


# Default target
all: core_87000000 bisrv install

core:
	$(call echo_c,"compiling $(CORE)")
	$(MAKE) -j$(NPROC) -C $(CORE) platform=sf2000
	cp $(CORE)/*.a libretro_core.a

# core.o: core.s
# $(MIPS)-gcc $(CFLAGS) -o $@ -c $<

core_api.o: core_api.c
	$(MIPS)-gcc $(CFLAGS) -Ilibs/libretro-common/include -o $@ -c $<

lib.o: lib.c
	$(MIPS)-gcc $(CFLAGS) -o $@ -c $<

debug.o: debug.c
	$(MIPS)-gcc $(CFLAGS) -o $@ -c $<

core.elf: core $(CORE_OBJS)
	$(MIPS)-ld -Map $@.map $(LDFLAGS) -e __core_entry__ -Ttext=0x87000000 bisrv_08_03.ld -o core.elf \
		--start-group $(LIBS) $(CORE_OBJS) libretro_core.a --end-group

core_87000000: core.elf
	$(MIPS)-objcopy -O binary -j .text -j .rodata -j .data core.elf core_87000000


init.o: init.s
	$(MIPS)-gcc $(CFLAGS) -o $@ -c $<

main.o: main.c
	$(MIPS)-gcc $(CFLAGS) -Wall -I libs/libretro-common/include -o $@ -c $<

loader.elf: $(LOADER_OBJS)
	$(MIPS)-ld $(LDFLAGS) -Ttext=0x800016d0 bisrv_08_03.ld $(LOADER_OBJS) -o loader.elf


bisrv: loader.elf crc
	$(call echo_c,"patching bisrv")

	$(Q)cp bisrv_08_03.asd bisrv.asd

	$(Q)$(MIPS)-objcopy -O binary -j .text -j .rodata -j .data loader.elf loader.bin

	$(Q)dd if=loader.bin of=bisrv.asd bs=$$((0x16d0)) seek=1 conv=notrunc 2>/dev/null

	# jal run_nes -> jal 0x800016d0
	printf "\xB4\x05\x00\x0C" | dd of=bisrv.asd bs=1 seek=$$((0x35a900)) conv=notrunc
	# endless loop in sys_watchdog_reboot -> j 0x800016d8
	printf "\xB6\x05\x00\x08" | dd of=bisrv.asd bs=1 seek=$$((0x30d4)) conv=notrunc
	# endless loop in INT_General_Exception_Hdlr -> j 0x800016e0
	printf "\xB8\x05\x00\x08" | dd of=bisrv.asd bs=1 seek=$$((0x495a0)) conv=notrunc

	$(Q)./crc bisrv.asd

crc: crc.c
	gcc -o crc crc.c

install:
	$(call echo_c,"install to sdcard")
	$(Q)cp bisrv.asd sdcard/bios/
	$(Q)cp core_87000000 sdcard/

# Clean intermediate files and the final executable
clean:
	-rm $(CORE_OBJS)
	-rm $(LOADER_OBJS)
	-rm loader.elf loader.bin core.elf core_87000000
	-rm bisrv.asd
	-rm libretro_core.a
	$(MAKE) -j$(NPROC) -C $(CORE) clean

.PHONY: all clean

define echo_c
    @echo -e "\033[1;33m$(1)\033[0m"
endef
