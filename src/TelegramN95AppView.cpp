#include <coemain.h>
#include <aknutils.h>
#include "TelegramN95AppView.h"

CTelegramN95AppView* CTelegramN95AppView::NewL( const TRect& aRect )
    {
    CTelegramN95AppView* self = NewLC( aRect );
    CleanupStack::Pop( self );
    return self;
    }

CTelegramN95AppView* CTelegramN95AppView::NewLC( const TRect& aRect )
    {
    CTelegramN95AppView* self = new ( ELeave ) CTelegramN95AppView;
    CleanupStack::PushL( self );
    self->ConstructL( aRect );
    return self;
    }

CTelegramN95AppView::CTelegramN95AppView()
    : iFont( NULL )
    {
    }

CTelegramN95AppView::~CTelegramN95AppView()
    {
    }

void CTelegramN95AppView::ConstructL( const TRect& aRect )
    {
    CreateWindowL();

    iFont = AknLayoutUtils::FontFromId( EAknLogicalFontPrimaryFont );
    iStatus = _L( "Not connected" );

    SetRect( aRect );
    ActivateL();
    }

void CTelegramN95AppView::SetStatusL( const TDesC& aStatus )
    {
    iStatus.Copy( aStatus.Left( iStatus.MaxLength() ) );
    DrawDeferred();
    }

void CTelegramN95AppView::Draw( const TRect& /*aRect*/ ) const
    {
    CWindowGc& gc = SystemGc();
    TRect rect( Rect() );

    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbWhite );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.Clear( rect );

    if ( iFont && iStatus.Length() > 0 )
        {
        gc.UseFont( iFont );
        gc.SetPenStyle( CGraphicsContext::ESolidPen );
        gc.SetPenColor( KRgbBlack );
        gc.DrawText( iStatus, rect, rect.Height() / 2, CGraphicsContext::ECenter );
        gc.DiscardFont();
        }
    }

void CTelegramN95AppView::SizeChanged()
    {
    }
