/* Copyright (C) 2023 Nikita Burnashev

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted.

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND! */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef uint32_t HANDLE;
typedef int BOOL;
typedef unsigned long TMO;

extern void st7789v_restart_frame(void);
extern void os_disable_interrupt(void);
extern void os_enable_interrupt(void);
extern void dly_tsk(TMO);

extern volatile struct {
	uint32_t ier[3];
	uint32_t input_st;
	uint32_t output_val;
	uint32_t dir;
	uint32_t isr;
} GPIOLCTRL;

void realign_lcd(void)
{
	os_disable_interrupt();
	GPIOLCTRL.dir &= ~(1 << 8); // L08 (VSync) input
	while ((GPIOLCTRL.input_st & (1 << 8)) != 0) ;
	while ((GPIOLCTRL.input_st & (1 << 8)) == 0) ;
	// rising VSync edge
	st7789v_restart_frame();
	os_enable_interrupt();
}

void hook_lcd_init(void)
{
	st7789v_restart_frame(); // sets active-polarity = <0>
	dly_tsk(20); // wait at least one full frame
	realign_lcd();
}

extern void switch_tv_mode(int, int);

static enum tvsystem {
	PAL,
	NTSC,
	RGB_LCD = 0x16 // LINE_800x480_60 hacked by HiChip
} cur_tvsys = RGB_LCD;

void hook_switch_tv(enum tvsystem tvsys, int b_progressive)
{
	cur_tvsys = tvsys; // must be set first (multithreading)
	switch_tv_mode(tvsys, b_progressive);
	if (cur_tvsys == RGB_LCD) realign_lcd();
}
/*
static enum {
	CORE_PROVIDED,
	USER_DEFINED,
	SQUARE_PIXELS
} scaling_mode = CORE_PROVIDED;
static float scaling_user = 4.0f / 3.0f;
static int scaling_filtered = 1;

extern void api_audio_set_mute(int);

void load_scaling_options(void)
{
	// automatic string suffix merging failed to work for me
	static const char opt_file_name[] = "/mnt/sda1/cores/config/sf2000_scaling.opt";
	const int prefix1 = sizeof "/mnt/sda1/cores/config/" - 1;
	const int prefix2 = sizeof "sf2000_scaling_" - 1;
	FILE *pf = fopen(opt_file_name, "rb");
	// rb is intentional to save on an entry in the string table
	char s[99];

	if (pf == NULL) return;
	while (fgets(s, sizeof s, pf) != NULL) {
		unsigned short name = s[prefix2] | s[prefix2 + 1] << 8;
		char *pval = strstr(s, "= \"");

		// rules out comments, too
		if (memcmp(s, opt_file_name + prefix1, prefix2 - 1) != 0) continue;
		if (pval == NULL) continue;
		pval += 3;
		if (name == ('m' | 'o' << 8)) {
			if (pval[0] == 'c')
				scaling_mode = CORE_PROVIDED;
			else if (pval[0] == 'u')
				scaling_mode = USER_DEFINED;
			else
				scaling_mode = SQUARE_PIXELS;
		}
		else if (name == ('u' | 's' << 8)) // user_ratio
			sscanf(pval, "%f", &scaling_user);
		else if (name == ('f' | 'i' << 8))
			scaling_filtered = atoi(pval);
		else if (name == ('m' | 'u' << 8)) {
			if (pval[1] == 'n') api_audio_set_mute(1); // on
		}
	}
	fclose(pf);
}
*/
#define HLD_DEV_TYPE_OSD 0x1030000

static HANDLE m_osd_dev;

extern HANDLE dev_get_by_id(uint32_t, uint16_t);

struct osdrect {
	uint16_t u_left;
	uint16_t u_top;
	uint16_t u_width;
	uint16_t u_height;
};

extern int osddrv_open(HANDLE, void *);
extern int osddrv_close(HANDLE);
extern int osddrv_3x_create_region(HANDLE, uint8_t, struct osdrect *, void *); // osdpara
extern int osddrv_3x_region_write(HANDLE, uint8_t, void *, struct osdrect *); // osd_vscr
extern int osddrv_scale(HANDLE, uint32_t, uintptr_t);

#define OSD_HD_RGB565	5
#define OSD_HD_ARGB8888	10

#define OSD_SET_SCALE_MODE	0x0B
#define OSD_SCALE_WITH_PARAM	0x0C

static void region_create(uint16_t width, uint16_t height)
{
	struct osdrect r = { 0, 0, width, height };
	struct osdpara {
		int e_mode;
		uint8_t u_galpha_enable;
		uint8_t u_galpha;
		uint8_t u_pallette_sel;
		uint8_t u_rotate; // hangs the driver
		uint8_t u_flip_v;
		uint8_t u_flip_h;
	} para = { OSD_HD_RGB565 };
	struct osd_scale_param {
		uint16_t /* sic */ tv_sys;
		uint16_t h_div;
		uint16_t v_div;
		uint16_t h_mul;
		uint16_t v_mul;
	} scale_param = { cur_tvsys, width, height };

	if (!m_osd_dev) m_osd_dev = dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
	osddrv_close(m_osd_dev);
	osddrv_open(m_osd_dev, &para);
	dly_tsk(20); // wait at least one full frame

	// TODO
	if (cur_tvsys == RGB_LCD) {// && scaling_mode == SQUARE_PIXELS) {
		scale_param.h_mul = 240;
		scale_param.v_mul = 320;
	}
	else {
		scale_param.h_mul = 720;
		scale_param.v_mul = cur_tvsys == PAL ? 576 : 480;
	}

	osddrv_3x_create_region(m_osd_dev, 0, &r, &para);
	osddrv_scale(m_osd_dev, OSD_SCALE_WITH_PARAM, (uintptr_t)&scale_param);
	osddrv_scale(m_osd_dev, OSD_SET_SCALE_MODE, 1); // TODO scaling_filtered);
}

static void region_write(void *buf, uint16_t width, uint16_t height, uint16_t pixel_pitch)
{
	struct osd_vscr {
		struct osdrect	v_r;
		void *	lpb_scr;
		uint8_t	b_block_id;
		BOOL	update_pending;
		uint8_t	b_color_mode;
		uint8_t	b_draw_mode;
	} vscr = { 0, 0, pixel_pitch, height, buf };
	struct osdrect r = { 0, 0, width, height };

	vscr.b_color_mode = OSD_HD_RGB565;
	osddrv_3x_region_write(m_osd_dev, 0, &vscr, &r);
}

void hook_rotate(void *buf, uint16_t width, uint16_t height, uint16_t pixel_pitch)
{
	static uint16_t cur_width, cur_height, *rot_buf = NULL;

	if (rot_buf == NULL) {
		rot_buf = malloc(480 * 640 * 2);
		// TODO load_scaling_options(); // not the best place but will suffice
	}
	if (cur_tvsys == RGB_LCD) {
		uint16_t *row = buf;
		unsigned x, y;

		for (y = height; y > 0; y--) {
			uint16_t *src = row, *dst = rot_buf + y - 1; // column
			for (x = width; x > 0; x--) {
				*dst = *src++;
				dst += height;
			}
			row += pixel_pitch;
		}
		buf = rot_buf;
		pixel_pitch = height;
		height = width;
		width = pixel_pitch;
	}
	if (cur_width != width || cur_height != height) {
		cur_width = width;
		cur_height = height;
		region_create(width, height);
	}
	region_write(buf, width, height, pixel_pitch);
}
