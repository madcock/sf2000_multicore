#ifndef __STOCKFW_H
#define __STOCKFW_H

#include <stdbool.h>
#include <stdint.h>

typedef int BOOL;

extern size_t fw_fread(void *ptr, size_t size, size_t count, FILE *stream);

extern int fs_stat(const char *path, void *sbuf);
extern int fs_fstat(int fd, void *sbuf);
extern int fs_sync(const char *path);
extern int fs_mkdir(const char *path, int mode);

extern int fs_opendir(const char *path);
extern int fs_closedir(int fd);
extern ssize_t fs_readdir(int fd, void *buffer);

extern void osal_tds2_cache_flush(void *buf, unsigned sz);
extern int dly_tsk(unsigned ms);
extern void os_disable_interrupt(void);
extern uint32_t os_get_tick_count(void);

extern int run_emulator(int load_state);
extern void run_gba(const char *filename, int load_state);

extern void retro_video_refresh_cb(const void *data, unsigned width, unsigned height, size_t pitch);
extern size_t retro_audio_sample_batch_cb(const int16_t *data, size_t frames);
extern void retro_input_poll_cb(void);
extern int16_t retro_input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id);
extern bool retro_environment_cb(unsigned cmd, void *data);

/* .data */

struct retro_system_av_info;
struct retro_game_info;

extern unsigned RAMSIZE;

extern int (* gfn_state_load)(const char *);
extern int (* gfn_state_save)(const char *);

extern unsigned (* gfn_retro_get_region)(void);
extern void (* gfn_get_system_av_info)(struct retro_system_av_info *info);
extern bool (* gfn_retro_load_game)(const struct retro_game_info *game);
extern void (* gfn_retro_unload_game)(void);
extern void (* gfn_retro_run)(void);
extern void (* gfn_frameskip)(BOOL flag);

extern int g_snd_task_flags;

extern struct retro_game_info g_retro_game_info;
extern const unsigned g_run_file_size;
extern void * gp_buf_64m;

#endif
