# Build environment

Verified working on Windows 11 Pro x64 (build 26200), August 2026.

## Installed components

| Component | Version | Location |
| --- | --- | --- |
| S60 3rd Edition FP1 SDK for Symbian OS, C++ | 1.0.1 | `C:\Symbian\9.2\S60_3rd_FP1` |
| CSL ARM Toolchain (GCCE) | GCC 3.4.3, CodeSourcery ARM Q1C 2005 | `C:\Program Files (x86)\CSL Arm Toolchain` |
| ActivePerl | 5.6.1 build 635 | `D:\Perl` |
| Symbian device registry tools | — | `C:\Program Files (x86)\Common Files\Symbian\tools` |

Sources for all of the above are archived at
[archive.org/details/nokia_sdks_n_dev_tools](https://archive.org/details/nokia_sdks_n_dev_tools);
the original Forum Nokia distribution site is long gone.

Environment variables are set at user scope: `EPOCROOT=\Symbian\9.2\S60_3rd_FP1\`, plus the
tool directories appended to `PATH`.

## Building

```
tools\build.bat              REM GCCE release, for the device
tools\build.bat gcce udeb    REM GCCE debug
tools\build.bat winscw udeb  REM emulator
```

Packaging and self-signing:

```
makesis sis\<name>.pkg <name>.sis
makekeys -cert -password <pw> -len 1024 -dname "CN=... OU=... OR=... CO=FI" selfsign.key selfsign.cer
signsis <name>.sis <name>.sisx selfsign.cer selfsign.key <pw>
```

## Platform quirks that cost time

These are not obvious, and each one produces a misleading error message.

**Use `cmd.exe`, never PowerShell.** The SDK ships every tool as a 45 KB launcher stub that
shells out to a same-named `.pl` script, and it does not quote the script path. `cmd.exe` uses
`PATH` entries verbatim, so putting the 8.3 short forms (`C:\PROGRA~2\COMMON~1\Symbian\tools`)
on `PATH` sidesteps the spaces. PowerShell canonicalises the short form back to the long path
before launching, which reintroduces the spaces and yields
`Can't open perl script "C:\Program": No such file or directory`.

**Perl must be 5.6.1.** `abld` and `bldmake` are Perl scripts that do not run on modern Perl.

**`EPOCROOT` has no drive letter** and is resolved against the current drive, so a project on
`D:` cannot build against an SDK on `C:`. Rather than reinstalling, `D:\Symbian` is an NTFS
junction pointing at `C:\Symbian`; junctions are transparent to path resolution, so the build
sees consistent `D:` paths and writes its output through to `C:`. Verified by building the same
example from both drives.

**`set` in a batch file captures trailing spaces.** `set EPOCROOT=\path\ && cmd` assigns
`"\path\ "` and the tools then complain that `EPOCROOT must end with a backslash`. Always use
the quoted form `set "EPOCROOT=\path\"`.

**Apply the Vista patch.** It replaces `Epoc32\tools\cl_bpabi.pm`, the module that generates
GCCE makefiles, and is required on anything newer than Windows XP. The original is kept
alongside as `cl_bpabi.pm.orig`.

**SVG icons do not build.** `mifconv` delegates SVG-to-SVGB conversion to `svgtbinencode.exe`,
which this SDK only ships as a WINSCW emulator binary under `Epoc32\release\winscw\udeb`. Run
standalone it exits 0 and silently produces nothing, so `mifconv` then fails with
`Unable to open file for reading! ...svgb` and the `RESOURCE` build step dies. Everything else
compiles and links normally. `mifconv` itself is fine and handles BMP sources, which need an
accompanying `<name>_mask_soft.bmp` mask.

**`makesis` does not expand `$(EPOCROOT)`** in this SDK — that came with later tooling. Package
paths are therefore written from the drive root, e.g.
`\Symbian\9.2\S60_3rd_FP1\epoc32\release\gcce\urel\...`, which also resolves through the junction.

**The emulator is untested.** WINSCW needs a 32-bit Java 5 for its preferences dialogs and is
known to be unreliable on modern Windows. Device builds via GCCE are unaffected.
