#include "SymgramDocument.h"
#include "SymgramAppUi.h"

CSymgramDocument* CSymgramDocument::NewL( CEikApplication& aApp )
    {
    CSymgramDocument* self = NewLC( aApp );
    CleanupStack::Pop( self );
    return self;
    }

CSymgramDocument* CSymgramDocument::NewLC( CEikApplication& aApp )
    {
    CSymgramDocument* self = new ( ELeave ) CSymgramDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CSymgramDocument::CSymgramDocument( CEikApplication& aApp )
    : CAknDocument( aApp )
    {
    }

CSymgramDocument::~CSymgramDocument()
    {
    }

void CSymgramDocument::ConstructL()
    {
    }

CEikAppUi* CSymgramDocument::CreateAppUiL()
    {
    return static_cast<CEikAppUi*>( new ( ELeave ) CSymgramAppUi );
    }
