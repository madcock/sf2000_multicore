#ifndef __BSOD_DEBUG_H
#define __BSOD_DEBUG_H

void lcd_bsod(const char *fmt, ...);

void dbg_cls();
void dbg_print(const char *fmt, ...);
void dbg_show();
void dbg_show_noblock();

#endif
