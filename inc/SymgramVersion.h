#ifndef __SYMGRAMVERSION_H__
#define __SYMGRAMVERSION_H__

#include <e32base.h>

#define SYMGRAM_VERSION_MAJOR 0
#define SYMGRAM_VERSION_MINOR 2
#define SYMGRAM_VERSION_BUILD 12

_LIT( KSymgramAppName, "Symgram" );
_LIT( KSymgramVersionName, "0.2.12" );

// Kept in sync with the tag pushed to GitHub; the update check compares
// against the tag name of the latest release.
_LIT( KSymgramReleaseTag, "v0.2.12" );

#endif // __SYMGRAMVERSION_H__
