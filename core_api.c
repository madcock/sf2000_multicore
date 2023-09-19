#include "libretro.h"
#include "core_api.h"
#include <string.h>

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
   .retro_load_game = retro_load_game,
   .retro_load_game_special = retro_load_game_special,
   .retro_unload_game = retro_unload_game,
   .retro_get_region = retro_get_region,
   .retro_get_memory_data = retro_get_memory_data,
   .retro_get_memory_size = retro_get_memory_size,
};

static inline void clear_bss()
{
	extern void *__bss_start;
	extern void *_end;

    void *start = &__bss_start;
    void *end = &_end;

	memset(start, 0, end - start);
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
	return &core_exports;
}
