/* Copyright (C) 2023 Nikita Burnashev

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted.

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND! */

#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h> // fixes inttypes.h for libretro

#include "libretro.h"
#include "file/config_file.h"

#include "stockfw.h"
#include "debug.h"
#include "video.h"

// some are only used once, a temporary structure is more appropriate TODO
static enum {
	DISABLED,
	FAST,
	ROTATE // applicable ONLY to SF2000's screen FIXME
} tearing_fix = FAST;

int rgb_clock = VPO_RGB_CLOCK_9M;
unsigned h_total_len = 477, v_total_len = 326; // 9 MHz / 477 / 326 = 57.88 Hz

static enum {
	CORE_PROVIDED,
	FULL_SCREEN,
	SQUARE_PIXELS,
	USER_DEFINED
} scaling_mode = CORE_PROVIDED;
static double ratio = 4.0 / 3.0;

static bool filtered = true;

static void config_load(config_file_t *conf)
{
	const struct config_entry_list *e;

	if (!conf) return;

	e = config_get_entry(conf, "sf2000_tearing_fix");
	if (e) {
		if (strcasecmp(e->value, "disabled") == 0)
			tearing_fix = DISABLED;
		else if (strcasecmp(e->value, "fast") == 0)
			tearing_fix = FAST;
		else if (strcasecmp(e->value, "rotate") == 0)
			tearing_fix = ROTATE;
	}

	e = config_get_entry(conf, "sf2000_rgb_clock");
	if (e) {
		if (strcasecmp(e->value, "6.6 MHz") == 0)
			rgb_clock = VPO_RGB_CLOCK_6_6M;
		else if (strcasecmp(e->value, "9 MHz") == 0)
			rgb_clock = VPO_RGB_CLOCK_9M;
	}
	config_get_uint(conf, "sf2000_h_total_len", &h_total_len);
	config_get_uint(conf, "sf2000_v_total_len", &v_total_len);

	config_get_entry(conf, "sf2000_scaling_mode");
	if (e) {
		if (strcasecmp(e->value, "core_provided") == 0)
			scaling_mode = CORE_PROVIDED;
		else if (strcasecmp(e->value, "full_screen") == 0)
			scaling_mode = FULL_SCREEN;
		else if (strcasecmp(e->value, "square_pixels") == 0)
			scaling_mode = SQUARE_PIXELS;
		else if (strcasecmp(e->value, "user_defined") == 0)
			scaling_mode = USER_DEFINED;
	}
	config_get_double(conf, "sf2000_scaling_user", &ratio);

	config_get_bool(conf, "sf2000_scaling_filtered", &filtered);
}

static HANDLE m_osd_dev;

static void region_create(int tvsys, uint16_t width, uint16_t height)
{
	struct osdrect r = { 0, 0, width, height };
	struct osdpara para = { .e_mode = OSD_HD_RGB565 };
	struct {
		uint16_t tv_sys; // specifically NOT enum tvsystem
		uint16_t h_div;
		uint16_t v_div;
		uint16_t h_mul;
		uint16_t v_mul;
	} scale_param = { tvsys, width, height };

	osddrv_close(m_osd_dev);
	osddrv_open(m_osd_dev, &para);
	dly_tsk(20); // wait at least one full frame

	osddrv_3x_create_region(m_osd_dev, 0, &r, &para);

	// TODO only full screen
	if (tvsys == RGB_LCD) {
		scale_param.h_mul = tearing_fix == ROTATE ? 240 : 320;
		scale_param.v_mul = tearing_fix == ROTATE ? 320 : 240;
	}
	else {
		scale_param.h_mul = 720;
		scale_param.v_mul = tvsys ? 576 : 480;
	}
	osddrv_scale(m_osd_dev, OSD_SCALE_WITH_PARAM, (uintptr_t)&scale_param);

	osddrv_scale(m_osd_dev, OSD_SET_SCALE_MODE, filtered ? 1 : 0);
}

static void region_write(void *buf, uint16_t width, uint16_t height, uint16_t pixel_pitch)
{
	struct osd_vscr vscr = { 0, 0, pixel_pitch, height, buf };
	struct osdrect r = { 0, 0, width, height };

	vscr.b_color_mode = OSD_HD_RGB565;
	osddrv_3x_region_write(m_osd_dev, 0, &vscr, &r);
}

static HANDLE m_vpo_dev;

void apply_rgb_timings(void)
{
	int tvsys;

	vpo_ioctl(m_vpo_dev, VPO_IO_GET_OUT_MODE, (uintptr_t)&tvsys);
	if (tvsys == RGB_LCD) {
		switch_lcd_or_tv(1, RGB_LCD);
		region_create(RGB_LCD, 640, 480); // FIXME
	}
}

static uint16_t *rot_buf;

