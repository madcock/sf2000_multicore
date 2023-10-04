#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "libretro.h"
#include "file/file_path.h"
#include "file/config_file.h"

#include "core_api.h"
#include "debug.h"
#include "stockfw.h"

#define MAXPATH 	255
#define SYSTEM_DIRECTORY	"/mnt/sda1/bios"
#define SAVE_DIRECTORY		"/mnt/sda1/ROMS/save"
#define CONFIG_DIRECTORY	"/mnt/sda1/cores/config"

// this is needed when linking with a c++ project that have static objects with custom destructors
void *__dso_handle = 0;

static config_file_t *s_core_config = NULL;
static void config_load();
static void config_free();
static bool config_get_var(struct retro_variable *var);

static retro_environment_t environ_cb;

static void wrap_retro_set_environment(retro_environment_t cb);
static bool wrap_retro_load_game(const struct retro_game_info* info);
static bool wrap_environ_cb(unsigned cmd, void *data);
static void wrap_retro_init(void);
static void wrap_retro_deinit(void);

static void log_cb(enum retro_log_level level, const char *fmt, ...);

static const char *s_game_filepath = NULL;
static int state_load(const char *frontend_state_filepath);
static int state_save(const char *frontend_state_filepath);

struct retro_core_t core_exports = {
   .retro_init = wrap_retro_init,
   .retro_deinit = wrap_retro_deinit,
   .retro_api_version = retro_api_version,
   .retro_get_system_info = retro_get_system_info,
   .retro_get_system_av_info = retro_get_system_av_info,
   .retro_set_environment = wrap_retro_set_environment,
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

	// no need to strcpy, because info->path string should be valid during the execution of the game
	s_game_filepath = info->path;

