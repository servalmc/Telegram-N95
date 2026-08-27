# Telegram-N95

Native Symbian C++ application targeting **Symbian OS 9.2 / S60 3rd Edition Feature Pack 1** — the platform of the Nokia N95.

## Status

Runnable application skeleton: an AVKON GUI app that starts, shows a status line and an Options
menu, and builds all the way to a self-signed `.sisx` installable on a device. None of the
Telegram protocol is implemented yet.

The application UID is `0xE0A11E95`, taken from the unprotected range so the package can be
self-signed. Requested capabilities are `NetworkServices`, `ReadUserData`, `WriteUserData` and
`UserEnvironment` — all user-grantable, which keeps self-signing viable.

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
tools\package.bat            REM sis\TelegramN95.sisx, self-signed
```

`package.bat` generates a self-signing certificate on first run and reuses it afterwards; the
certificate and key stay out of version control.

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
