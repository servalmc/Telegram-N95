#include <coemain.h>
#include <aknutils.h>
#include <stringloader.h>
#include <Symgram.rsg>

#include "SymgramAppView.h"

CSymgramAppView* CSymgramAppView::NewL( const TRect& aRect )
    {
    CSymgramAppView* self = NewLC( aRect );
    CleanupStack::Pop( self );
    return self;
    }

CSymgramAppView* CSymgramAppView::NewLC( const TRect& aRect )
    {
    CSymgramAppView* self = new ( ELeave ) CSymgramAppView;
    CleanupStack::PushL( self );
    self->ConstructL( aRect );
    return self;
    }

CSymgramAppView::CSymgramAppView()
    : iFont( NULL ), iStatus( NULL )
    {
    }

CSymgramAppView::~CSymgramAppView()
    {
    delete iStatus;
    }

void CSymgramAppView::ConstructL( const TRect& aRect )
    {
    CreateWindowL();

    iFont = AknLayoutUtils::FontFromId( EAknLogicalFontPrimaryFont );
    iStatus = StringLoader::LoadL( R_SYMGRAM_STATUS_OFFLINE );

    SetRect( aRect );
    ActivateL();
    }

void CSymgramAppView::SetStatusL( const TDesC& aStatus )
    {
    HBufC* status = aStatus.AllocL();
    delete iStatus;
    iStatus = status;
    DrawDeferred();
    }

void CSymgramAppView::Draw( const TRect& /*aRect*/ ) const
    {
    CWindowGc& gc = SystemGc();
    TRect rect( Rect() );

    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbWhite );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.Clear( rect );

    if ( iFont && iStatus )
        {
        gc.UseFont( iFont );
        gc.SetPenStyle( CGraphicsContext::ESolidPen );
        gc.SetPenColor( KRgbBlack );
        gc.DrawText( *iStatus, rect, rect.Height() / 2, CGraphicsContext::ECenter );
        gc.DiscardFont();
        }
    }

void CSymgramAppView::SizeChanged()
    {
    }
