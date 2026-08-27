#ifndef __SYMGRAMVERSION_H__
#define __SYMGRAMVERSION_H__

#include <e32base.h>

#define SYMGRAM_VERSION_MAJOR 0
#define SYMGRAM_VERSION_MINOR 1
#define SYMGRAM_VERSION_BUILD 0

_LIT( KSymgramAppName, "Symgram" );
_LIT( KSymgramVersionName, "0.1.0" );

// Kept in sync with the tag pushed to GitHub; the update check compares
// against the tag name of the latest release.
_LIT( KSymgramReleaseTag, "v0.1.0" );

#endif // __SYMGRAMVERSION_H__
