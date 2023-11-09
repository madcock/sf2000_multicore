NPROC=$(shell nproc)
SHELL:=/bin/bash

# clear the log file every boot
CLEAR_LOG_ON_BOOT = 0
# experimental tearing fix
TEARING_FIX = 0

LCDFONT_OFFSET=0x2260
LOADER_OFFSET=0x1500
LOADER_ADDR=0x80001500
LOADER_ADDR_MAX=0x80002180

MIPS=/opt/mips32-mti-elf/2019.09-03-2/bin/mips-mti-elf-

CC = $(MIPS)gcc
LD = $(MIPS)ld
OBJCOPY = $(MIPS)objcopy

CFLAGS := -EL -march=mips32 -mtune=mips32 -msoft-float
CFLAGS += -Os -G0 -mno-abicalls -fno-pic -ffreestanding
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -I libs/libretro-common/include
# CFLAGS += -Wall
ifeq ($(TEARING_FIX), 1)
CFLAGS += -DTEARING_FIX=1 -DSPACE_OPTIMIZED=1
endif
ifeq ($(CLEAR_LOG_ON_BOOT), 1)
CFLAGS += -DCLEAR_LOG_ON_BOOT=1
endif

LDFLAGS := -EL -nostdlib -z max-page-size=32
LDFLAGS += --gc-sections --build-id

LDFLAGS += -L$(abspath $(dir $(shell $(CC) $(CFLAGS) -print-file-name=libgcc.a)))
LIBS+=-lgcc

LDFLAGS += -L$(abspath $(dir $(shell $(CC) $(CFLAGS) -print-file-name=libc.a)))
LIBS+=-lc -lm
LIBS+=-lstdc++

CORE_OBJS=core_api.o lib.o debug.o
ifneq ($(TEARING_FIX), 1)
LOADER_OBJS=init.o main.o debug.o
else
LOADER_OBJS=init_tearing_fix.o main.o debug.o video.o
endif

# CORE=cores/vice
# CONSOLE=c64

# CORE=cores/2048
# MAKEFILE=-f Makefile.libretro
# CONSOLE=2048

# CORE=cores/lowres-nx/platform/LibRetro
# CONSOLE=lownx

# CORE=cores/stella2014
# CONSOLE=a26

# CORE=cores/beetle-pce-fast
# CONSOLE=pce

# CORE=cores/beetle-supergrafx
# CONSOLE=pcesgx

# CORE=cores/gambatte
# CORE=cores/tgbdual
# CONSOLE=gb

# CORE=cores/gpsp
# CONSOLE=gba

# CORE=cores/snes9x2005
# CONSOLE=snes

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	$(CC) $(CFLAGS) -o $@ -c $<

# Default target
all: core_87000000 bisrv.asd install

libretro_core:
	@$(call echo_i,"compiling $(CORE)")
	$(MAKE) -j$(NPROC) -C $(CORE) $(MAKEFILE) platform=sf2000

