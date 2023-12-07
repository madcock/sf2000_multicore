/* libc */
strcat	= 0x801b0b28;
strncmp	= 0x801b0bc4;
strncpy	= 0x801b0c9c;
strchr	= 0x801b0d6c;
strrchr	= 0x801b0dfc;
strstr	= 0x801b0e1c;
strcasecmp	= 0x801b0e70;
strncasecmp	= 0x801b0ef0;
memchr	= 0x801b0f50;
tolower	= 0x801b0f80;
toupper	= 0x801b0fa0;
isalnum	= 0x801b0fc0;
isalpha	= 0x801b0fd8;
isascii	= 0x801b0ff0;
iscntrl	= 0x801b0ff8;
isdigit	= 0x801b1010;
isgraph	= 0x801b1028;
islower	= 0x801b1040;
isprint	= 0x801b1058;
ispunct	= 0x801b1070;
isspace	= 0x801b1088;
isupper	= 0x801b10a0;
isxdigit	= 0x801b10b8;
toascii	= 0x801b10d0;
_tolower	= 0x801b10d8;
_toupper	= 0x801b10e0;
isnan	= 0x801b10e8;
isinf	= 0x801b1134;
strtol	= 0x801b1184;
strtoul	= 0x801b1374;

sbrk    = 0x8028b5f8;	/* TODO: exposing and using sbrk might be dangerous? */

malloc	= 0x8028b8b8;
free	= 0x8028c4c8;
realloc	= 0x8028c58c;
calloc	= 0x8028c844;

vsnprintf	= 0x8028e2d8;
sprintf	= 0x8028e38c;
vprintf	= 0x8028e434;
printf	= 0x8028e474;
snprintf	= 0x8028e49c;
vsprintf	= 0x8028e4bc;
rand	= 0x8028e56c;
memcpy	= 0x8028e620;
memset	= 0x8028e850;
memcmp	= 0x8028e930;
memmove	= 0x8028e994;
strcpy	= 0x8028ea60;
strcmp	= 0x8028eaa0;
strlen	= 0x8028eae4;
atoi	= 0x8028eb0c;
time	= 0x8028edb4;
localtime	= 0x8028effc;
vsscanf	= 0x802903d0;
sscanf	= 0x80290410;

/* these are actually fs_ variants but ABI is the same */
open	= 0x802a5a0c;
read	= 0x802a5e04;
write	= 0x802a5f28;
lseek	= 0x802a6048;
close	= 0x802a6188;

fs_fstat = 0x802a5d34;
fs_stat  = 0x802a5d58;
fs_mkdir = 0x802a5b68;

fs_opendir  = 0x802a5adc;
fs_closedir = 0x802a61a4;
fs_readdir  = 0x802a60ec;

fs_sync = 0x802a58e4;

strdup	= 0x802acb40; /* only needed for vfs it seems */
fclose	= 0x802acbf4;
fflush	= 0x802acd64; /* actually __fflush but sufficiently close */
fopen	= 0x802ad1d8;
fw_fread = 0x802ad34c;
fseeko	= 0x802ad4b8;
ftell	= 0x802adbd0;
ftello	= 0x802adbec;
fwrite	= 0x802adf60;
fgets	= 0x802acec0;

/* TDS2 */
osal_tds2_cache_flush	= 0x80049214;
os_disable_interrupt	= 0x802da404;
os_enable_interrupt		= 0x802da42c;

os_get_tick_count = 0x80309b7c;

dly_tsk	= 0x80309134;

/* libretro frontend */
retro_video_refresh_cb	= 0x803583a8;
retro_audio_sample_batch_cb	= 0x80358430;
retro_input_poll_cb	= 0x80358688;
retro_input_state_cb	= 0x80358778;
retro_environment_cb	= 0x803587bc;

run_emulator	= 0x803589a4;
run_gba			= 0x80359d1c;

/* .rodata */

_ctype_	= 0x8099553c;

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
