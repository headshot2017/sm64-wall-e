# libsm64-wall-e

Mod for WALL-E (PC) that replaces the player with Mario from Super Mario 64 by using the [libsm64](https://github.com/libsm64/libsm64) library

Work in progress... at this time, the mod only loads the ROM. Mario does not appear in-game yet.

## Requirements to play

* Installation of WALL-E on PC, either the original disc release (untested) or from Steam
* Super Mario 64 US ROM placed alongside WALL-E.exe, with filename sm64.z64
  * ROM must match SHA-1 hash 9bef1128717f958171a4afac3ed78ee2bb4e86ce
* [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/Win32-latest/version-Win32.zip) placed alongside WALL-E.exe
* libsm64-wall-e.asi placed in WALL-E/scripts folder

## Requirements to compile
* CodeBlocks
* SDL2

## Third-party libraries used
* [libsm64](https://github.com/libsm64/libsm64) to provide SM64 Mario
* [SafetyHook](https://github.com/cursey/safetyhook) to hook into WALL-E game functions
* [TinySHA1](https://github.com/mohaps/TinySHA1) to check for valid SM64 ROM
* [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2) to provide an SM64 audio backend
