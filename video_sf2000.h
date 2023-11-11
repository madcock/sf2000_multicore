#ifndef __SF2000_VIDEO_H
#define __SF2000_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "file/config_file.h"


#define HLD_DEV_TYPE_DIS 0x10f0000

// struct vp_rgb_timing_param.rgb_clock
#define VPO_RGB_CLOCK_6_6M	8
#define VPO_RGB_CLOCK_9M	9

#define VPO_IO_GET_OUT_MODE 3

#define RGB_LCD 0x16 // LINE_800x480_60 in enum tvsystem hacked for RGB by HiChip

extern int vpo_ioctl(HANDLE, uint32_t, uintptr_t);

#define HLD_DEV_TYPE_OSD 0x1030000

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
	uint8_t u_rotate; // hangs the driver
	uint8_t u_flip_v;
	uint8_t u_flip_h;
};

struct osd_vscr {
	struct osdrect	v_r;
	void *	lpb_scr;
	uint8_t	b_block_id;
	BOOL	update_pending;
	uint8_t	b_color_mode;
	uint8_t	b_draw_mode;
};

extern int osddrv_open(HANDLE, void *);
extern int osddrv_close(HANDLE);
extern int osddrv_3x_create_region(HANDLE, uint8_t, struct osdrect *, struct osdpara *);
extern int osddrv_3x_region_write(HANDLE, uint8_t, struct osd_vscr *, struct osdrect *);

#define OSD_SET_SCALE_MODE	0x0B
#define OSD_SCALE_WITH_PARAM	0x0C

extern int osddrv_scale(HANDLE, uint32_t, uintptr_t);

extern void switch_lcd_or_tv(int is_lcd, int tvsys);

extern void run_osd_region_write(void *, uint16_t, uint16_t, uint16_t);


void video_options(config_file_t *config);
void video_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
