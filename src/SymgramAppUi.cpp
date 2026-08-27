#include <avkon.hrh>
#include <aknnotewrappers.h>
#include <stringloader.h>
#include <Symgram.rsg>

#include "Symgram.hrh"
#include "SymgramVersion.h"
#include "SymgramAppUi.h"
#include "SymgramAppView.h"

CSymgramAppUi::CSymgramAppUi()
    : iAppView( NULL )
    {
    }

CSymgramAppUi::~CSymgramAppUi()
    {
    if ( iAppView )
        {
        RemoveFromStack( iAppView );
        delete iAppView;
        iAppView = NULL;
        }
    }

void CSymgramAppUi::ConstructL()
    {
    BaseConstructL( EAknEnableSkin );

    iAppView = CSymgramAppView::NewL( ClientRect() );
    AddToStackL( iAppView );
    }

void CSymgramAppUi::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            Exit();
            break;

        case ESymgramCmdAbout:
            ShowAboutL();
            break;

        default:
            break;
        }
    }

void CSymgramAppUi::HandleStatusPaneSizeChange()
    {
    CAknAppUi::HandleStatusPaneSizeChange();

    if ( iAppView )
        {
        iAppView->SetRect( ClientRect() );
        }
    }

void CSymgramAppUi::ShowAboutL()
    {
    HBufC* body = StringLoader::LoadLC( R_SYMGRAM_ABOUT_TEXT );

    HBufC* text = HBufC::NewLC(
        KSymgramAppName().Length() + KSymgramVersionName().Length() +
        body->Length() + 2 );
    TPtr ptr( text->Des() );
    ptr.Append( KSymgramAppName );
    ptr.Append( ' ' );
    ptr.Append( KSymgramVersionName );
    ptr.Append( '\n' );
    ptr.Append( *body );

    CAknInformationNote* note = new ( ELeave ) CAknInformationNote( ETrue );
    note->ExecuteLD( *text );

    CleanupStack::PopAndDestroy( 2, body );
    }