	// setup load/save state handlers
	gfn_state_load = state_load;
	gfn_state_save = state_save;

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

void wrap_retro_set_environment(retro_environment_t cb)
{
	environ_cb = cb;
	retro_set_environment(wrap_environ_cb);
}

bool wrap_environ_cb(unsigned cmd, void *data)
{
	switch (cmd)
	{
		case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
		{
			struct retro_log_callback *cb = (struct retro_log_callback*)data;
			cb->log = log_cb;
			return true;
		}

		case RETRO_ENVIRONMENT_SET_MESSAGE:
		case RETRO_ENVIRONMENT_SET_MESSAGE_EXT:
		{
			const struct retro_message *msg = (const struct retro_message*)data;
			log_cb(RETRO_LOG_INFO, "[Environ]: SET_MESSAGE: %s\n", msg->msg);
			return true;
		}

		case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
		{
			const char *dir = SYSTEM_DIRECTORY;
			*(const char**)data = dir;
			log_cb(RETRO_LOG_INFO, "[Environ]: SYSTEM_DIRECTORY: \"%s\"\n", dir);
			return true;
		}

		case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
		{
			const char *dir = SAVE_DIRECTORY;
			*(const char**)data = dir;
			log_cb(RETRO_LOG_INFO, "[Environ]: SAVE_DIRECTORY: \"%s\"\n", dir);
			return true;
		}

		case RETRO_ENVIRONMENT_GET_CAN_DUPE:
			*(bool*)data = true;
			log_cb(RETRO_LOG_INFO, "[Environ]: GET_CAN_DUPE: true\n");
			return true;

		case RETRO_ENVIRONMENT_GET_VARIABLE:
		{
			struct retro_variable *var = (struct retro_variable*)data;
			bool ret = config_get_var(var);
			log_cb(RETRO_LOG_INFO, "[Environ]: GET_VARIABLE: %s=%s\n", var->key, ret ? var->value : "");
			return ret;
		}

		case RETRO_ENVIRONMENT_SET_MEMORY_MAPS:
		{
			log_cb(RETRO_LOG_INFO, "[Environ]: SET_MEMORY_MAPS\n");
			break;
		}
		case RETRO_ENVIRONMENT_SET_GEOMETRY:
		{
			const struct retro_game_geometry *geom = (const struct retro_game_geometry*)data;
            log_cb(RETRO_LOG_INFO, "[Environ]: SET_GEOMETRY: %ux%u, Aspect: %.3f.\n",
				geom->base_width, geom->base_height, geom->aspect_ratio);
			break;
		}
	}
	return environ_cb(cmd, data);
}

void log_cb(enum retro_log_level level, const char *fmt, ...)
{
	char buffer[500];

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	switch (level)
	{
		case RETRO_LOG_DEBUG:
			xlog("[core][DEBUG] %s", buffer);
			break;

		case RETRO_LOG_INFO:
			xlog("[core][INFO] %s", buffer);
			break;

		case RETRO_LOG_WARN:
			xlog("[core][WARN] %s", buffer);
			break;

		case RETRO_LOG_ERROR:
			xlog("[core][ERROR] %s", buffer);
			break;

		default:
			break;
	}
}


void build_state_filepath(char *state_filepath, size_t size, const char *game_filepath, const char *frontend_state_filepath)
{
//	"/mnt/sda1/ROMS/pce/Alien Crush.pce"	->
//	"/mnt/sda1/ROMS/save/Alien Crush.state[slot]"

	// last char is the save slot number
	char save_slot = frontend_state_filepath[strlen(frontend_state_filepath) - 1];

	char basename[MAXPATH];
	fill_pathname_base(basename, game_filepath, sizeof(basename));
	path_remove_extension(basename);

	snprintf(state_filepath, size, SAVE_DIRECTORY "/%s.state%c", basename, save_slot);
}

int state_load(const char *frontend_state_filepath)
{
	char state_filepath[MAXPATH];
	build_state_filepath(state_filepath, sizeof(state_filepath), s_game_filepath, frontend_state_filepath);
	xlog("state_load: file=%s\n", state_filepath);

	FILE *file = fopen(state_filepath, "rb");
	if (!file)
		return 0;

	fseeko(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseeko(file, 0, SEEK_SET);

	void *data = malloc(size);

	fread(data, 1, size, file);
	fclose(file);

	retro_unserialize(data, size);

	free(data);

	return 1;
}

int state_save(const char *frontend_state_filepath)
{
	char state_filepath[MAXPATH];
	build_state_filepath(state_filepath, sizeof(state_filepath), s_game_filepath, frontend_state_filepath);
	xlog("state_save: file=%s\n", state_filepath);

	FILE *file = fopen(state_filepath, "wb");
	if (!file)
		return 0;

	size_t size = retro_serialize_size();
	void *data = calloc(size, 1);

	retro_serialize(data, size);

	fwrite(data, size, 1, file);
	fclose(file);

	free(data);

	fs_sync(state_filepath);

	return 1;
}

void build_config_filepath(char *filepath, size_t size)
{
	struct retro_system_info sysinfo;
	retro_get_system_info(&sysinfo);

	snprintf(filepath, size, CONFIG_DIRECTORY "/%s.opt", sysinfo.library_name);
}

void config_load()
{
	char config_filepath[MAXPATH];
	build_config_filepath(config_filepath, sizeof(config_filepath));

	s_core_config = config_file_new_from_path_to_string(config_filepath);
	xlog("config_load: %s %s\n", config_filepath, s_core_config ? "loaded" : "not found");
}

void config_free()
{
	config_file_free(s_core_config);
}

bool config_get_var(struct retro_variable *var)
{
	if (!s_core_config)
		return false;

	const struct config_entry_list *entry = config_get_entry(s_core_config, var->key);
	if (!entry)
		return false;

	var->value = entry->value;
	return true;
}

void wrap_retro_init(void)
{
	config_load();
	retro_init();
}

void wrap_retro_deinit(void)
{
	retro_deinit();
	config_free();
}
