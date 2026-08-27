#include <coemain.h>
#include <aknutils.h>
#include <stringloader.h>
#include <Symgram.rsg>

#include "SymgramVersion.h"
#include "SymgramAppView.h"

namespace
    {
    inline TRgb Brand()      { return TRgb( 0x22, 0x9E, 0xD9 ); }
    inline TRgb Paper()      { return TRgb( 0xFF, 0xFF, 0xFF ); }
    inline TRgb Ink()        { return TRgb( 0x00, 0x00, 0x00 ); }
    inline TRgb Muted()      { return TRgb( 0x8A, 0x8A, 0x8E ); }
    inline TRgb Rule()       { return TRgb( 0xEC, 0xEC, 0xEC ); }
    inline TRgb Highlight()  { return TRgb( 0xE3, 0xF1, 0xFB ); }

    // Telegram gives every conversation a coloured disc; the hue is derived from
    // the row so the same chat keeps its colour between redraws.
    inline TRgb AvatarColour( TInt aIndex )
        {
        switch ( aIndex % 6 )
            {
            case 0:  return TRgb( 0xE1, 0x7B, 0x60 );
            case 1:  return TRgb( 0x7B, 0xC8, 0x62 );
            case 2:  return TRgb( 0x65, 0xAA, 0xDD );
            case 3:  return TRgb( 0xA6, 0x95, 0xE7 );
            case 4:  return TRgb( 0xEE, 0x9C, 0x5A );
            default: return TRgb( 0xE0, 0x7B, 0xA0 );
            }
        }

    // Trims text to the available width, marking the cut with an ellipsis.
    // Note: AVKON already defines KEllipsis as a character constant, hence the
    // less obvious name here.
    void ClipText( TDes& aText, const CFont& aFont, TInt aWidth )
        {
        _LIT( KClipTail, "..." );

        if ( aWidth <= 0 )
            {
            aText.Zero();
            return;
            }
        if ( aFont.TextWidthInPixels( aText ) <= aWidth )
            {
            return;
            }

        const TInt tail = aFont.TextWidthInPixels( KClipTail );
        while ( aText.Length() > 0 &&
                aFont.TextWidthInPixels( aText ) + tail > aWidth )
            {
            aText.SetLength( aText.Length() - 1 );
            }
        if ( aText.Length() + KClipTail().Length() <= aText.MaxLength() )
            {
            aText.Append( KClipTail );
            }
        }
    }

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
    : iTitleFont( NULL ), iNameFont( NULL ), iTextFont( NULL ),
      iStatus( NULL ), iSignInTitle( NULL ), iSignInDetail( NULL ),
      iEmptyTitle( NULL ), iEmptyDetail( NULL ), iSignedIn( EFalse ),
      iSelected( 0 ), iTopRow( 0 )
    {
    }

CSymgramAppView::~CSymgramAppView()
    {
    iChats.Close();
    delete iStatus;
    delete iSignInTitle;
    delete iSignInDetail;
    delete iEmptyTitle;
    delete iEmptyDetail;
    }

void CSymgramAppView::ConstructL( const TRect& aRect )
    {
    CreateWindowL();

    iTitleFont = AknLayoutUtils::FontFromId( EAknLogicalFontSecondaryFont );
    iNameFont  = AknLayoutUtils::FontFromId( EAknLogicalFontPrimaryFont );
    iTextFont  = AknLayoutUtils::FontFromId( EAknLogicalFontSecondaryFont );

    iStatus       = StringLoader::LoadL( R_SYMGRAM_STATUS_UNSIGNED );
    iSignInTitle  = StringLoader::LoadL( R_SYMGRAM_SIGNIN_TITLE );
    iSignInDetail = StringLoader::LoadL( R_SYMGRAM_SIGNIN_DETAIL );
    iEmptyTitle   = StringLoader::LoadL( R_SYMGRAM_EMPTY_TITLE );
    iEmptyDetail  = StringLoader::LoadL( R_SYMGRAM_EMPTY_DETAIL );

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

TInt CSymgramAppView::HeaderHeight() const
    {
    const TInt h = iTitleFont ? iTitleFont->HeightInPixels() : 14;
    return h + 12;
    }

TInt CSymgramAppView::RowHeight() const
    {
    const TInt name = iNameFont ? iNameFont->HeightInPixels() : 16;
    const TInt text = iTextFont ? iTextFont->HeightInPixels() : 14;
    return name + text + 14;
    }

TRect CSymgramAppView::ListRect() const
    {
    TRect list( Rect() );
    list.iTl.iY += HeaderHeight();
    return list;
    }

TInt CSymgramAppView::VisibleRows() const
    {
    const TInt h = RowHeight();
    return h > 0 ? ListRect().Height() / h : 0;
    }

void CSymgramAppView::EnsureSelectionVisible()
    {
    const TInt rows = VisibleRows();
    if ( rows <= 0 )
        {
        return;
        }

    if ( iSelected < iTopRow )
        {
        iTopRow = iSelected;
        }
    else if ( iSelected >= iTopRow + rows )
        {
        iTopRow = iSelected - rows + 1;
        }

    if ( iTopRow < 0 )
        {
        iTopRow = 0;
        }
    }

TKeyResponse CSymgramAppView::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                              TEventCode aType )
    {
    if ( aType != EEventKey || iChats.Count() == 0 )
        {
        return EKeyWasNotConsumed;
        }

    switch ( aKeyEvent.iCode )
        {
        case EKeyUpArrow:
            if ( iSelected > 0 )
                {
                iSelected--;
                EnsureSelectionVisible();
                DrawDeferred();
                }
            return EKeyWasConsumed;

        case EKeyDownArrow:
            if ( iSelected < iChats.Count() - 1 )
                {
                iSelected++;
                EnsureSelectionVisible();
                DrawDeferred();
                }
            return EKeyWasConsumed;

        default:
            return EKeyWasNotConsumed;
        }
    }

