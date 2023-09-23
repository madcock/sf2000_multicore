#ifndef __LCDFONT_H
#define __LCDFONT_H

/* borrowed from RetroArch/gfx/drivers_font_renderer, usual license restrictions apply */
#define FONT_WIDTH 5
#define FONT_HEIGHT 10

#define FONT_WIDTH_STRIDE (FONT_WIDTH + 1)

#define FONT_OFFSET(x) ((x - 0x20) * ((FONT_HEIGHT * FONT_WIDTH + 7) / 8))

// the actual lcd_font is defined in lcd_font.c
// lcd_font.c is compiled seperately and its binary is inserted
// into bisrv.asd at file offset 0x2260 which at runtime resides
// at 0x80002260 memory address

static const unsigned char *lcd_font = (unsigned char *)0x80002260;

#endif
