#!/bin/bash

#fully working
make CONSOLE=a26		CORE=cores/libretro-stella2014	&& \
make CONSOLE=lnx		CORE=cores/libretro-handy	&& \
make CONSOLE=chip8		CORE=cores/jaxe	MAKEFILE=-fMakefile.libretro	&& \
make CONSOLE=col		CORE=cores/Gearcoleco/platforms/libretro	&& \
make CONSOLE=fcf		CORE=cores/FreeChaF	&& \
make CONSOLE=retro8		CORE=cores/retro8	&& \
make CONSOLE=vapor		CORE=cores/vaporspec/machine	MAKEFILE=-fMakefile.libretro	&& \
make CONSOLE=2048		CORE=cores/libretro-2048	MAKEFILE=-fMakefile.libretro	&& \
make CONSOLE=gong		CORE=cores/gong	MAKEFILE=-fMakefile.libretro	&& \
make CONSOLE=prboom		CORE=cores/libretro-prboom	&& \
make CONSOLE=flashback	CORE=cores/REminiscence	&& \
make CONSOLE=cdg		CORE=cores/libretro-pocketcdg	&& \
make CONSOLE=gme		CORE=cores/libretro-gme	&& \
make CONSOLE=pce		CORE=cores/libretro-beetle-pce-fast	&& \
make CONSOLE=ngpc		CORE=cores/RACE	&& \
make CONSOLE=gba		CORE=cores/gpsp	&& \
make CONSOLE=gb			CORE=cores/libretro-tgbdual	&& \
make CONSOLE=nes		CORE=cores/libretro-fceumm	&& \
make CONSOLE=snes02		CORE=cores/snes9x2002	&& \
make CONSOLE=snes		CORE=cores/snes9x2005	&& \
make CONSOLE=sega		CORE=cores/picodrive	MAKEFILE=-fMakefile.libretro	&& \
make CONSOLE=gg			CORE=cores/Gearsystem/platforms/libretro	&& \
make CONSOLE=spec		CORE=cores/libretro-fuse	&& \
make CONSOLE=thom		CORE=cores/theodore	&& \
make CONSOLE=wsv		CORE=cores/potator/platform/libretro	&& \

#working but issues
make CONSOLE=amstrad	CORE=cores/libretro-crocods	&& \
make CONSOLE=m2k		CORE=cores/libretro-mame2000	&& \
make CONSOLE=mame2003	CORE=cores/libretro-mame2003-plus	&& \
make CONSOLE=arduboy	CORE=cores/arduous	&& \
make CONSOLE=a5200		CORE=cores/a5200	&& \
make CONSOLE=a78		CORE=cores/libretro-prosystem	&& \
make CONSOLE=lnxb		CORE=cores/libretro-beetle-lynx	&& \
make CONSOLE=wswan		CORE=cores/libretro-beetle-wswan	&& \
make CONSOLE=lowres-nx	CORE=cores/lowres-nx/platform/LibRetro	&& \
make CONSOLE=mrboom		CORE=cores/libretro-mrboom	&& \
make CONSOLE=jnb		CORE=cores/libretro-jumpnbump	&& \
make CONSOLE=cavestory	CORE=cores/libretro-nxengine	&& \
make CONSOLE=o2em		CORE=cores/libretro-o2em	&& \
make CONSOLE=pcesgx		CORE=cores/libretro-beetle-supergrafx	&& \
make CONSOLE=pcfx		CORE=cores/libretro-beetle-pcfx	&& \
make CONSOLE=vb			CORE=cores/libretro-beetle-vb	&& \
make CONSOLE=gpgx		CORE=cores/Genesis-Plus-GX	MAKEFILE=-fMakefile.libretro	&& \
make CONSOLE=xmil		CORE=cores/libretro-xmil/libretro	&& \

#test cores
make CONSOLE=testadv		CORE=cores/libretro-samples/tests/test_advanced	&& \
make CONSOLE=testwav		CORE=cores/libretro-samples/audio/audio_playback_wav	&& \


true