void CSymgramAppView::Draw( const TRect& /*aRect*/ ) const
    {
    CWindowGc& gc = SystemGc();
    const TRect rect( Rect() );

    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.SetBrushColor( Paper() );
    gc.DrawRect( rect );

    const TRect header( rect.iTl, TSize( rect.Width(), HeaderHeight() ) );
    DrawHeader( gc, header );

    const TRect list( ListRect() );
    if ( !iSignedIn )
        {
        DrawSignIn( gc, list );
        return;
        }
    if ( iChats.Count() == 0 )
        {
        DrawEmptyState( gc, list );
        return;
        }

    const TInt rowH = RowHeight();
    const TInt rows = VisibleRows();
    for ( TInt i = 0; i < rows; i++ )
        {
        const TInt index = iTopRow + i;
        if ( index >= iChats.Count() )
            {
            break;
            }

        const TRect row( list.iTl.iX,
                         list.iTl.iY + i * rowH,
                         list.iBr.iX,
                         list.iTl.iY + ( i + 1 ) * rowH );
        DrawRow( gc, row, index );
        }
    }

void CSymgramAppView::DrawHeader( CWindowGc& aGc, const TRect& aRect ) const
    {
    aGc.SetPenStyle( CGraphicsContext::ENullPen );
    aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    aGc.SetBrushColor( Brand() );
    aGc.DrawRect( aRect );

    if ( !iTitleFont )
        {
        return;
        }

    aGc.UseFont( iTitleFont );
    aGc.SetPenStyle( CGraphicsContext::ESolidPen );
    aGc.SetPenColor( Paper() );
    aGc.SetBrushStyle( CGraphicsContext::ENullBrush );

    const TInt baseline =
        ( aRect.Height() + iTitleFont->AscentInPixels() ) / 2;

    TRect left( aRect );
    left.iTl.iX += 6;
    aGc.DrawText( KSymgramAppName(), left, baseline, CGraphicsContext::ELeft );

    if ( iStatus )
        {
        TRect right( aRect );
        right.iBr.iX -= 6;
        aGc.DrawText( *iStatus, right, baseline, CGraphicsContext::ERight );
        }

    aGc.DiscardFont();
    }

