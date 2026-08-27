#include "TelegramN95Application.h"
#include "TelegramN95Document.h"

CApaDocument* CTelegramN95Application::CreateDocumentL()
    {
    return static_cast<CApaDocument*>( CTelegramN95Document::NewL( *this ) );
    }

TUid CTelegramN95Application::AppDllUid() const
    {
    return KUidTelegramN95App;
    }
