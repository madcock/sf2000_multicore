#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "libretro.h"
#include "core_api.h"
#include "debug.h"

bool wrap_retro_load_game(const struct retro_game_info* info);

struct retro_core_t core_exports = {
   .retro_init = retro_init,
   .retro_deinit = retro_deinit,
   .retro_api_version = retro_api_version,
   .retro_get_system_info = retro_get_system_info,
   .retro_get_system_av_info = retro_get_system_av_info,
   .retro_set_environment = retro_set_environment,
   .retro_set_video_refresh = retro_set_video_refresh,
   .retro_set_audio_sample = retro_set_audio_sample,
   .retro_set_audio_sample_batch = retro_set_audio_sample_batch,
   .retro_set_input_poll = retro_set_input_poll,
   .retro_set_input_state = retro_set_input_state,
   .retro_set_controller_port_device = retro_set_controller_port_device,
   .retro_reset = retro_reset,
   .retro_run = retro_run,
   .retro_serialize_size = retro_serialize_size,
   .retro_serialize = retro_serialize,
   .retro_unserialize = retro_unserialize,
   .retro_cheat_reset = retro_cheat_reset,
   .retro_cheat_set = retro_cheat_set,
   .retro_load_game = wrap_retro_load_game,
   .retro_load_game_special = retro_load_game_special,
   .retro_unload_game = retro_unload_game,
   .retro_get_region = retro_get_region,
   .retro_get_memory_data = retro_get_memory_data,
   .retro_get_memory_size = retro_get_memory_size,
};

static void clear_bss()
{
	extern void *__bss_start;
	extern void *_end;

    void *start = &__bss_start;
    void *end = &_end;

	memset(start, 0, end - start);
}

static void call_ctors()
{
	typedef void (*func_ptr) (void);
	extern func_ptr __init_array_start;
	extern func_ptr __init_array_end;

	xlog("call_ctors: start=%p end=%p\n", &__init_array_start, &__init_array_end);

	// call ctors from last to first
	for (func_ptr *pfunc = &__init_array_end - 1; pfunc >= &__init_array_start; --pfunc) {
		xlog("pfunc=%p func=%p\n", pfunc, *pfunc);
		(*pfunc)();
	}
}

// TODO: need a place to call dtors as well. maybe when retro_deinit is called?
static void call_dtors()
{
	typedef void (*func_ptr) (void);
	extern func_ptr __fini_array_start;
	extern func_ptr __fini_array_end;

	// dtors are called in reverse order of ctors
	for (func_ptr *pfunc = &__fini_array_start; pfunc < &__fini_array_end; ++pfunc) {
		(*pfunc)();
	}
}

// __core_entry__ must be placed at a known location in the binary (at the beginning)
// so that when the loader actually loads the binary into mem address 0x87000000,
// then __core_entry__ will be the first function there for the loader to call.
struct retro_core_t *__core_entry__(void) __attribute__((section(".text.unlikely")));
// TODO: define a special .text section for __core_entry__ function to better control
// at which address the linker places it.

struct retro_core_t *__core_entry__(void)
{
	clear_bss();
	call_ctors();
	return &core_exports;
}

bool wrap_retro_load_game(const struct retro_game_info* info)
{
	bool ret;
	struct retro_system_info sysinfo;
	retro_get_system_info(&sysinfo);

	xlog("core=%s-%s need_fullpath=%d exts=%s\n", sysinfo.library_name, sysinfo.library_version, sysinfo.need_fullpath, sysinfo.valid_extensions);

	// if core wants to load the content by itself directly from files, then let it
	if (sysinfo.need_fullpath)
	{
		xlog("core loads content directly from file\n");
		ret = retro_load_game(info);
	}
	else
	{
		// otherwise load the content into a temp buffer and pass it to the core

		FILE *hfile = fopen(info->path, "rb");
		if (!hfile) {
			xlog("[core] Error opening rom file=%s\n", info->path);
			return false;
		}

		fseeko(hfile, 0, SEEK_END);
		long size = ftell(hfile);
		fseeko(hfile, 0, SEEK_SET);

		void *buffer = malloc(size);

		fread(buffer, 1, size, hfile);
		fclose(hfile);

		struct retro_game_info gameinfo;
		gameinfo.path = info->path;
		gameinfo.data = buffer;
		gameinfo.size = size;

		xlog("game loaded into temp buffer. size=%u\n", size);

		ret = retro_load_game(&gameinfo);

		free(buffer);
	}

	xlog("retro_load_game: ret=%d\n", ret);
	return ret;
}
