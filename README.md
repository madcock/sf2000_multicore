# SF2000 Multicore

## About the platform
More information about the SF2000 handheld gaming system can be found [here](https://vonmillhausen.github.io/sf2000/).

## Status
Some cores build and work well, others build and have issues (no sound, slowness, etc.), some build but do not currently work, and others do not build at all. For current information on the state of each core, please see the [spreadsheet](https://docs.google.com/spreadsheets/d/1BDPqLwRcY2cN7tObuyW7RzLw8oGyY9XGLS1D4jLgz2Q/edit?usp=sharing).

## Releases
Multicore releases can be found here: https://github.com/madcock/sf2000_multicore_cores/releases

## Setup (before building)
(This is the top level project, forked from: https://gitlab.com/kobily/sf2000_multicore)
- choose a directory to contain all this
- decide if you want EVERYTHING (there are a lot of cores!)
    - YES, I want ALL the cores!
        - `git clone --recursive https://github.com/madcock/sf2000_multicore`
        - wait for a long while
    - NO, I want to select individual cores
        - `git clone https://github.com/madcock/sf2000_multicore`
        - `cd sf2000_multicore/libs/libretro-common`
        - `git submodule update --init .`
        - `cd ../..`
        - choose a core to grab, for example, picodrive
        - `cd cores/picodrive`
        - `git submodule update --init .`
        - repeat this process for as many cores as you want
- from the top level of this repo, install the compiler tools
    - `./install-toolchain.sh`

## Building
If setup properly, there are a few ways to build.

If all the core sources for working cores are available:
- clean everything with `./cleancoresworking.sh`
- and build everything with `./buildcoresworking.sh`

If only some of the core sources are available:
- edit the Makefile as necessary to point to the core(s) you want to build
- the Makefile must include one entry for `CORE` and one for `CONSOLE` with an optional `MAKEFILE` entry
- the easiest thing to do is just uncomment a single core

All of the cores are independent.

## Discussion
All the latest information can be found in the [dev channel on Discord](https://discord.com/channels/741895796315914271/1099465777825972347) which is part of [Retro Handhelds](https://discord.gg/retrohandhelds).

## Notes
This repo was forked from the official repo in github that kobil maintains at https://gitlab.com/kobily/sf2000_multicore
I will try to keep things here in sync.
