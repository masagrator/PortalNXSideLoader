

# Portal Collection File Sideloader

Plugin for Portal 1 and 2 that allow to load files outside of game.zip, so no more repacking and sending huge archives every time you want to change something.

# Installation
From releases download:
- for Portal 1:
> Portal-NXSideLoader.zip
- for Portal 2:
>Without network patches (it won't block you from using network):
> - Portal2-NXSideLoader.zip
>
> With network patches (for people that block Nintendo servers, it will block game from connecting to network to prevent crash + helps with debugging):
> - Portal2-NoWeb-NXSideLoader.zip

Put `atmosphere` folder to root of sdcard (yes, your CFW won't be deleted...)

# Informations for mod makers

Few files in `nxcontent` may not be supported as they are preloaded with separate functions. I needed to add specific support for one function so `rom_boot_params.txt` could be loaded. If there is any file it's not working and you want it to work, write na issue.

# How this works?

Devs redesigned whole cstdio to use game.zip as filesystem.
Portal games to open most files are using function called `fopen_nx()`. To read this file - `fread_nx()`, etc.
All functions are cross compatible with cstdio, so solution was pretty easy:
1. Hook `fopen_nx()`
2. Detect if passed file path exists on SD card
3. If it exists, redirect call to `fopen()` with correct path starting with `rom:/`

There were 2 issues with this solution:
- not all files are using this function. It seems there is not many of them and only important one in my opinion was `rom_boot_params.txt` so I have hooked function reading this file and redesigned it to load file from SD card.
- fopen_nx() path load is ignoring case + has priority to check first if file is inside `nxcontent` folder, and if not checks root of zip. Inside hook remade this check + used `tolower()` for file path since all files and folders names inside zip are lower case.

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
with internet patches to allow debugging via network that has blocked access to Nintendo servers
```
make PORTAL="-DPORTAL2 -DPDEBUG"
```
