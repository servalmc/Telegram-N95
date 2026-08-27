# Telegram-N95

Native Symbian C++ application targeting **Symbian OS 9.2 / S60 3rd Edition Feature Pack 1** — the platform of the Nokia N95.

## Status

Early setup. Repository scaffolding only; no application code yet.

## Build environment

| Component | Version |
| --- | --- |
| Platform | Symbian OS 9.2, S60 3rd Edition FP1 |
| SDK | S60 3rd Edition FP1 SDK for Symbian OS, for C++ |
| IDE | Carbide.c++ |
| Compiler (device) | GCCE (`abld build gcce urel`) |
| Compiler (emulator) | WINSCW (`abld build winscw udeb`) |

## Building from the command line

```
cd group
bldmake bldfiles
abld build winscw udeb      # emulator
abld build gcce urel        # device
```

Packaging a signed installer:

```
makesis <name>.pkg
signsis <name>.sis <name>.sisx <cert>.cer <key>.key
```

## Layout

```
group/    bld.inf, .mmp project files
inc/      headers
src/      sources
data/     .rss resource files
sis/      .pkg packaging scripts
gfx/      icons (.svg) and bitmaps
```

## Notes

Applications for Symbian 9.x are capability-constrained. Self-signed builds are limited to the basic
capability set; anything beyond that requires a Symbian Signed or developer certificate, and the
application UID must come from the unprotected range (`0xE0000000`–`0xEFFFFFFF`) for self-signing.
