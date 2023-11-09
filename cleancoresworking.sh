#!/bin/bash

#fully working
make clean CONSOLE=a26		CORE=cores/libretro-stella2014	&& \
make clean CONSOLE=lnx		CORE=cores/libretro-handy	&& \
make clean CONSOLE=chip8	CORE=cores/jaxe	MAKEFILE=-fMakefile.libretro	&& \
make clean CONSOLE=col		CORE=cores/Gearcoleco/platforms/libretro	&& \
make clean CONSOLE=fcf		CORE=cores/FreeChaF	&& \
make clean CONSOLE=retro8	CORE=cores/retro8	&& \
make clean CONSOLE=vapor	CORE=cores/vaporspec/machine	MAKEFILE=-fMakefile.libretro	&& \
make clean CONSOLE=2048		CORE=cores/libretro-2048	MAKEFILE=-fMakefile.libretro	&& \
make clean CONSOLE=gong		CORE=cores/gong	MAKEFILE=-fMakefile.libretro	&& \
make clean CONSOLE=prboom	CORE=cores/libretro-prboom	&& \
make clean CONSOLE=flashback	CORE=cores/REminiscence	&& \
make clean CONSOLE=cdg		CORE=cores/libretro-pocketcdg	&& \
make clean CONSOLE=gme		CORE=cores/libretro-gme	&& \
make clean CONSOLE=pce		CORE=cores/libretro-beetle-pce-fast	&& \
make clean CONSOLE=ngpc		CORE=cores/RACE	&& \
make clean CONSOLE=gba		CORE=cores/gpsp	&& \
make clean CONSOLE=gb		CORE=cores/libretro-tgbdual	&& \
make clean CONSOLE=nes		CORE=cores/libretro-fceumm	&& \
make clean CONSOLE=snes02	CORE=cores/snes9x2002	&& \
make clean CONSOLE=snes		CORE=cores/snes9x2005	&& \
make clean CONSOLE=sega		CORE=cores/picodrive	MAKEFILE=-fMakefile.libretro	&& \
make clean CONSOLE=gg		CORE=cores/Gearsystem/platforms/libretro	&& \
make clean CONSOLE=spec		CORE=cores/libretro-fuse	&& \
make clean CONSOLE=thom		CORE=cores/theodore	&& \
make clean CONSOLE=wsv		CORE=cores/potator/platform/libretro	&& \

#working but issues
make clean CONSOLE=amstrad	CORE=cores/libretro-crocods	&& \
make clean CONSOLE=m2k		CORE=cores/libretro-mame2000	&& \
make clean CONSOLE=mame2003	CORE=cores/libretro-mame2003-plus	&& \
make clean CONSOLE=arduboy	CORE=cores/arduous	&& \
make clean CONSOLE=a5200	CORE=cores/a5200	&& \
make clean CONSOLE=a78		CORE=cores/libretro-prosystem	&& \
make clean CONSOLE=lnxb		CORE=cores/libretro-beetle-lynx	&& \
make clean CONSOLE=wswan	CORE=cores/libretro-beetle-wswan	&& \
make clean CONSOLE=lowres-nx	CORE=cores/lowres-nx/platform/LibRetro	&& \
make clean CONSOLE=mrboom	CORE=cores/libretro-mrboom	&& \
make clean CONSOLE=jnb		CORE=cores/libretro-jumpnbump	&& \
make clean CONSOLE=cavestory	CORE=cores/libretro-nxengine	&& \
make clean CONSOLE=o2em		CORE=cores/libretro-o2em	&& \
make clean CONSOLE=pcesgx	CORE=cores/libretro-beetle-supergrafx	&& \
make clean CONSOLE=pcfx		CORE=cores/libretro-beetle-pcfx	&& \
make clean CONSOLE=vb		CORE=cores/libretro-beetle-vb	&& \
make clean CONSOLE=gpgx		CORE=cores/Genesis-Plus-GX	MAKEFILE=-fMakefile.libretro	&& \
make clean CONSOLE=xmil		CORE=cores/libretro-xmil/libretro	&& \

#test cores
make clean CONSOLE=testadv		CORE=cores/libretro-samples/tests/test_advanced	&& \
make clean CONSOLE=testwav		CORE=cores/libretro-samples/audio/audio_playback_wav	&& \


true
