# Telegram-N95

Native Symbian C++ application targeting **Symbian OS 9.2 / S60 3rd Edition Feature Pack 1** — the platform of the Nokia N95.

## Status

Toolchain is installed and verified end to end: sources compile with GCCE, and the resulting
binary packages into a self-signed `.sisx` installable on a device. No application code yet.

## Build environment

| Component | Version |
| --- | --- |
| Platform | Symbian OS 9.2, S60 3rd Edition FP1 |
| SDK | S60 3rd Edition FP1 SDK for Symbian OS, for C++ |
| Compiler (device) | GCCE — GCC 3.4.3, CodeSourcery ARM Q1C 2005 |
| Compiler (emulator) | WINSCW |

See [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md) for how it is installed and for the platform quirks
worth knowing before touching a build.

## Building

Run from `cmd.exe`, not PowerShell — see the toolchain notes for why.

```
tools\build.bat              REM GCCE release, for the device
tools\build.bat winscw udeb  REM emulator
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
