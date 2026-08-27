#include <eikstart.h>
#include "SymgramApplication.h"

LOCAL_C CApaApplication* NewApplication()
    {
    return new CSymgramApplication;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }
