#ifndef __STOCKFW_H
#define __STOCKFW_H

extern int fs_sync(const char *path);

extern void osal_tds2_cache_flush(void *buf, unsigned sz);
extern int dly_tsk(unsigned ms);

extern int run_emulator(int load_state);

extern void retro_video_refresh_cb(const void *data, unsigned width, unsigned height, size_t pitch);
extern size_t retro_audio_sample_batch_cb(const int16_t *data, size_t frames);
extern void retro_input_poll_cb(void);
extern int16_t retro_input_state_cb(unsigned port, unsigned device, unsigned index, unsigned id);
extern bool retro_set_environment_cb(unsigned cmd, void *data);

/* .data */

struct retro_system_av_info;
struct retro_game_info;

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

#endif
