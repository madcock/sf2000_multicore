/* Copyright (C) 2023 Nikita Burnashev

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted.

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND! */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
	uint32_t tab[256], c;
	int i, j;
	FILE *pf;
	static uint8_t buf[8192];
	size_t sz;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <bisrv.asd>\n", argv[0]);
		return EXIT_FAILURE;
	}

	pf = fopen(argv[1], "rb+");
	if (pf == NULL) {
		fprintf(stderr, "Cannot open %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	/* CRC-32/MPEG-2 variant https://reveng.sourceforge.io/crc-catalogue/all.htm#crc.cat.crc-32-mpeg-2 */
	for (i = 0; i < 256; i++) {
		c = (unsigned long)i << 24;
		for (j = 0; j < 8; j++) {
			c = c & (1 << 31) ? c << 1 ^ 0x4c11db7 : c << 1;
		}
		tab[i] = c;
	}
	c = 0xffffffff;
	fseek(pf, 512, SEEK_SET);
	while ((sz = fread(buf, 1, sizeof buf, pf)) > 0) {
		for (i = 0; i < sz; i++) c = c << 8 ^ tab[c >> 24 ^ buf[i]];
	}

	buf[0] = c & 255;
	buf[1] = c >> 8 & 255;
	buf[2] = c >> 16 & 255;
	buf[3] = c >> 24;
	fseek(pf, 0x18c, SEEK_SET);
	fwrite(buf, 1, 4, pf);
	fclose(pf);

	return EXIT_SUCCESS;
}
