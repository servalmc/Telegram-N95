#include <avkon.hrh>
#include <aknnotewrappers.h>
#include <stringloader.h>
#include <e32keys.h>
#include <w32std.h>
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
    SetKeyBlockMode( ENoKeyBlock );

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

        case ESymgramCmdNext:
            if ( iAppView )
                {
                iAppView->NextL();
                }
            break;

        case ESymgramCmdCountry:
            if ( iAppView )
                {
                iAppView->CycleCountry( 1 );
                }
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

TBool CSymgramAppUi::IsNaviKey( const TKeyEvent& aKey ) const
    {
    switch ( aKey.iScanCode )
        {
        case EStdKeyUpArrow:
        case EStdKeyDownArrow:
        case EStdKeyLeftArrow:
        case EStdKeyRightArrow:
        case EStdKeyDevice3:
        case EStdKeyEnter:
            return ETrue;
        default:
            break;
        }
    switch ( aKey.iCode )
        {
        case EKeyUpArrow:
        case EKeyDownArrow:
        case EKeyLeftArrow:
        case EKeyRightArrow:
        case EKeyDevice3:
        case EKeyEnter:
            return ETrue;
        default:
            return EFalse;
        }
    }

void CSymgramAppUi::HandleWsEventL( const TWsEvent& aEvent,
                                    CCoeControl* aDestination )
    {
    const TInt type = aEvent.Type();
    if ( iAppView &&
         ( type == EEventKeyDown || type == EEventKey || type == EEventKeyUp ) )
        {
        TKeyEvent key = *aEvent.Key();
        TUint alias = key.iCode;
        TRAP_IGNORE( GetAliasKeyCodeL( alias, key, (TEventCode)type ) );
        if ( alias != 0 )
            {
            key.iCode = alias;
            }
        if ( IsNaviKey( key ) || IsNaviKey( *aEvent.Key() ) )
            {
            if ( iAppView->OfferKeyEventL( key, (TEventCode)type ) ==
                 EKeyWasConsumed )
                {
                return;
                }
            }
        }
    CAknAppUi::HandleWsEventL( aEvent, aDestination );
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
