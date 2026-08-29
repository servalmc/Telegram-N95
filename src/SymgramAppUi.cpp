#include <avkon.hrh>
#include <aknnotewrappers.h>
#include <stringloader.h>
#include <e32keys.h>
#include <w32std.h>
#include <eikmenub.h>
#include <eikmenup.h>
#include <eikenv.h>
#include <eikaufty.h>
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

        case ESymgramCmdRefresh:
            if ( iAppView )
                {
                iAppView->RefreshL();
                }
            break;

        case ESymgramCmdContacts:
            if ( iAppView )
                {
                iAppView->ShowContactsL();
                }
            break;

        case ESymgramCmdSettings:
            if ( iAppView )
                {
                iAppView->ShowSettingsL();
                }
            break;

        case ESymgramCmdWrite:
            if ( iAppView )
                {
                iAppView->ComposeL();
                }
            break;

        case ESymgramCmdEmoji:
            if ( iAppView )
                {
                iAppView->PickEmojiL();
                }
            break;

        case ESymgramCmdPhoto:
            if ( iAppView )
                {
                iAppView->PickPhotoL();
                }
            break;

        case ESymgramCmdOpen:
            if ( iAppView )
                {
                iAppView->OpenAttachmentL();
                }
            break;

        case ESymgramCmdSave:
            if ( iAppView )
                {
                iAppView->SaveAttachmentL();
                }
            break;

        case ESymgramCmdLogout:
            if ( iAppView )
                {
                iAppView->LogoutAskL();
                }
            break;

        case ESymgramCmdUpdate:
            if ( iAppView )
                {
                iAppView->CheckUpdateL();
                }
            break;

        case ESymgramCmdAbout:
            ShowAboutL();
            break;

        default:
            break;
        }
    }

void CSymgramAppUi::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    CAknAppUi::DynInitMenuPaneL( aResourceId, aMenuPane );
    if ( aResourceId != R_SYMGRAM_MENU || !iAppView || !aMenuPane )
        {
        return;
        }
    aMenuPane->SetItemDimmed( ESymgramCmdNext, !iAppView->ShowNextCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdCountry, !iAppView->ShowCountryCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdRefresh, !iAppView->ShowRefreshCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdContacts, !iAppView->ShowListCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdSettings, !iAppView->ShowListCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdWrite, !iAppView->ShowWriteCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdEmoji, !iAppView->ShowWriteCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdPhoto, !iAppView->ShowWriteCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdOpen, !iAppView->ShowAttachCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdSave, !iAppView->ShowAttachCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdLogout, !iAppView->ShowLogoutCommand() );
    aMenuPane->SetItemDimmed( ESymgramCmdUpdate, !iAppView->ShowListCommand() );
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
    // Leave Options, dialogs and other Avkon panes their own joystick.
    // Intercepting navi keys globally made the stock menu unusable.
    if ( aDestination && aDestination != iAppView )
        {
        CAknAppUi::HandleWsEventL( aEvent, aDestination );
        return;
        }
    CEikMenuBar* bar = NULL;
    if ( iEikonEnv && iEikonEnv->AppUiFactory() )
        {
        bar = iEikonEnv->AppUiFactory()->MenuBar();
        }
    if ( IsDisplayingMenuOrDialog() || ( bar && bar->IsDisplayed() ) )
        {
        CAknAppUi::HandleWsEventL( aEvent, aDestination );
        return;
        }
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
