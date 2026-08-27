#include "SymgramApplication.h"
#include "SymgramDocument.h"

CApaDocument* CSymgramApplication::CreateDocumentL()
    {
    return static_cast<CApaDocument*>( CSymgramDocument::NewL( *this ) );
    }

TUid CSymgramApplication::AppDllUid() const
    {
    return KUidSymgramApp;
    }
