#ifndef __SYMGRAMAPI_H__
#define __SYMGRAMAPI_H__

#include <e32base.h>

// Credentials from https://my.telegram.org — API development tools.
// Leave at 0 until you paste your own; auth.sendCode will not be sent.
const TInt KSymgramApiId = 0;
_LIT8( KSymgramApiHash, "" );

#endif
