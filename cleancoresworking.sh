#!/bin/bash

#fully working
make clean CONSOLE=amstradb   CORE=cores/libretro-cap32 && \
make clean CONSOLE=a26        CORE=cores/libretro-stella2014 && \
make clean CONSOLE=lnx        CORE=cores/libretro-handy && \
make clean CONSOLE=chip8      CORE=cores/jaxe MAKEFILE=-fMakefile.libretro && \
make clean CONSOLE=col        CORE=cores/Gearcoleco/platforms/libretro && \
make clean CONSOLE=fcf        CORE=cores/FreeChaF && \
make clean CONSOLE=retro8     CORE=cores/retro8 && \
make clean CONSOLE=vapor      CORE=cores/vaporspec/machine MAKEFILE=-fMakefile.libretro && \
make clean CONSOLE=2048       CORE=cores/libretro-2048 MAKEFILE=-fMakefile.libretro && \
make clean CONSOLE=gong       CORE=cores/gong MAKEFILE=-fMakefile.libretro && \
make clean CONSOLE=outrun     CORE=cores/cannonball && \
make clean CONSOLE=prboom     CORE=cores/libretro-prboom && \
make clean CONSOLE=flashback  CORE=cores/REminiscence && \
make clean CONSOLE=cdg        CORE=cores/libretro-pocketcdg && \
make clean CONSOLE=int        CORE=cores/FreeIntv && \
make clean CONSOLE=gme        CORE=cores/libretro-gme && \
make clean CONSOLE=pce        CORE=cores/libretro-beetle-pce-fast && \
make clean CONSOLE=ngpc       CORE=cores/RACE && \
make clean CONSOLE=gba        CORE=cores/gpsp && \
make clean CONSOLE=gb         CORE=cores/libretro-tgbdual && \
make clean CONSOLE=nes        CORE=cores/libretro-fceumm && \
make clean CONSOLE=nesq       CORE=cores/QuickNES_Core && \
make clean CONSOLE=pokem      CORE=cores/PokeMini && \
make clean CONSOLE=snes02     CORE=cores/snes9x2002 && \
make clean CONSOLE=snes       CORE=cores/snes9x2005 && \
make clean CONSOLE=sega       CORE=cores/picodrive MAKEFILE=-fMakefile.libretro && \
make clean CONSOLE=gg         CORE=cores/Gearsystem/platforms/libretro && \
make clean CONSOLE=spec       CORE=cores/libretro-fuse && \
make clean CONSOLE=thom       CORE=cores/theodore && \
make clean CONSOLE=vec        CORE=cores/libretro-vecx && \
make clean CONSOLE=wsv        CORE=cores/potator/platform/libretro && \

#working but issues
make clean CONSOLE=amstrad    CORE=cores/libretro-crocods && \
make clean CONSOLE=m2k        CORE=cores/libretro-mame2000 && \
make clean CONSOLE=arduboy    CORE=cores/arduous && \
make clean CONSOLE=a5200      CORE=cores/a5200 && \
make clean CONSOLE=a78        CORE=cores/libretro-prosystem && \
make clean CONSOLE=lnxb       CORE=cores/libretro-beetle-lynx && \
make clean CONSOLE=wswan      CORE=cores/libretro-beetle-wswan && \
make clean CONSOLE=c64        CORE=cores/libretro-vice EMUTYPE=x64 && \
make clean CONSOLE=c64sc      CORE=cores/libretro-vice EMUTYPE=x64sc && \
#make clean CONSOLE=c128      CORE=cores/libretro-vice EMUTYPE=x128 && \
#make clean CONSOLE=c128      CORE=cores/libretro-vice EMUTYPE=x128 && \
#make clean CONSOLE=cbm2      CORE=cores/libretro-vice EMUTYPE=xcbm2 && \
#make clean CONSOLE=cbm5x0    CORE=cores/libretro-vice EMUTYPE=xcbm5x0 && \
#make clean CONSOLE=pet       CORE=cores/libretro-vice EMUTYPE=xpet && \
#make clean CONSOLE=plus4     CORE=cores/libretro-vice EMUTYPE=xplus4 && \
#make clean CONSOLE=scpu64    CORE=cores/libretro-vice EMUTYPE=xscpu64 && \
make clean CONSOLE=vic20      CORE=cores/libretro-vice EMUTYPE=xvic && \
make clean CONSOLE=fake08     CORE=cores/fake-08/platform/libretro && \
make clean CONSOLE=lowres-nx  CORE=cores/lowres-nx/platform/LibRetro && \
make clean CONSOLE=mrboom     CORE=cores/libretro-mrboom && \
make clean CONSOLE=wolf3d     CORE=cores/ecwolf/src/libretro && \
make clean CONSOLE=jnb        CORE=cores/libretro-jumpnbump && \
make clean CONSOLE=cavestory  CORE=cores/libretro-nxengine && \
make clean CONSOLE=o2em       CORE=cores/libretro-o2em && \
make clean CONSOLE=pcesgx     CORE=cores/libretro-beetle-supergrafx && \
make clean CONSOLE=pcfx       CORE=cores/libretro-beetle-pcfx && \
make clean CONSOLE=nest       CORE=cores/nestopia/libretro && \
make clean CONSOLE=vb         CORE=cores/libretro-beetle-vb && \
make clean CONSOLE=gpgx       CORE=cores/Genesis-Plus-GX MAKEFILE=-fMakefile.libretro && \
make clean CONSOLE=xmil       CORE=cores/libretro-xmil/libretro && \

#working but major issues, not to release
#make -C cores/fbalpha2012/svn-current/trunk platform=sf2000 -fmakefile.libretro generate-files-clean && make clean CONSOLE=fba CORE=cores/fbalpha2012/svn-current/trunk MAKEFILE=-fmakefile.libretro && \
#make clean CONSOLE=mame2003   CORE=cores/libretro-mame2003-plus && \

#test cores
make clean CONSOLE=testadv CORE=cores/libretro-samples/tests/test_advanced && \
make clean CONSOLE=testwav CORE=cores/libretro-samples/audio/audio_playback_wav && \


true
