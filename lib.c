#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "debug.h"
#include "stockfw.h"

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

// TODO: this __locale_ctype_ptr collide with the one from libc
// maybe defined it with attribute((weak)) or delete it all together
//
///* localization, */
// extern char _ctype_[257];

// const char *__locale_ctype_ptr (void)
// {
	// return _ctype_;
// }

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

// wrap fs_stat to supply a more standard stat implementation
// for now only `type` (for dir or file) and `size` fields of `struct stat` are filled
int	stat(const char *path, struct stat *sbuf)
{
	struct _ {
		union {
			struct {
				uint8_t	_1[0x18];	// type is at offset 0x18
				uint32_t type;		// 0x81b6 - file,	0x41ff - dir
			};
			struct {
				uint8_t	_2[0x38];	// size is at offset 0x38
				uint32_t size;		// filesize if type is a file
			};
			uint8_t	__[160];		// total struct size is 160
		};
	} buffer = {0};

	int ret = fs_stat(path, &buffer);
	if (ret == 0)
	{
		memset(sbuf, 0, sizeof(*sbuf));
		sbuf->st_mode = S_ISREG(buffer.type)*S_IFREG | S_ISDIR(buffer.type)*S_IFDIR;
		sbuf->st_size = buffer.size;
	}

	return ret;
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

int isatty(int fd)
{
    return 0;
}
