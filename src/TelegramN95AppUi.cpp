#include <avkon.hrh>
#include <aknnotewrappers.h>
#include <stringloader.h>
#include <TelegramN95.rsg>

#include "TelegramN95.hrh"
#include "TelegramN95AppUi.h"
#include "TelegramN95AppView.h"

CTelegramN95AppUi::CTelegramN95AppUi()
    : iAppView( NULL )
    {
    }

CTelegramN95AppUi::~CTelegramN95AppUi()
    {
    if ( iAppView )
        {
        RemoveFromStack( iAppView );
        delete iAppView;
        iAppView = NULL;
        }
    }

void CTelegramN95AppUi::ConstructL()
    {
    BaseConstructL( EAknEnableSkin );

    iAppView = CTelegramN95AppView::NewL( ClientRect() );
    AddToStackL( iAppView );
    }

void CTelegramN95AppUi::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            Exit();
            break;

        case ETelegramN95CmdAbout:
            ShowAboutL();
            break;

        default:
            break;
        }
    }

void CTelegramN95AppUi::HandleStatusPaneSizeChange()
    {
    CAknAppUi::HandleStatusPaneSizeChange();

    if ( iAppView )
        {
        iAppView->SetRect( ClientRect() );
        }
    }

void CTelegramN95AppUi::ShowAboutL()
    {
    HBufC* text = StringLoader::LoadLC( R_TELEGRAMN95_ABOUT_TEXT );
    CAknInformationNote* note = new ( ELeave ) CAknInformationNote( ETrue );
    note->ExecuteLD( *text );
    CleanupStack::PopAndDestroy( text );
    }
