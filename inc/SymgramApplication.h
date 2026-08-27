#ifndef __SYMGRAMAPPLICATION_H__
#define __SYMGRAMAPPLICATION_H__

#include <aknapp.h>

const TUid KUidSymgramApp = { 0xE0A11E95 };

class CSymgramApplication : public CAknApplication
    {
    public:
        TUid AppDllUid() const;

    protected:
        CApaDocument* CreateDocumentL();
    };

#endif // __SYMGRAMAPPLICATION_H__
