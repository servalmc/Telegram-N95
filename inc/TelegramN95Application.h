#ifndef __TELEGRAMN95APPLICATION_H__
#define __TELEGRAMN95APPLICATION_H__

#include <aknapp.h>

const TUid KUidTelegramN95App = { 0xE0A11E95 };

class CTelegramN95Application : public CAknApplication
    {
    public:
        TUid AppDllUid() const;

    protected:
        CApaDocument* CreateDocumentL();
    };

#endif // __TELEGRAMN95APPLICATION_H__
