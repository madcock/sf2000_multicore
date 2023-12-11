
fs_open	= 0x802a5a0c;
fs_read	= 0x802a5e04;
fs_write = 0x802a5f28;
fs_lseek = 0x802a6048;
fs_close = 0x802a6188;

fs_access = 0x802a5c9c;

fs_fstat = 0x802a5d34;
fs_stat  = 0x802a5d58;
fs_mkdir = 0x802a5b68;

fs_opendir  = 0x802a5adc;
fs_closedir = 0x802a61a4;
fs_readdir  = 0x802a60ec;

fs_sync = 0x802a58e4;

/* TDS2 */
osal_tds2_cache_flush	= 0x80049214;
os_disable_interrupt	= 0x802da404;
os_enable_interrupt		= 0x802da42c;

os_get_tick_count = 0x80309b7c;

dly_tsk	= 0x80309134;

/* drivers */
get_vp_init_low_lcd_para	= 0x801b9d0c;
switch_lcd_or_tv	= 0x801b9dd0;

vpo_ioctl	= 0x80271314;

osddrv_open	= 0x802719c8;
osddrv_close	= 0x80271aa8;
osddrv_3x_create_region	= 0x802721bc;
osddrv_3x_region_write	= 0x80272ab4;
osddrv_scale	= 0x80272dd8;

dev_get_by_id	= 0x802730b0;

st7789v_caset_raset	= 0x8029a69c;
st7789v_ramwr	= 0x8029a740;

/* libretro frontend */
run_osd_region_write	= 0x80355fdc;
run_screen_write	= 0x80356058;
run_sound_advance	= 0x80356168;

retro_video_refresh_cb		= 0x803583a8;
retro_audio_sample_batch_cb	= 0x80358430;
retro_input_poll_cb			= 0x80358688;
retro_input_state_cb 		= 0x80358778;
retro_environment_cb		= 0x803587bc;

run_emulator	= 0x803589a4;
run_gba			= 0x80359d1c;

/* .data */
RAMSIZE	= 0x80c09bdc;

g_errno	= 0x80c0a3b0;

g_snd_task_flags	= 0x80c0b574;

g_retro_game_info	= 0x80c0b678;

gfn_state_load	= 0x80c107cc;
gfn_state_save	= 0x80c1081c;

gfn_retro_get_region	= 0x80c107f8;
gfn_get_system_av_info	= 0x80c10808;
gfn_retro_load_game		= 0x80c10828;
gfn_retro_unload_game	= 0x80c10830;
gfn_retro_run	= 0x80c10840;
gfn_frameskip	= 0x80c1083c;

g_run_file_size	= 0x80c107d8;
gp_buf_64m	= 0x80c10834;

/* HiChip's hardware wired to LCD for UARTless debug.c */
PINMUXL	= 0xb88004a0;
PINMUXT	= 0xb8800500;
GPIOLCTRL	= 0xb8800044;
GPIOTCTRL	= 0xb8800344;