static void hook_region_write(void *buf, uint16_t width, uint16_t height, uint16_t pixel_pitch)
{
	static uint16_t cur_width, cur_height;
	int tvsys; // enum tvsystem

	vpo_ioctl(m_vpo_dev, VPO_IO_GET_OUT_MODE, (uintptr_t)&tvsys);
	if (tvsys == RGB_LCD && tearing_fix == ROTATE) {
		uint16_t *row = buf;
		unsigned x, y;

		for (y = height; y > 0; y--) {
			uint16_t *src = row, *dst = rot_buf + y - 1; // column
			for (x = width; x > 0; x--) {
				*dst = *src++;
				dst += height; // rot_buf pixel pitch
			}
			row += pixel_pitch;
		}
		buf = rot_buf;
		pixel_pitch = height; // swap width <-> height
		height = width;
		width = pixel_pitch; // in rot_buf only
	}
	if (cur_width != width || cur_height != height) {
		cur_width = width;
		cur_height = height;
		region_create(tvsys, width, height);
	}
	region_write(buf, width, height, pixel_pitch);
}

// all the patches are for the August's bisrv.asd ONLY FIXME
static void patch__get_vp_init_low_lcd_para(uint16_t rgb_clock,
	uint16_t h_total_len, uint16_t v_total_len,
	uint16_t h_active_len, uint16_t v_active_len)
{
	os_disable_interrupt(); // avoid mode switch inbetween

	*(volatile uint16_t *)0x801b9d5c = rgb_clock;
	*(volatile uint16_t *)0x801b9d64 = v_total_len;
	*(volatile uint16_t *)0x801b9d6c = h_total_len;
	*(volatile uint16_t *)0x801b9d74 = v_active_len;
	*(volatile uint16_t *)0x801b9d7c = h_active_len; // used for lcd_width, too

	__builtin___clear_cache((void *)0x801b9d0c, (void *)0x801b9dd0);

	os_enable_interrupt();
}

#define MIPS_LI_A0 (9 << 26 | 5 << 16 | 0 << 21) // addiu a0, zero

static void patch__st7789v_caset_raset(uint16_t cols, uint16_t rows)
{
	os_disable_interrupt(); // called in a gpio interrupt!

	*(volatile uint32_t *)0x8029a6e0 = MIPS_LI_A0 | cols >> 8 & 255;
	*(volatile uint32_t *)0x8029a6ec = MIPS_LI_A0 | cols & 255;
	*(volatile uint32_t *)0x8029a71c = MIPS_LI_A0 | rows >> 8 & 255;
	*(volatile uint32_t *)0x8029a728 = MIPS_LI_A0 | rows & 255;

	__builtin___clear_cache((void *)0x8029a69c, (void *)0x8029a740);

	os_enable_interrupt();
}

#define MIPS_J (2 << 26)

static void patch__run_screen_write(void *pregion_write)
{
	os_disable_interrupt(); // avoid screen writes in joy_task inbetween

	*(volatile uint32_t *)0x80356118 = MIPS_J |
		(uint32_t)pregion_write >> 2 & ((1 << 26) - 1);

	__builtin___clear_cache((void *)0x80356058, (void *)0x80356168);

	os_enable_interrupt();
}

static void lcd_memory_data_access_ctl(uint8_t data)
{
	os_disable_interrupt(); // avoid the gpio interrupt messing pinmux

	lcd_pinmux_gpio();

	lcd_send_cmd(0x36); // MADCTL
	lcd_send_data(data);

	os_enable_interrupt();
}

void video_options(config_file_t *conf)
{
	struct retro_system_av_info info;

	config_load(conf);
	if (scaling_mode == CORE_PROVIDED) {
		retro_get_system_av_info(&info);
		if (info.geometry.aspect_ratio <= 0.1)
			ratio = 1.0 * info.geometry.base_width / info.geometry.base_height;
		else
			ratio = info.geometry.aspect_ratio;
	}

	m_osd_dev = dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
	m_vpo_dev = dev_get_by_id(HLD_DEV_TYPE_DIS, 0);

	if (tearing_fix == FAST) {
		patch__get_vp_init_low_lcd_para(rgb_clock,
			h_total_len, v_total_len, 320, 240
		);
		apply_rgb_timings();
	}
	else if (tearing_fix == ROTATE) {
		patch__get_vp_init_low_lcd_para(rgb_clock,
			v_total_len, h_total_len, 240, 320
		);
		apply_rgb_timings();
		patch__st7789v_caset_raset(240, 320);
		lcd_memory_data_access_ctl(0); // fb write order == scan order
		rot_buf = malloc(640 * 480 * 2); // magic numbers FIXME
	}

	patch__run_screen_write(&hook_region_write);
}

void video_cleanup(void)
{
	patch__run_screen_write(&run_osd_region_write);

	if (tearing_fix == ROTATE) {
		tearing_fix = FAST;
		// leave the fast patch active for the stock cores
		patch__get_vp_init_low_lcd_para(rgb_clock,
			h_total_len, v_total_len, 320, 240
		);
		apply_rgb_timings();
		patch__st7789v_caset_raset(320, 240);
		lcd_memory_data_access_ctl(0x60); // SF2000
		free(rot_buf);
	}
}
