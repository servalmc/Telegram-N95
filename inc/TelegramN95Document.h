#ifndef __TELEGRAMN95DOCUMENT_H__
#define __TELEGRAMN95DOCUMENT_H__

#include <akndoc.h>

class CEikAppUi;

class CTelegramN95Document : public CAknDocument
    {
    public:
        static CTelegramN95Document* NewL( CEikApplication& aApp );
        static CTelegramN95Document* NewLC( CEikApplication& aApp );
        ~CTelegramN95Document();

    public:
        CEikAppUi* CreateAppUiL();

    private:
        void ConstructL();
        CTelegramN95Document( CEikApplication& aApp );
    };

#endif // __TELEGRAMN95DOCUMENT_H__