void CSymgramAppView::DrawRow( CWindowGc& aGc, const TRect& aRect,
                               TInt aIndex ) const
    {
    const TSymgramChat& chat = iChats[ aIndex ];

    aGc.SetPenStyle( CGraphicsContext::ENullPen );
    aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    aGc.SetBrushColor( aIndex == iSelected ? Highlight() : Paper() );
    aGc.DrawRect( aRect );

    // --- avatar disc with the first letter of the chat name ---
    const TInt diameter = aRect.Height() - 12;
    const TRect avatar( aRect.iTl.iX + 6,
                        aRect.iTl.iY + 6,
                        aRect.iTl.iX + 6 + diameter,
                        aRect.iTl.iY + 6 + diameter );

    aGc.SetBrushColor( AvatarColour( aIndex ) );
    aGc.DrawEllipse( avatar );

    if ( iNameFont && chat.iName.Length() > 0 )
        {
        aGc.UseFont( iNameFont );
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( Paper() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );

        TBuf<1> initial;
        initial.Append( chat.iName[ 0 ] );
        aGc.DrawText( initial, avatar,
                      ( avatar.Height() + iNameFont->AscentInPixels() ) / 2,
                      CGraphicsContext::ECenter );

        aGc.DiscardFont();
        }

    if ( !iNameFont || !iTextFont )
        {
        return;
        }

    const TInt textLeft = avatar.iBr.iX + 8;
    const TInt rightEdge = aRect.iBr.iX - 6;

    const TInt nameH = iNameFont->HeightInPixels();
    const TInt textH = iTextFont->HeightInPixels();
    const TInt top = aRect.iTl.iY + ( aRect.Height() - nameH - textH - 2 ) / 2;

    // --- timestamp, right aligned on the first line ---
    TInt timeWidth = 0;
    if ( chat.iTime.Length() > 0 )
        {
        timeWidth = iTextFont->TextWidthInPixels( chat.iTime );

        const TRect timeBox( rightEdge - timeWidth, top, rightEdge, top + nameH );
        aGc.UseFont( iTextFont );
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( Muted() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
        aGc.DrawText( chat.iTime, timeBox, iTextFont->AscentInPixels(),
                      CGraphicsContext::ERight );
        aGc.DiscardFont();
        }

    // --- unread badge, right aligned on the second line ---
    TInt badgeWidth = 0;
    if ( chat.iUnread > 0 )
        {
        TBuf<8> count;
        count.Num( chat.iUnread );

        const TInt badgeH = textH + 4;
        badgeWidth = iTextFont->TextWidthInPixels( count ) + badgeH;

        const TRect badge( rightEdge - badgeWidth, top + nameH + 2,
                           rightEdge, top + nameH + 2 + badgeH );

        aGc.SetPenStyle( CGraphicsContext::ENullPen );
        aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
        aGc.SetBrushColor( Brand() );
        aGc.DrawRoundRect( badge, TSize( badgeH / 2, badgeH / 2 ) );

        aGc.UseFont( iTextFont );
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( Paper() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
        aGc.DrawText( count, badge, iTextFont->AscentInPixels() + 2,
                      CGraphicsContext::ECenter );
        aGc.DiscardFont();

        badgeWidth += 6;
        }

    // --- chat name ---
    TBuf<40> name( chat.iName );
    ClipText( name, *iNameFont,
              rightEdge - textLeft - ( timeWidth > 0 ? timeWidth + 6 : 0 ) );

    aGc.UseFont( iNameFont );
    aGc.SetPenStyle( CGraphicsContext::ESolidPen );
    aGc.SetPenColor( Ink() );
    aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
    aGc.DrawText( name,
                  TRect( textLeft, top, rightEdge - timeWidth - 6, top + nameH ),
                  iNameFont->AscentInPixels(), CGraphicsContext::ELeft );
    aGc.DiscardFont();

    // --- last message ---
    TBuf<80> preview( chat.iPreview );
    ClipText( preview, *iTextFont, rightEdge - textLeft - badgeWidth );

    aGc.UseFont( iTextFont );
    aGc.SetPenColor( Muted() );
    aGc.DrawText( preview,
                  TRect( textLeft, top + nameH + 2,
                         rightEdge - badgeWidth, top + nameH + 2 + textH ),
                  iTextFont->AscentInPixels(), CGraphicsContext::ELeft );
    aGc.DiscardFont();

    // --- separator, inset to line up with the text column ---
    aGc.SetPenStyle( CGraphicsContext::ESolidPen );
    aGc.SetPenColor( Rule() );
    aGc.DrawLine( TPoint( textLeft, aRect.iBr.iY - 1 ),
                  TPoint( aRect.iBr.iX, aRect.iBr.iY - 1 ) );
    }

void CSymgramAppView::DrawSignIn( CWindowGc& aGc, const TRect& aRect ) const
    {
    DrawCenteredPair( aGc, aRect, iSignInTitle, iSignInDetail );
    }

void CSymgramAppView::DrawEmptyState( CWindowGc& aGc, const TRect& aRect ) const
    {
    DrawCenteredPair( aGc, aRect, iEmptyTitle, iEmptyDetail );
    }

void CSymgramAppView::DrawCenteredPair( CWindowGc& aGc, const TRect& aRect,
                                        const HBufC* aTitle,
                                        const HBufC* aDetail ) const
    {
    if ( !iNameFont || !iTextFont )
        {
        return;
        }

    const TInt titleH  = iNameFont->HeightInPixels();
    const TInt detailH = iTextFont->HeightInPixels();
    const TInt top = aRect.iTl.iY + ( aRect.Height() - titleH - detailH - 6 ) / 2;

    aGc.SetPenStyle( CGraphicsContext::ESolidPen );
    aGc.SetBrushStyle( CGraphicsContext::ENullBrush );

    if ( aTitle )
        {
        aGc.UseFont( iNameFont );
        aGc.SetPenColor( Ink() );
        aGc.DrawText( *aTitle,
                      TRect( aRect.iTl.iX, top, aRect.iBr.iX, top + titleH ),
                      iNameFont->AscentInPixels(), CGraphicsContext::ECenter );
        aGc.DiscardFont();
        }

    if ( aDetail )
        {
        aGc.UseFont( iTextFont );
        aGc.SetPenColor( Muted() );
        aGc.DrawText( *aDetail,
                      TRect( aRect.iTl.iX, top + titleH + 6,
                             aRect.iBr.iX, top + titleH + 6 + detailH ),
                      iTextFont->AscentInPixels(), CGraphicsContext::ECenter );
        aGc.DiscardFont();
        }
    }

void CSymgramAppView::SizeChanged()
    {
    EnsureSelectionVisible();
    }
