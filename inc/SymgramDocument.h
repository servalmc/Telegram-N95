#ifndef __SYMGRAMDOCUMENT_H__
#define __SYMGRAMDOCUMENT_H__

#include <akndoc.h>

class CEikAppUi;

class CSymgramDocument : public CAknDocument
    {
    public:
        static CSymgramDocument* NewL( CEikApplication& aApp );
        static CSymgramDocument* NewLC( CEikApplication& aApp );
        ~CSymgramDocument();

    public:
        CEikAppUi* CreateAppUiL();

    private:
        void ConstructL();
        CSymgramDocument( CEikApplication& aApp );
    };

#endif // __SYMGRAMDOCUMENT_H__
