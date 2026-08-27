#include <eikstart.h>
#include "TelegramN95Application.h"

LOCAL_C CApaApplication* NewApplication()
    {
    return new CTelegramN95Application;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }
