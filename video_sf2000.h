#ifndef __SF2000_VIDEO_H
#define __SF2000_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "file/config_file.h"

#define HLD_DEV_TYPE_DIS	0x10f0000

enum tvsystem {
	PAL	= 1,
	RGB_LCD	= 0x16 // LINE_800x480_60 hacked for RGB output by HiChip
};

struct vp_init_info; // all of it's rather large thus better patched in-place

extern void get_vp_init_low_lcd_para(struct vp_init_info *);
// is_lcd is named switch_flag in debug printf (it's redunant btw)
extern void switch_lcd_or_tv(BOOL is_lcd, enum tvsystem tvsys);

// struct vp_rgb_timing_param.rgb_clock
#define VPO_RGB_CLOCK_6_6M	8
#define VPO_RGB_CLOCK_9M	9

#define VPO_IO_GET_OUT_MODE	3

extern int vpo_ioctl(HANDLE, uint32_t, uintptr_t);

#define HLD_DEV_TYPE_OSD	0x1030000

struct osdrect {
	uint16_t u_left;
	uint16_t u_top;
	uint16_t u_width;
	uint16_t u_height;
};

#define OSD_HD_RGB565	5
#define OSD_HD_ARGB8888	10 // currently unused (needs a run_emulator_menu hook)

struct osdpara {
	int e_mode;
	uint8_t u_galpha_enable;
	uint8_t u_galpha;
	uint8_t u_pallette_sel;
	uint8_t u_rotate; // hangs the driver, needs serious research to fix
	uint8_t u_flip_v;
	uint8_t u_flip_h;
};

struct osd_vscr {
	struct osdrect v_r;
	const void *lpb_scr;
	uint8_t	b_block_id;
	BOOL update_pending;
	uint8_t b_color_mode;
	uint8_t b_draw_mode;
};

extern int osddrv_open(HANDLE, void *);
extern int osddrv_close(HANDLE);
extern int osddrv_3x_create_region(HANDLE, uint8_t, struct osdrect *, struct osdpara *);
extern int osddrv_3x_region_write(HANDLE, uint8_t, struct osd_vscr *, struct osdrect *);

#define OSD_SET_SCALE_MODE	0x0B
#define OSD_SCALE_WITH_PARAM	0x0C

extern int osddrv_scale(HANDLE, uint32_t, uintptr_t);

extern void st7789v_caset_raset(unsigned start_column, unsigned start_row);
extern void st7789v_ramwr(void);

extern void run_osd_region_write(const void *, uint16_t, uint16_t, uint16_t);
extern void run_screen_write(const void *, int width, int height, uint pitch);


void video_options(config_file_t *config);
void video_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
