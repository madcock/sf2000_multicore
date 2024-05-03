#!/bin/bash
dos2unix buildcoresworking.sh
dos2unix cleanandbuildworking.sh

# TODO: move into separate repo with assets
wget https://raw.githubusercontent.com/EricGoldsteinNz/SF2000_Resources/main/OS/V1.6/bios/bisrv.asd -O /__w/sf2000_multicore/sf2000_multicore/sdcard

rm -rf sdcard
./cleancoresworking.sh
./buildcoresworking.sh

cp -r assets/coreconfig/* /__w/gb300_multicore/gb300_multicore/sdcard/core/config/*

tar -czvf /__w/sf2000_multicore/sf2000_multicore/sf2000_multicore/sf2000-multicore-canary.tar.gz /__w/sf2000_multicore/sf2000_multicore/sdcard