libretro_core.a: libretro_core
	cp $(CORE)/*.a libretro_core.a

libretro-common:
	@$(call echo_i,"compiling $@")
	$(MAKE) -j$(NPROC) -C libs/libretro-common

libretro-common.a: libretro-common
	cp -u libs/libretro-common/$@ $@

core.elf: libretro_core.a libretro-common.a $(CORE_OBJS)
ifneq ($(TEARING_FIX), 1)
	@$(call echo_i,"compiling $@")
	$(LD) -Map $@.map $(LDFLAGS) -e __core_entry__ -Ttext=0x87000000 bisrv_08_03.ld -o $@ \
		--start-group $(LIBS) $(CORE_OBJS) libretro_core.a libretro-common.a --end-group
else
	@$(call echo_i,"compiling $@ with TEARING_FIX")
	$(LD) -Map $@.map $(LDFLAGS) -e __core_entry__ -Ttext=0x87000000 bisrv_08_03_tearing_fix.ld -o $@ \
		--start-group $(LIBS) $(CORE_OBJS) libretro_core.a libretro-common.a --end-group
endif

core_87000000: core.elf
ifneq ($(TEARING_FIX), 1)
	$(OBJCOPY) -O binary -j .text -j .rodata -j .data -j .sdata -j .eh_frame -j .gcc_except_table -j .init_array -j .fini_array core.elf core_87000000
else
	$(OBJCOPY) -O binary -R .MIPS.abiflags -R .note.gnu.build-id core.elf core_87000000
endif

loader.elf: $(LOADER_OBJS)
ifneq ($(TEARING_FIX), 1)
	@$(call echo_i,"compiling $@")
	$(LD) -Map $@.map $(LDFLAGS) -e __start -Ttext=$(LOADER_ADDR) bisrv_08_03.ld $(LOADER_OBJS) -o loader.elf
else
	@$(call echo_i,"compiling $@ with TEARING_FIX")
	$(LD) -Map $@.map $(LDFLAGS) -e __start -Ttext=$(LOADER_ADDR) bisrv_08_03_tearing_fix.ld $(LOADER_OBJS) -o loader.elf
endif

loader.bin: loader.elf
	$(Q)$(OBJCOPY) -O binary -j .text -j .rodata -j .data loader.elf loader.bin

bisrv.asd: loader.bin lcd_font.bin crc
	@$(call echo_i,"patching $@")

# check that loader's .bss does not exceeds LOADER_ADDR_MAX
	@BSSEND=$(shell grep -w "_end =" loader.elf.map | awk '{print $$1}'); \
	if [ $$(($${BSSEND})) -gt $$(($(LOADER_ADDR_MAX))) ]; then \
		$(call echo_e,"error: loader is too big. \
		bss ending $${BSSEND} exceeds $(LOADER_ADDR_MAX) by \
		$$(( $${BSSEND} - $(LOADER_ADDR_MAX) )) bytes") ; \
		exit 1; \
	else \
		echo "bss ending $${BSSEND}. still $$(( $(LOADER_ADDR_MAX) - $${BSSEND} )) bytes left" ; \
	fi

# check that lcd_font.bin has the anticipated size
	@LCDFONT_SIZE=$(shell stat -c %s lcd_font.bin) ; \
	if [ $${LCDFONT_SIZE} -ne 672 ]; then \
		$(call echo_e,"error: lcd_font.bin size $${LCDFONT_SIZE}. should be 672") ; \
		exit 1; \
	fi

	$(Q)cp bisrv_08_03.asd bisrv.asd

	$(Q)dd if=loader.bin of=bisrv.asd bs=$$(($(LOADER_OFFSET))) seek=1 conv=notrunc 2>/dev/null

	dd if=lcd_font.bin of=bisrv.asd bs=$$(($(LCDFONT_OFFSET))) seek=1 conv=notrunc

	# note: this patch must match $(LOADER_ADDR)
	# jal run_gba -> jal 0x80001500
	printf "\x40\x05\x00\x0C" | dd of=bisrv.asd bs=1 seek=$$((0x35a900)) conv=notrunc

	# endless loop in sys_watchdog_reboot -> j 0x80001508
	printf "\x42\x05\x00\x08" | dd of=bisrv.asd bs=1 seek=$$((0x30d4)) conv=notrunc
	# endless loop in INT_General_Exception_Hdlr -> j 0x80001510
	printf "\x44\x05\x00\x08" | dd of=bisrv.asd bs=1 seek=$$((0x495a0)) conv=notrunc

ifeq ($(TEARING_FIX), 1)
	# jal switch_tv_mode -> hook_switch_tv
	printf "\x50\x05\x00\x0c" | dd of=bisrv.asd bs=1 seek=$$((0x1b9ec0)) conv=notrunc

	# j run_osd_region_write -> hook_rotate
	printf "\x52\x05\x00\x08" | dd of=bisrv.asd bs=1 seek=$$((0x356118)) conv=notrunc

	# jal st7789v_80i_register_vsync_isr -> hook_lcd_init
	printf "\x54\x05\x00\x0c" | dd of=bisrv.asd bs=1 seek=$$((0x29ab50)) conv=notrunc

	# get_vp_init_low_lcd_para
	# rgb_clock
	#printf "\x08\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d5c)) conv=notrunc
	printf "\x09\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d5c)) conv=notrunc
	# v_total_len
	#printf "\x30\x01" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d64)) conv=notrunc
	printf "\xdd\x01" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d64)) conv=notrunc
	# h_total_len
	#printf "\xbc\x01" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d6c)) conv=notrunc
	printf "\x46\x01" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d6c)) conv=notrunc
	# v_active_len
	#printf "\xf0\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d74)) conv=notrunc
	printf "\x40\x01" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d74)) conv=notrunc
	# h_active_len, lcd_width
	#printf "\x40\x01" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d7c)) conv=notrunc
	printf "\xf0\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d7c)) conv=notrunc
	# v_front_len
	#printf "\x12\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d84)) conv=notrunc
	# v_sync_len
	#printf "\x24\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d8c)) conv=notrunc
	# v_back_len
	#printf "\x0a\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d94)) conv=notrunc
	# h_front_len
	#printf "\x17\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9d9c)) conv=notrunc
	# h_sync_len
	#printf "\x02\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9da4)) conv=notrunc
	# h_back_len
	#printf "\x05\x00" | dd of=bisrv.asd bs=1 seek=$$((0x1b9dac)) conv=notrunc

	# m_st7789v_init
	# MADCTL
	#printf "\x60" | dd of=bisrv.asd bs=1 seek=$$((0xa1627a)) conv=notrunc
	printf "\x00" | dd of=bisrv.asd bs=1 seek=$$((0xa1627a)) conv=notrunc
	# PORCTRL
	#printf "\x0c\x00\x0c" | dd of=bisrv.asd bs=1 seek=$$((0xa1628a)) conv=notrunc
	# FRCTRL2
	#printf "\x0f" | dd of=bisrv.asd bs=1 seek=$$((0xa162ae)) conv=notrunc
	# CASET
	#printf "\x01\x00\x3f" | dd of=bisrv.asd bs=1 seek=$$((0xa162fc)) conv=notrunc
	#printf "\x00\x00\xef" | dd of=bisrv.asd bs=1 seek=$$((0xa162fc)) conv=notrunc
	# RASET
	#printf "\x00\x00\xef" | dd of=bisrv.asd bs=1 seek=$$((0xa16306)) conv=notrunc
	#printf "\x01\x00\x3f" | dd of=bisrv.asd bs=1 seek=$$((0xa16306)) conv=notrunc

	# st7789v_320_240_caset_raset -> 240_320_caset_raset
	printf "\x00\x00\x05\x24" | dd of=bisrv.asd bs=1 seek=$$((0x29a6e0)) conv=notrunc
	printf "\xef\x00\x05\x24" | dd of=bisrv.asd bs=1 seek=$$((0x29a6ec)) conv=notrunc
	printf "\x01\x00\x05\x24" | dd of=bisrv.asd bs=1 seek=$$((0x29a71c)) conv=notrunc
	printf "\x3f\x00\x05\x24" | dd of=bisrv.asd bs=1 seek=$$((0x29a728)) conv=notrunc

	# run_osd_create_region osdpara flags
	#printf "\x00\x00\x02\x24\x16\x00\xa2\xa7\x00\x00\x02\x24\x18\x00\xa2\xa7" | dd of=bisrv.asd bs=1 seek=$$((0x355e94)) conv=notrunc

	# g_run_osd_scale.h_mul, v_mul
	#printf "\x40\x01\xf0\x00" | dd of=bisrv.asd bs=1 seek=$$((0xa19212)) conv=notrunc
	#printf "\xf0\x00\x40\x01" | dd of=bisrv.asd bs=1 seek=$$((0xa19212)) conv=notrunc

	# run_osd_scale_lcd_tv v_mul, h_mul
	#printf "\xf0\x00" | dd of=bisrv.asd bs=1 seek=$$((0x355e14)) conv=notrunc
	#printf "\x40\x01" | dd of=bisrv.asd bs=1 seek=$$((0x355e14)) conv=notrunc
	#printf "\x40\x01" | dd of=bisrv.asd bs=1 seek=$$((0x355e1c)) conv=notrunc
	#printf "\xf0\x00" | dd of=bisrv.asd bs=1 seek=$$((0x355e1c)) conv=notrunc

	# remove g_run_osd_width = height = 0 from run_menu
	printf "\x00\x00\x00\x00\x00\x00\x00\x00" | dd of=bisrv.asd bs=1 seek=$$((0x350168)) conv=notrunc
endif

	$(Q)./crc bisrv.asd

lcd_font.bin: lcd_font.o
	$(OBJCOPY) -O binary -j ".rodata.lcd_font" $< $@

crc: crc.c
	gcc -o crc crc.c

install:
	@$(call echo_i,"install to sdcard")
ifeq ($(TEARING_FIX), 1)
	-$(call copy_if_updated,bisrv.asd,sdcard/bios/bisrv_tearing_fix.asd)
else
	-$(call copy_if_updated,bisrv.asd,sdcard/bios/bisrv.asd)
endif
	-$(call copy_if_updated,core_87000000,sdcard/cores/$(CONSOLE)/core_87000000)
	-rm -f sdcard/log.txt
ifeq ($(TEARING_FIX), 1)
	@$(call echo_d,"bisrv_tearing_fix")
else
	@$(call echo_d,"bisrv.asd")
endif
ifeq ($(CLEAR_LOG_ON_BOOT), 1)
	@$(call echo_d,"log cleared on boot")
else
	@$(call echo_d,"log NOT cleared on boot")
endif
	@$(call echo_d,"$(CORE)")

# Clean intermediate files and the final executable
clean:
	-rm -f $(CORE_OBJS)
	-rm -f $(LOADER_OBJS)
	-rm -f loader.elf loader.bin core.elf core_87000000
	-rm -f bisrv.asd
	-rm -f libretro_core.a
	$(MAKE) -j$(NPROC) -C $(CORE) $(MAKEFILE) clean platform=sf2000

.PHONY: all clean

define echo_i
    echo -e "\033[1;33m$(1)\033[0m"
endef

define echo_e
    echo -e "\033[1;31m$(1)\033[0m"
endef

define echo_d
    echo -e "\033[1;37m$(1)\033[0m"
endef

define copy_if_updated
	diff -q $(1) $(2) || (mkdir -p $$(dirname $(2)) && cp $(1) $(2) && echo "$(1) updated")
endef
