NPROC=$(shell nproc)
SHELL:=/bin/bash

MIPS=/opt/mips32-mti-elf/2019.09-03-2/bin/mips-mti-elf-
MIPS2=/home/icemano/x-tools/mipsel-unknown-elf/bin/mips-mti-elf-

CC = $(MIPS)gcc
LD = $(MIPS2)ld
OBJCOPY = $(MIPS)objcopy

CFLAGS := -EL -march=mips32 -mtune=mips32 -msoft-float
CFLAGS += -Os -G0 -mno-abicalls -fno-pic -ffreestanding

LDFLAGS := -EL -nostdlib -z max-page-size=32
LDFLAGS += --gc-sections

LDFLAGS += -L$(abspath $(dir $(shell $(CC) $(CFLAGS) -print-file-name=libgcc.a)))
LIBS+=-lgcc

LDFLAGS += -L$(abspath $(dir $(shell $(CC) $(CFLAGS) -print-file-name=libc.a)))
LIBS+=-lc -lm

LIBRETRO_COMM_DIR=$(abspath libs/libretro-common)
export LIBRETRO_COMM_DIR

CORE_OBJS=core_api.o lib.o debug.o
LOADER_OBJS=init.o main.o debug.o

# CORE=cores/gpsp
CORE=cores/snes9x2005
# CORE=cores/snes9x2010
# CORE=cores/snes9x2002

# Default target
all: core_87000000 bisrv.asd install

libretro_core:
	$(call echo_c,"compiling $(CORE)")
	$(MAKE) -j$(NPROC) -C $(CORE) platform=sf2000

libretro_core.a: libretro_core
	cp -u $(CORE)/*.a libretro_core.a

libretro-common:
	$(call echo_c,"compiling $@")
	$(MAKE) -j$(NPROC) -C libs/libretro-common

libretro-common.a: libretro-common
	cp -u libs/libretro-common/$@ $@

core_api.o: core_api.c
	$(CC) $(CFLAGS) -Ilibs/libretro-common/include -o $@ -c $<

lib.o: lib.c
	$(CC) $(CFLAGS) -o $@ -c $<

debug.o: debug.c
	$(CC) $(CFLAGS) -o $@ -c $<

core.elf: libretro_core.a libretro-common.a $(CORE_OBJS)
	$(call echo_c,"compiling $@")
	$(LD) -Map $@.map $(LDFLAGS) -e __core_entry__ -Ttext=0x87000000 bisrv_08_03.ld -o $@ \
		--start-group $(LIBS) $(CORE_OBJS) libretro_core.a libretro-common.a --end-group

core_87000000: core.elf
	$(OBJCOPY) -O binary -j .text -j .rodata -j .data core.elf core_87000000


init.o: init.s
	$(CC) $(CFLAGS) -o $@ -c $<

main.o: main.c
	$(CC) $(CFLAGS) -Wall -I libs/libretro-common/include -o $@ -c $<

loader.elf: $(LOADER_OBJS)
	$(call echo_c,"compiling $@")
	$(LD) -Map $@.map $(LDFLAGS) -e __start -Ttext=0x800016d0 bisrv_08_03.ld $(LOADER_OBJS) -o loader.elf


bisrv.asd: loader.elf crc
	$(call echo_c,"patching $@")

	$(Q)cp bisrv_08_03.asd bisrv.asd

	$(Q)$(OBJCOPY) -O binary -j .text -j .rodata -j .data loader.elf loader.bin

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
	-$(call copy_if_updated,bisrv.asd,sdcard/bios/bisrv.asd)
	-$(call copy_if_updated,core_87000000,sdcard/core_87000000)

# Clean intermediate files and the final executable
clean:
	-rm $(CORE_OBJS)
	-rm $(LOADER_OBJS)
	-rm loader.elf loader.bin core.elf core_87000000
	-rm bisrv.asd
	-rm libretro_core.a
	$(MAKE) -j$(NPROC) -C $(CORE) clean platform=sf2000

.PHONY: all clean

define echo_c
    @echo -e "\033[1;33m$(1)\033[0m"
endef

define copy_if_updated
	diff -q $(1) $(2) || (cp $(1) $(2) && echo "$(1) updated")
endef
