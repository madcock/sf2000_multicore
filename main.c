/* Copyright (C) 2023 Nikita Burnashev

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted.

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND! */

#include <stdio.h>
#include <string.h>
#include "libretro.h"
#include "core_api.h"
#include "stockfw.h"
#include "debug.h"

static void callonce_init();
static void xcache_flush(void *addr, size_t size);

static int state_stub(const char *path) {
	return 1;
}

void load_and_run_core(const char *file_path, int load_state)
{
	callonce_init();

	xlog("run file=%s\n", file_path);

	// this will show a blueish flickering at the top of the screen when loading a rom.
	// it will act as an indicator that a custom core and not a stock emulator is running.
	dbg_cls();
	dbg_show_noblock();

	void *core_load_addr = (void*)0x87000000;

	FILE *pf;
	size_t core_size;

	/* wait for the sound thread to exit, replicated in all run_... functions */
	g_snd_task_flags = g_snd_task_flags & 0xfffe;
	while (g_snd_task_flags != 0) {
		dly_tsk(1);
	}

	/* FIXME! all of it!! */
	RAMSIZE = 0x87000000;

	const char* corefile = NULL;
	{
		size_t len = strlen(file_path);
		if (file_path[len-7]=='s' &&
			file_path[len-6]=='f' &&
			file_path[len-5]=='c')
			corefile = "/mnt/sda1/cores/snes/core_87000000";
		else if (file_path[len-7]=='p' &&
			file_path[len-6]=='c' &&
			file_path[len-5]=='e')
			corefile = "/mnt/sda1/cores/pce/core_87000000";
		else if (file_path[len-7]=='g' &&
			file_path[len-6]=='b' &&
			(file_path[len-5]=='c' || file_path[len-5]=='_'))
			corefile = "/mnt/sda1/cores/gb/core_87000000";
		else
			corefile = "/mnt/sda1/cores/gba/core_87000000";
	}

	xlog("core=%s\n", corefile);

	pf = fopen(corefile, "rb");
	fseeko(pf, 0, SEEK_END);
	core_size = ftell(pf);
	fseeko(pf, 0, SEEK_SET);
	fread(core_load_addr, 1, core_size, pf);
	fclose(pf);

	xlog("core loaded\n");

	xcache_flush(core_load_addr, core_size);

	xlog("cache flushed\n");

	//osal_tds2_cache_flush(core_load_addr, core_size);
	/* TODO I-cache must be invalidated to load a different core over */

	// address of the core entry function resides at the begining of the loaded core
	core_entry_t core_entry = core_load_addr;

	// the entry function clears core's .bss and return the core's exported api
	struct retro_core_t *core_api = core_entry();

	/* TODO */
	gfn_state_load = state_stub;
	gfn_state_save = state_stub;

	core_api->retro_set_video_refresh(retro_video_refresh_cb);
	core_api->retro_set_audio_sample_batch(retro_audio_sample_batch_cb);
	core_api->retro_set_input_poll(retro_input_poll_cb);
	core_api->retro_set_input_state(retro_input_state_cb);
	core_api->retro_set_environment(retro_set_environment_cb);

	xlog("retro_init\n");
	core_api->retro_init();

	g_retro_game_info.path = file_path;
	g_retro_game_info.data = gp_buf_64m;
	g_retro_game_info.size = g_run_file_size;

	gfn_retro_get_region	= core_api->retro_get_region;
	gfn_get_system_av_info	= core_api->retro_get_system_av_info;
	gfn_retro_load_game	= core_api->retro_load_game;
	gfn_retro_deinit	= core_api->retro_deinit;
	gfn_retro_run	= core_api->retro_run;

	xlog("run_emulator(%d)\n", load_state);
	run_emulator(load_state);
	xlog("run return\n");
}

/* FIXME gets repetitive but we really need this $ra (in lib.c, too) */

void hook_sys_watchdog_reboot(void)
{
	unsigned ra;
	asm volatile ("move %0, $ra" : "=r" (ra));
	lcd_bsod("sys_watchdog_reboot() called from 0x%08x", ra);
}

void hook_exception_handler(unsigned code)
{
	unsigned ra;
	asm volatile ("move %0, $ra" : "=r" (ra));
	lcd_bsod("exception code %d at 0x%08x", code, ra);
}

static void clear_bss()
{
	extern void *__bss_start;
	extern void *_end;

    void *start = &__bss_start;
    void *end = &_end;

	memset(start, 0, end - start);
}

static void callonce_init()
{
	static bool do_init = true;
	if (!do_init)
		return;

	do_init = false;

	clear_bss();
	lcd_init();
}

static void xcache_flush(void *addr, size_t size)
{
	uintptr_t idx;
	uintptr_t begin = (uintptr_t)addr;
	uintptr_t end = begin + size - 1;
	// Index_Writeback_Inv_D
	for (idx = begin; idx <= end; idx += 16)
		asm volatile("cache 1, 0(%0); cache 1, 0(%0)" : : "r"(idx));
	// Index_Invalidate_I
	for (idx = begin; idx <= end; idx += 16)
		asm volatile("cache 0, 0(%0); cache 0, 0(%0)" : : "r"(idx));
}
