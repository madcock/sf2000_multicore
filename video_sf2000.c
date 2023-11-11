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
#include "video_sf2000.h"

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

static void recreate_region(enum tvsystem tvsys, uint16_t width, uint16_t height)
{
	struct osdrect r = { 0, 0, width, height };
	struct osdpara para = { .e_mode = OSD_HD_RGB565 };
	struct {
		uint16_t tv_sys; // specifically NOT enum tvsystem here
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
		scale_param.v_mul = tvsys == PAL ? 576 : 480;
	}
	osddrv_scale(m_osd_dev, OSD_SCALE_WITH_PARAM, (uintptr_t)&scale_param);

	osddrv_scale(m_osd_dev, OSD_SET_SCALE_MODE, filtered ? 1 : 0);
}

static void region_write(const void *buf,
	uint16_t width, uint16_t height, uint16_t pixel_pitch)
{
	struct osd_vscr vscr = { 0, 0, pixel_pitch, height, buf };
	struct osdrect r = { 0, 0, width, height };

	vscr.b_color_mode = OSD_HD_RGB565;
	osddrv_3x_region_write(m_osd_dev, 0, &vscr, &r);
}

static HANDLE m_vpo_dev;

static enum tvsystem get_cur_tvsys(void) {
	enum tvsystem tvsys;

	vpo_ioctl(m_vpo_dev, VPO_IO_GET_OUT_MODE, (uintptr_t)&tvsys);
	return tvsys;
}

static uint16_t *rot_buf;
// do we have a sane way to determine menu region resolution in init? TODO
static uint16_t cur_width = 640, cur_height = 480;

static void hooked_run_osd_region_write(const void *buf,
	uint16_t width, uint16_t height, uint16_t pixel_pitch)
{
	if (get_cur_tvsys() == RGB_LCD && tearing_fix == ROTATE) {
		const uint16_t *row = buf;
		unsigned x, y;

		for (y = height; y > 0; y--) {
			const uint16_t *src = row;
			uint16_t *dst = rot_buf + y - 1; // column
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
		recreate_region(get_cur_tvsys(), width, height);
	}
	region_write(buf, width, height, pixel_pitch);
}

// patch rationale: joy_task switching to TV and back asynchronously
// (and annotating all of vp_init_info for 0x82 ioctl seems overkill)
static void patch__get_vp_init_low_lcd_para(uint16_t rgb_clock,
	uint16_t h_total_len, uint16_t v_total_len,
	uint16_t h_active_len, uint16_t v_active_len)
{
	uint8_t *pfn = (uint8_t *)&get_vp_init_low_lcd_para;

	os_disable_interrupt(); // avoid mode switch inbetween

	*(volatile uint16_t *)(pfn + 0x50) = rgb_clock;
	*(volatile uint16_t *)(pfn + 0x58) = v_total_len;
	*(volatile uint16_t *)(pfn + 0x60) = h_total_len;
	*(volatile uint16_t *)(pfn + 0x68) = v_active_len;
	*(volatile uint16_t *)(pfn + 0x70) = h_active_len; // used for lcd_width, too

	__builtin___clear_cache(pfn, &switch_lcd_or_tv);

	os_enable_interrupt();
}

#define MIPS_LI_A0 (9 << 26 | 5 << 16 | 0 << 21) // addiu a0, zero

static void patch__st7789v_caset_raset(uint16_t cols, uint16_t rows)
{
	uint8_t *pfn = (uint8_t *)&st7789v_caset_raset;

	os_disable_interrupt(); // patch rationale: called in a VSync GPIO ISR!

	*(volatile uint32_t *)(pfn + 0x44) = MIPS_LI_A0 | cols >> 8 & 255;
	*(volatile uint32_t *)(pfn + 0x50) = MIPS_LI_A0 | cols & 255;

	// this one actually was addu a1, $0, $0 hence the full instructions
	*(volatile uint32_t *)(pfn + 0x80) = MIPS_LI_A0 | rows >> 8 & 255;
	*(volatile uint32_t *)(pfn + 0x8c) = MIPS_LI_A0 | rows & 255;

	__builtin___clear_cache(pfn, &st7789v_ramwr);

	os_enable_interrupt();
}

#define MIPS_J (2 << 26)

static void patch__run_screen_write(void *pregion_write)
{
	uint8_t *pfn = (uint8_t *)&run_screen_write;
	extern void run_sound_advance(void *, unsigned); // happens to be next fn

	os_disable_interrupt(); // avoid screen writes inbetween

	*(volatile uint32_t *)(pfn + 0xc0) = MIPS_J |
		(uint32_t)pregion_write >> 2 & ((1 << 26) - 1);

	__builtin___clear_cache(pfn, &run_sound_advance);

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

inline static void apply_rgb_timings(void)
{
	if (get_cur_tvsys() == RGB_LCD)
		switch_lcd_or_tv(1, RGB_LCD);
}

inline static void rotate_region(void)
{
	if (get_cur_tvsys() == RGB_LCD)
		recreate_region(RGB_LCD, cur_height, cur_width);
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
		enum tvsystem tvsys;

		patch__get_vp_init_low_lcd_para(rgb_clock,
			v_total_len, h_total_len, 240, 320
		);
		apply_rgb_timings();

		patch__st7789v_caset_raset(240, 320);
		lcd_memory_data_access_ctl(0); // fb write order == scan order

		rotate_region(); // also clears garbled "Loading..."

		rot_buf = malloc(cur_width * cur_height * sizeof rot_buf[0]);
	}

	// scaling needs the hook even with the tearing fixes disabled
	patch__run_screen_write(&hooked_run_osd_region_write);
}

void video_cleanup(void)
{
	patch__run_screen_write(&run_osd_region_write);

	if (tearing_fix != ROTATE) return; // that's all folks! fast patch stays
	tearing_fix = FAST; // ROTATE affects recreate_region's scaling

	patch__get_vp_init_low_lcd_para(rgb_clock,
		h_total_len, v_total_len, 320, 240
	);
	apply_rgb_timings();

	patch__st7789v_caset_raset(320, 240);
	lcd_memory_data_access_ctl(0x60); // only for SF2000 FIXME

	rotate_region();

	free(rot_buf);
}
