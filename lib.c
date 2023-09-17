#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "debug.h"

extern void osal_tds2_cache_flush(void *buf, unsigned sz);

void _flush_cache(void *buf, size_t sz, int flags)
{
	osal_tds2_cache_flush(buf, sz);
	/* TODO invalidate I-cache */
}

/* ALi's libc lacks newer POSIX stuff, */
char *stpcpy(char *dst, const char *src)
{
	size_t sz = strlen(src) + 1;

	memcpy(dst, src, sz);
	return &dst[sz - 1];
}

/* localization, */
extern char _ctype_[257];

const char *__locale_ctype_ptr (void)
{
	return _ctype_;
}

/* reentrant functions (newer builds fixed that), */
extern int g_errno;

int *__errno(void)
{
	return &g_errno;
}

/* but also some ages-old ISO/ANSI stuff, too! */
int puts(const char *s)
{
	return (printf("%s\n", s) < 0 ? EOF : '\n');
}

/* ALi violated ISO/ANSI with fseek taking off_t */
int fseek(FILE *stream, long offset, int whence)
{
	return fseeko(stream, offset, whence);
}

void rewind(FILE *stream)
{
	fseeko(stream, 0, SEEK_SET);
}

void abort(void)
{
	unsigned ra;
	asm volatile ("move %0, $ra" : "=r" (ra));
	lcd_bsod("abort() called from 0x%08x", ra);
}

void exit(int status)
{
	unsigned ra;
	asm volatile ("move %0, $ra" : "=r" (ra));
	lcd_bsod("exit(%d) called from 0x%08x", status, ra);
}

/* wrappers were not compiled in, but vfs drivers likely support these ops */
int remove(const char *path)
{
	return -1;
}

int rename(const char *old, const char* new)
{
	return -1;
}
