#include "TelegramN95Document.h"
#include "TelegramN95AppUi.h"

CTelegramN95Document* CTelegramN95Document::NewL( CEikApplication& aApp )
    {
    CTelegramN95Document* self = NewLC( aApp );
    CleanupStack::Pop( self );
    return self;
    }

CTelegramN95Document* CTelegramN95Document::NewLC( CEikApplication& aApp )
    {
    CTelegramN95Document* self = new ( ELeave ) CTelegramN95Document( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CTelegramN95Document::CTelegramN95Document( CEikApplication& aApp )
    : CAknDocument( aApp )
    {
    }

CTelegramN95Document::~CTelegramN95Document()
    {
    }

void CTelegramN95Document::ConstructL()
    {
    }

CEikAppUi* CTelegramN95Document::CreateAppUiL()
    {
    return static_cast<CEikAppUi*>( new ( ELeave ) CTelegramN95AppUi );
    }
