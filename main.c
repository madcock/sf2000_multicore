/* Copyright (C) 2023 Nikita Burnashev

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted.

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND! */

#include <stdio.h>
#include <string.h>
#include "libretro.h"
#include "core_api.h"

extern void osal_tds2_cache_flush(void *buf, unsigned sz);
extern int dly_tsk(unsigned ms);

extern int run_emulator(int load_state);

extern void retro_video_refresh_cb(const void *data,
	unsigned width, unsigned height, size_t pitch);
extern size_t retro_audio_sample_batch_cb(const int16_t *data, size_t frames);
extern void retro_input_poll_cb(void);
extern int16_t retro_input_state_cb(unsigned port, unsigned device,
	unsigned index, unsigned id);
extern bool retro_set_environment_cb(unsigned cmd, void *data);

/* .data */

extern unsigned RAMSIZE;

extern int (* gfn_state_load)(const char *);
extern int (* gfn_state_save)(const char *);

extern unsigned (* gfn_retro_get_region)(void);
extern void (* gfn_get_system_av_info)(struct retro_system_av_info *info);
extern bool (* gfn_retro_load_game)(const struct retro_game_info *game);
extern void (* gfn_retro_deinit)(void);
extern void (* gfn_retro_run)(void);

extern int g_snd_task_flags;

extern struct retro_game_info g_retro_game_info;
extern const unsigned g_run_file_size;
extern void * gp_buf_64m;

static int state_stub(const char *path) {
	return 1;
}

void load_and_run_core(const char *file_path, int load_state)
{
	void *core_load_addr = (void*)0x87000000;

	FILE *pf;
	size_t core_size;
	static char m_file_path[256];

	/* wait for the sound thread to exit, replicated in all run_... functions */
	g_snd_task_flags = g_snd_task_flags & 0xfffe;
	while (g_snd_task_flags != 0) {
		dly_tsk(1);
	}

	/* FIXME! all of it!! */
	RAMSIZE = 0x87000000;

	pf = fopen("/mnt/sda1/core_87000000", "rb");
	fseeko(pf, 0, SEEK_END);
	core_size = ftell(pf);
	fseeko(pf, 0, SEEK_SET);
	fread(core_load_addr, 1, core_size, pf);
	fclose(pf);

	osal_tds2_cache_flush(core_load_addr, core_size);
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

	core_api->retro_init();

	strncpy(m_file_path, file_path, sizeof m_file_path);
	g_retro_game_info.path = m_file_path;
	g_retro_game_info.data = gp_buf_64m;
	g_retro_game_info.size = g_run_file_size;

	gfn_retro_get_region	= core_api->retro_get_region;
	gfn_get_system_av_info	= core_api->retro_get_system_av_info;
	gfn_retro_load_game	= core_api->retro_load_game;
	gfn_retro_deinit	= core_api->retro_deinit;
	gfn_retro_run	= core_api->retro_run;

	run_emulator(load_state);
}

/* FIXME gets repetitive but we really need this $ra (in lib.c, too) */
extern int lcd_bsod(const char *msg);

void hook_sys_watchdog_reboot(void)
{
	unsigned ra;
	char msg[99];

	asm volatile ("move %0, $ra" : "=r" (ra));
	snprintf(msg, sizeof msg, "sys_watchdog_reboot() called from 0x%08x", ra);
	lcd_bsod(msg);
}

void hook_exception_handler(unsigned code)
{
	unsigned ra;
	char msg[99];

	asm volatile ("move %0, $ra" : "=r" (ra));
	snprintf(msg, sizeof msg, "exception code %d at 0x%08x", code, ra);
	lcd_bsod(msg);
}
