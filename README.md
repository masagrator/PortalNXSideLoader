

# Portal Collection File Sideloader

# Installation (not released yet) 
From releases download:
- for Portal 1:
> Portal-NXSideLoader.zip
- for Portal 2:
>Without network patches:
> - Portal2-NXSideLoader.zip
>
> With network patches:
> - Portal2-NoWeb-NXSideLoader.zip

Put `atmosphere` folder to root of sdcard (yes, your CFW won't be deleted...)

# Informations for mod makers

`nxcontent` folder is not supported fully. Most of files from there you can put into `romfs` folder and they will be working.
Around dozen of files in Portal 1 from this folder needs special treatment and I have worked only to add support for `rom_boot_params.txt`. Portal 2 is still under development. 

So for example we have `Bringus mod` for Portal 1 which uses only `nxcontent` folder as of today.
Solution to use it is to copy everything from nxcontent folder except `rom_boot_params.txt` directly to `romfs` folder on sdcard, and put also there `nxcontent` folder with just `rom_boot_params.txt` file.

Example:
- Before
```
romfs/nxcontent/rom_boot_params.txt
romfs/nxcontent/portal/resource/gamemenu.res
romfs/nxcontent/portal/cfg/rocket.cfg
```
- After
```
romfs/nxcontent/rom_boot_params.txt
romfs/portal/resource/gamemenu.res
romfs/portal/cfg/rocket.cfg
```

# How this works?

Devs redesigned whole cstdio to use game.zip as filesystem.
Game to open most files is using function called `fopen_nx()`. To read this file - `fread_nx()`, etc.
All functions are cross compatible with cstdio, so solution was pretty easy:
1. Hook `fopen_nx()`
2. Detect if passed file path exists on SD card
3. If it exists, redirect call to `fopen()` with correct path starting with `rom:/`

There were 2 issues with this solution:
- not all files are using this function. From tests only files that have hardcoded path starting with "nxcontent" are passed through different functions. It seems there is not many of them (around dozen in Portal 1) and only important one in my opinion was `rom_boot_params.txt` so I have hooked function reading this file and redesigned it to load file from SD card.
- `fopen_nx()` is dependent on mounted path. So it can change mounted path to `game.zip/nxcontent/` in Portal 1 and pass to fopen_nx rest of path. As I didn't want to add additional overhead since checking what is mounted needs some work, solution is just to put folders from nxcontent into romfs root. 

# Compilation

You need standard devkitpro install with Switch-dev.

Patch `main.npdm` from exefs with this, otherwise plugin will crash:
https://github.com/skyline-dev/skyline/blob/master/scripts/patchNpdm.py

To compile it for Portal 1 use command
```
make PORTAL="-DPORTAL"
```

for Portal 2
```
make PORTAL="-DPORTAL2"
```
with internet patches to allow debugging via network
```
make PORTAL="-DPORTAL2 -DPDEBUG"
```
