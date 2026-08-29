#include <coemain.h>
#include <eikenv.h>
#include <e32keys.h>
#include <badesca.h>
#include <aknutils.h>
#include <stringloader.h>
#include <aknnotewrappers.h>
#include <aknquerydialog.h>
#include <e32err.h>
#include <fbs.h>
#include <imageconversion.h>
#include <f32file.h>
#include <DocumentHandler.h>
#include <apmstd.h>
#include <PathInfo.h>
#include <w32std.h>
#include <Symgram.rsg>

#include "SymgramVersion.h"
#include "SymgramAppView.h"
#include "SymgramStore.h"

namespace
    {
    const TInt KCallingCode[] = { 7, 375, 7, 380, 49, 1, 44, 358 };
    const TInt KCountryCount = 8;
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

    TUint NextCp( const TDesC& aText, TInt& aI )
        {
        if ( aI >= aText.Length() )
            {
            return 0;
            }
        const TUint c = aText[ aI++ ];
        if ( c >= 0xD800 && c <= 0xDBFF && aI < aText.Length() )
            {
            const TUint lo = aText[ aI ];
            if ( lo >= 0xDC00 && lo <= 0xDFFF )
                {
                ++aI;
                return 0x10000 + ( ( c - 0xD800 ) << 10 ) + ( lo - 0xDC00 );
                }
            }
        return c;
        }

    TBool SkipCp( TUint aCp )
        {
        return aCp == 0x200D || aCp == 0x20E3 ||
               ( aCp >= 0xFE00 && aCp <= 0xFE0F ) ||
               ( aCp >= 0x1F3FB && aCp <= 0x1F3FF ) ||
               ( aCp >= 0xE0020 && aCp <= 0xE007F );
        }

    TBool IsEmojiCp( TUint aCp )
        {
        if ( SkipCp( aCp ) )
            {
            return EFalse;
            }
        if ( aCp >= 0x10000 )
            {
            return ETrue;
            }
        if ( ( aCp >= 0x2600 && aCp <= 0x27BF ) ||
             aCp == 0x2B50 || aCp == 0x2B55 ||
             aCp == 0x3030 || aCp == 0x303D ||
             aCp == 0x3297 || aCp == 0x3299 )
            {
            return ETrue;
            }
        return EFalse;
        }

    // 0 generic, 1 heart, 2 up, 3 down, 4 fire, 5 star, 6 wink, 7 sad, 8 smile
    TInt EmojiStyle( TUint aCp )
        {
        if ( aCp == 0x2764 || aCp == 0x2665 || aCp == 0x2763 || aCp == 0x2661 ||
             aCp == 0x1F5A4 || aCp == 0x1F90D || aCp == 0x1F90E ||
             aCp == 0x1FA77 ||
             ( aCp >= 0x1F493 && aCp <= 0x1F49F ) )
            {
            return 1;
            }
        if ( aCp == 0x1F44D || aCp == 0x1F44C )
            {
            return 2;
            }
        if ( aCp == 0x1F44E )
            {
            return 3;
            }
        if ( aCp == 0x1F525 )
            {
            return 4;
            }
        if ( aCp == 0x2B50 || aCp == 0x1F31F || aCp == 0x2605 )
            {
            return 5;
            }
        if ( aCp == 0x1F609 )
            {
            return 6;
            }
        if ( aCp == 0x1F61E || aCp == 0x1F614 || aCp == 0x1F622 ||
             aCp == 0x1F62D || aCp == 0x1F641 || aCp == 0x2639 ||
             ( aCp >= 0x1F625 && aCp <= 0x1F62B ) )
            {
            return 7;
            }
        if ( ( aCp >= 0x1F600 && aCp <= 0x1F64F ) ||
             aCp == 0x263A )
            {
            return 8;
            }
        return 0;
        }

    const TUint KEmojiList[] =
        {
        0x1F600, 0x1F602, 0x1F60A, 0x1F60D, 0x1F609, 0x1F618,
        0x1F622, 0x1F62D, 0x1F621, 0x1F44D, 0x1F44E, 0x1F44C,
        0x2764,  0x1F525, 0x2B50,  0x1F389, 0x1F44F, 0x1F64F,
        0x1F60E, 0x1F914, 0x1F605, 0x1F923, 0x1F49C, 0x2728
        };
    const TInt KEmojiCount = 24;

    TInt EmojiSlot( TUint aCp )
        {
        TInt i = 0;
        for ( i = 0; i < KEmojiCount; i++ )
            {
            if ( KEmojiList[ i ] == aCp )
                {
                return i;
                }
            }
        return -1;
        }

    void FoldVisible( TDes& aText )
        {
        TBuf<140> tmp;
        TInt i = 0;
        while ( i < aText.Length() && tmp.Length() < tmp.MaxLength() )
            {
            const TUint cp = NextCp( aText, i );
            if ( SkipCp( cp ) )
                {
                continue;
                }
            if ( IsEmojiCp( cp ) )
                {
                _LIT( KHeart, "<3" );
                _LIT( KSm, ":)" );
                _LIT( KSd, ":(" );
                _LIT( KWk, ";)" );
                _LIT( KUp, "+1" );
                _LIT( KDn, "-1" );
                switch ( EmojiStyle( cp ) )
                    {
                    case 1:
                        if ( tmp.Length() + 2 <= tmp.MaxLength() )
                            {
                            tmp.Append( KHeart );
                            }
                        break;
                    case 2:
                        if ( tmp.Length() + 2 <= tmp.MaxLength() )
                            {
                            tmp.Append( KUp );
                            }
                        break;
                    case 3:
                        if ( tmp.Length() + 2 <= tmp.MaxLength() )
                            {
                            tmp.Append( KDn );
                            }
                        break;
                    case 6:
                        if ( tmp.Length() + 2 <= tmp.MaxLength() )
                            {
                            tmp.Append( KWk );
                            }
                        break;
                    case 7:
                        if ( tmp.Length() + 2 <= tmp.MaxLength() )
                            {
                            tmp.Append( KSd );
                            }
                        break;
                    case 8:
                        if ( tmp.Length() + 2 <= tmp.MaxLength() )
                            {
                            tmp.Append( KSm );
                            }
                        break;
                    default:
                        if ( tmp.Length() < tmp.MaxLength() )
                            {
                            tmp.Append( '*' );
                            }
                        break;
                    }
                }
            else if ( cp <= 0xFFFF )
                {
                tmp.Append( (TText)cp );
                }
            }
        aText.Copy( tmp.Left( aText.MaxLength() ) );
        }

    void DrawEmojiIcon( CWindowGc& aGc, TInt aX, TInt aY, TInt aSz, TUint aCp,
                        CFbsBitmap* aBmp, CFbsBitmap* aMask )
        {
        if ( aBmp && aMask && aSz > 4 )
            {
            const TSize srcSz = aBmp->SizeInPixels();
            if ( srcSz.iWidth > 0 && srcSz.iHeight > 0 )
                {
                aGc.DrawBitmapMasked( TRect( aX, aY, aX + aSz, aY + aSz ),
                                      aBmp, TRect( TPoint( 0, 0 ), srcSz ),
                                      aMask, EFalse );
                return;
                }
            }
        const TInt st = EmojiStyle( aCp );
        const TRect r( aX, aY, aX + aSz, aY + aSz );
        aGc.SetPenStyle( CGraphicsContext::ENullPen );
        aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
        if ( st == 1 )
            {
            aGc.SetBrushColor( TRgb( 0xE2, 0x3D, 0x4B ) );
            const TInt m = aSz / 2;
            aGc.DrawEllipse( TRect( aX, aY + aSz / 8,
                                    aX + m + 1, aY + m + aSz / 8 ) );
            aGc.DrawEllipse( TRect( aX + m - 1, aY + aSz / 8,
                                    aX + aSz, aY + m + aSz / 8 ) );
            TPoint p[ 3 ];
            p[ 0 ].iX = aX;
            p[ 0 ].iY = aY + aSz / 3 + 1;
            p[ 1 ].iX = aX + aSz;
            p[ 1 ].iY = aY + aSz / 3 + 1;
            p[ 2 ].iX = aX + m;
            p[ 2 ].iY = aY + aSz - 1;
            aGc.DrawPolygon( p, 3 );
            return;
            }
        if ( st == 4 )
            {
            aGc.SetBrushColor( TRgb( 0xFF, 0x6A, 0x00 ) );
            TPoint p[ 3 ];
            p[ 0 ].iX = aX + aSz / 2;
            p[ 0 ].iY = aY;
            p[ 1 ].iX = aX;
            p[ 1 ].iY = aY + aSz;
            p[ 2 ].iX = aX + aSz;
            p[ 2 ].iY = aY + aSz;
            aGc.DrawPolygon( p, 3 );
            return;
            }
        if ( st == 5 )
            {
            aGc.SetBrushColor( TRgb( 0xFF, 0xC1, 0x07 ) );
            TPoint p[ 4 ];
            p[ 0 ].iX = aX + aSz / 2;
            p[ 0 ].iY = aY;
            p[ 1 ].iX = aX + aSz;
            p[ 1 ].iY = aY + aSz / 2;
            p[ 2 ].iX = aX + aSz / 2;
            p[ 2 ].iY = aY + aSz;
            p[ 3 ].iX = aX;
            p[ 3 ].iY = aY + aSz / 2;
            aGc.DrawPolygon( p, 4 );
            return;
            }
        if ( st == 2 || st == 3 )
            {
            aGc.SetBrushColor( st == 2 ? TRgb( 0x2E, 0x7D, 0x32 )
                                       : TRgb( 0xC6, 0x28, 0x28 ) );
            aGc.DrawEllipse( r );
            aGc.SetPenStyle( CGraphicsContext::ESolidPen );
            aGc.SetPenColor( KRgbWhite );
            aGc.SetPenSize( TSize( 2, 2 ) );
            const TInt cx = aX + aSz / 2;
            const TInt cy = aY + aSz / 2;
            aGc.DrawLine( TPoint( cx, aY + aSz / 4 ),
                          TPoint( cx, aY + aSz - aSz / 4 ) );
            if ( st == 2 )
                {
                aGc.DrawLine( TPoint( aX + aSz / 4, cy ),
                              TPoint( aX + aSz - aSz / 4, cy ) );
                }
            aGc.SetPenSize( TSize( 1, 1 ) );
            return;
            }
        aGc.SetBrushColor( st == 7 ? TRgb( 0x90, 0xCA, 0xF9 )
                                   : TRgb( 0xFF, 0xCC, 0x33 ) );
        aGc.DrawEllipse( r );
        aGc.SetBrushColor( KRgbBlack );
        const TInt e = ( aSz / 7 < 2 ) ? 2 : aSz / 7;
        const TInt ey = aY + aSz / 3;
        if ( st == 6 )
            {
            aGc.SetPenStyle( CGraphicsContext::ESolidPen );
            aGc.SetPenColor( KRgbBlack );
            aGc.DrawLine( TPoint( aX + aSz / 4, ey ),
                          TPoint( aX + aSz / 4 + e + 1, ey ) );
            aGc.SetPenStyle( CGraphicsContext::ENullPen );
            aGc.DrawEllipse( TRect( aX + aSz - aSz / 4 - e, ey - e / 2,
                                    aX + aSz - aSz / 4, ey + e / 2 + 1 ) );
            }
        else
            {
            aGc.DrawEllipse( TRect( aX + aSz / 4, ey - e / 2,
                                    aX + aSz / 4 + e, ey + e / 2 + 1 ) );
            aGc.DrawEllipse( TRect( aX + aSz - aSz / 4 - e, ey - e / 2,
                                    aX + aSz - aSz / 4, ey + e / 2 + 1 ) );
            }
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( KRgbBlack );
        if ( st == 7 )
            {
            aGc.DrawLine( TPoint( aX + aSz / 4, aY + 3 * aSz / 4 ),
                          TPoint( aX + 3 * aSz / 4, aY + 5 * aSz / 8 ) );
            }
        else
            {
            aGc.DrawLine( TPoint( aX + aSz / 4, aY + 5 * aSz / 8 ),
                          TPoint( aX + aSz / 2, aY + 3 * aSz / 4 ) );
            aGc.DrawLine( TPoint( aX + aSz / 2, aY + 3 * aSz / 4 ),
                          TPoint( aX + 3 * aSz / 4, aY + 5 * aSz / 8 ) );
            }
        }

    void DrawMsgText( CWindowGc& aGc, const CFont& aFont, const TRect& aBox,
                      const TDesC& aText, CFbsBitmap* const* aBmp,
                      CFbsBitmap* const* aMask )
        {
        TInt x = aBox.iTl.iX;
        const TInt y = aBox.iTl.iY;
        const TInt maxX = aBox.iBr.iX;
        TInt sz = aFont.HeightInPixels();
        if ( sz < 12 )
            {
            sz = 12;
            }
        TInt i = 0;
        TBuf<96> run;
        aGc.UseFont( &aFont );
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( Ink() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
        while ( i < aText.Length() && x < maxX )
            {
            const TUint cp = NextCp( aText, i );
            if ( SkipCp( cp ) )
                {
                continue;
                }
            if ( IsEmojiCp( cp ) )
                {
                if ( run.Length() > 0 )
                    {
                    const TInt w = aFont.TextWidthInPixels( run );
                    TRect tb( x, y, x + w, aBox.iBr.iY );
                    aGc.DrawText( run, tb, aFont.AscentInPixels(),
                                  CGraphicsContext::ELeft );
                    x += w;
                    run.Zero();
                    }
                if ( x + sz > maxX )
                    {
                    break;
                    }
                CFbsBitmap* bmp = NULL;
                CFbsBitmap* mask = NULL;
                const TInt slot = EmojiSlot( cp );
                if ( slot >= 0 && aBmp && aMask )
                    {
                    bmp = aBmp[ slot ];
                    mask = aMask[ slot ];
                    }
                DrawEmojiIcon( aGc, x,
                               y + ( aFont.HeightInPixels() - sz ) / 2,
                               sz, cp, bmp, mask );
                aGc.UseFont( &aFont );
                aGc.SetPenStyle( CGraphicsContext::ESolidPen );
                aGc.SetPenColor( Ink() );
                aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
                x += sz + 1;
                }
            else if ( cp <= 0xFFFF )
                {
                TBuf<2> ch;
                ch.Append( (TText)cp );
                const TInt cw = aFont.TextWidthInPixels( ch );
                const TInt rw = aFont.TextWidthInPixels( run );
                if ( x + rw + cw > maxX )
                    {
                    if ( run.Length() > 0 )
                        {
                        TRect tb( x, y, maxX, aBox.iBr.iY );
                        aGc.DrawText( run, tb, aFont.AscentInPixels(),
                                      CGraphicsContext::ELeft );
                        }
                    run.Zero();
                    break;
                    }
                if ( run.Length() < run.MaxLength() )
                    {
                    run.Append( (TText)cp );
                    }
                }
            }
        if ( run.Length() > 0 && x < maxX )
            {
            TRect tb( x, y, maxX, aBox.iBr.iY );
            aGc.DrawText( run, tb, aFont.AscentInPixels(),
                          CGraphicsContext::ELeft );
            }
        aGc.DiscardFont();
        }

    void AppendCp( TDes& aOut, TUint aCp )
        {
        if ( aCp > 0xFFFF )
            {
            if ( aOut.Length() + 2 <= aOut.MaxLength() )
                {
                const TUint u = aCp - 0x10000;
                aOut.Append( (TText)( 0xD800 | ( u >> 10 ) ) );
                aOut.Append( (TText)( 0xDC00 | ( u & 0x3FF ) ) );
                }
            }
        else if ( aOut.Length() < aOut.MaxLength() )
            {
            aOut.Append( (TText)aCp );
            }
        }

    void ToUtf8( const TDesC& aIn, TDes8& aOut )
        {
        aOut.Zero();
        TInt i = 0;
        while ( i < aIn.Length() && aOut.Length() + 4 <= aOut.MaxLength() )
            {
            const TUint cp = NextCp( aIn, i );
            if ( cp < 0x80 )
                {
                aOut.Append( (TUint8)cp );
                }
            else if ( cp < 0x800 )
                {
                aOut.Append( (TUint8)( 0xC0 | ( cp >> 6 ) ) );
                aOut.Append( (TUint8)( 0x80 | ( cp & 0x3F ) ) );
                }
            else if ( cp < 0x10000 )
                {
                aOut.Append( (TUint8)( 0xE0 | ( cp >> 12 ) ) );
                aOut.Append( (TUint8)( 0x80 | ( ( cp >> 6 ) & 0x3F ) ) );
                aOut.Append( (TUint8)( 0x80 | ( cp & 0x3F ) ) );
                }
            else
                {
                aOut.Append( (TUint8)( 0xF0 | ( cp >> 18 ) ) );
                aOut.Append( (TUint8)( 0x80 | ( ( cp >> 12 ) & 0x3F ) ) );
                aOut.Append( (TUint8)( 0x80 | ( ( cp >> 6 ) & 0x3F ) ) );
                aOut.Append( (TUint8)( 0x80 | ( cp & 0x3F ) ) );
                }
            }
        }
    }

class CSymgramJpeg : public CActive
    {
    public:
        CSymgramJpeg( CSymgramAppView& aView )
            : CActive( EPriorityStandard ), iView( aView ),
              iDec( NULL ), iBmp( NULL ), iMsgId( 0 )
            {
            CActiveScheduler::Add( this );
            }
        ~CSymgramJpeg()
            {
            Cancel();
            delete iDec;
            delete iBmp;
            }
        void DecodeL( TInt aMsgId, const TDesC8& aJpeg )
            {
            Cancel();
            delete iDec;
            iDec = NULL;
            delete iBmp;
            iBmp = NULL;
            iMsgId = aMsgId;
            RFs& fs = CCoeEnv::Static()->FsSession();
            iDec = CImageDecoder::DataNewL( fs, aJpeg );
            const TFrameInfo inf = iDec->FrameInfo();
            TSize sz = inf.iOverallSizeInPixels;
            if ( sz.iWidth < 1 || sz.iHeight < 1 )
                {
                delete iDec;
                iDec = NULL;
                return;
                }
            if ( sz.iWidth > 96 )
                {
                sz.iHeight = sz.iHeight * 96 / sz.iWidth;
                sz.iWidth = 96;
                }
            if ( sz.iHeight < 1 )
                {
                sz.iHeight = 1;
                }
            iBmp = new ( ELeave ) CFbsBitmap;
            User::LeaveIfError( iBmp->Create( sz, EColor64K ) );
            iDec->Convert( &iStatus, *iBmp );
            SetActive();
            }
        void RunL()
            {
            CFbsBitmap* bmp = iBmp;
            iBmp = NULL;
            delete iDec;
            iDec = NULL;
            if ( iStatus.Int() != KErrNone )
                {
                delete bmp;
                iView.JpegReady( NULL, iMsgId );
                return;
                }
            iView.JpegReady( bmp, iMsgId );
            }
        void DoCancel()
            {
            if ( iDec )
                {
                iDec->Cancel();
                }
            }

    private:
        CSymgramAppView& iView;
        CImageDecoder* iDec;
        CFbsBitmap* iBmp;
        TInt iMsgId;
    };

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
      iChatFont( NULL ),
      iStatus( NULL ), iSignInTitle( NULL ), iSignInHint( NULL ),
      iEmptyTitle( NULL ), iEmptyDetail( NULL ), iCodeTitle( NULL ), iCodeHint( NULL ),
      iPasswordPrompt( NULL ), iPasswordHint( NULL ), iFieldCountry( NULL ),
      iFieldPhone( NULL ), iChatHint( NULL ),
      iCountries( NULL ), iSignedIn( EFalse ), iAwaitingCode( EFalse ),
      iAwaitingPassword( EFalse ), iPickingCountry( EFalse ), iFocus( 0 ),
      iCountry( 2 ), iSession( NULL ), iStore( NULL ), iJpeg( NULL ),
      iUpdate( NULL ),
      iNavDown( 0 ), iTab( 0 ), iPane( 0 ), iSelected( 0 ), iTopRow( 0 ),
      iInChat( EFalse ), iOpenId( 0 ), iThumbAt( 0 ), iSetSel( 0 ),
      iPick( 0 ), iPickSel( 0 ), iMsgSel( 0 ), iFiles( NULL )
    {
    TInt e = 0;
    for ( e = 0; e < KEmojiCount; e++ )
        {
        iEmojiBmp[ e ] = NULL;
        iEmojiMask[ e ] = NULL;
        }
    }

CSymgramAppView::~CSymgramAppView()
    {
    TInt e = 0;
    for ( e = 0; e < KEmojiCount; e++ )
        {
        delete iEmojiBmp[ e ];
        iEmojiBmp[ e ] = NULL;
        delete iEmojiMask[ e ];
        iEmojiMask[ e ] = NULL;
        }
    if ( iChatFont )
        {
        iCoeEnv->ReleaseScreenFont( iChatFont );
        iChatFont = NULL;
        }
    delete iJpeg;
    iJpeg = NULL;
    ClearMessages();
    iChats.Close();
    iContacts.Close();
    iMsgs.Close();
    delete iFiles;
    iFiles = NULL;
    delete iSession;
    iSession = NULL;
    delete iUpdate;
    iUpdate = NULL;
    delete iStore;
    delete iCountries;
    delete iStatus;
    delete iSignInTitle;
    delete iSignInHint;
    delete iEmptyTitle;
    delete iEmptyDetail;
    delete iCodeTitle;
    delete iCodeHint;
    delete iPasswordPrompt;
    delete iPasswordHint;
    delete iFieldCountry;
    delete iFieldPhone;
    delete iChatHint;
    }

void CSymgramAppView::ConstructL( const TRect& aRect )
    {
    CreateWindowL();

    iTitleFont = AknLayoutUtils::FontFromId( EAknLogicalFontSecondaryFont );
    iNameFont  = AknLayoutUtils::FontFromId( EAknLogicalFontPrimaryFont );
    iTextFont  = AknLayoutUtils::FontFromId( EAknLogicalFontSecondaryFont );
    TInt e = 0;
    for ( e = 0; e < KEmojiCount; e++ )
        {
        iEmojiBmp[ e ] = NULL;
        iEmojiMask[ e ] = NULL;
        }
    if ( iTextFont )
        {
        TFontSpec spec = iTextFont->FontSpecInTwips();
        spec.iHeight = ( spec.iHeight * 5 ) / 2;
        TRAPD( ferr, iChatFont = iCoeEnv->CreateScreenFontL( spec ) );
        if ( ferr != KErrNone )
            {
            iChatFont = NULL;
            }
        }
    TRAP_IGNORE( LoadEmojiL() );

    iStatus      = StringLoader::LoadL( R_SYMGRAM_STATUS_UNSIGNED );
    iSignInTitle = StringLoader::LoadL( R_SYMGRAM_SIGNIN_TITLE );
    iSignInHint  = StringLoader::LoadL( R_SYMGRAM_SIGNIN_HINT );
    iEmptyTitle  = StringLoader::LoadL( R_SYMGRAM_EMPTY_TITLE );
    iEmptyDetail = StringLoader::LoadL( R_SYMGRAM_EMPTY_DETAIL );
    iCodeTitle   = StringLoader::LoadL( R_SYMGRAM_CODE_TITLE );
    iCodeHint    = StringLoader::LoadL( R_SYMGRAM_CODE_HINT );
    iPasswordPrompt = StringLoader::LoadL( R_SYMGRAM_PASSWORD_PROMPT );
    iPasswordHint = StringLoader::LoadL( R_SYMGRAM_PASSWORD_HINT );
    iFieldCountry = StringLoader::LoadL( R_SYMGRAM_FIELD_COUNTRY );
    iFieldPhone = StringLoader::LoadL( R_SYMGRAM_FIELD_PHONE );
    iChatHint = StringLoader::LoadL( R_SYMGRAM_CHAT_HINT );

    iCountries = CEikonEnv::Static()->ReadDesCArrayResourceL( R_SYMGRAM_COUNTRIES );
    iStore = CSymgramStore::NewL();
    iJpeg = new ( ELeave ) CSymgramJpeg( *this );
    iUpdate = CSymgramUpdate::NewL( *this );
    iSession = CSymgramSession::NewL( *this );
    TRAP_IGNORE( iStore->LoadChatsL( iChats ) );
    TRAP_IGNORE( iStore->LoadContactsL( iContacts ) );
    if ( iSession->HasSavedSession() )
        {
        iSignedIn = ETrue;
        _LIT( KIn, "Вход..." );
        SetStatusL( KIn );
        TRAPD( err, iSession->StartSavedL() );
        if ( err != KErrNone )
            {
            iSignedIn = EFalse;
            }
        }

    SetRect( aRect );
    SetFocusing( ETrue );
    SetFocus( ETrue );
    ActivateL();
    }

void CSymgramAppView::SetStatusL( const TDesC& aStatus )
    {
    HBufC* status = aStatus.AllocL();
    delete iStatus;
    iStatus = status;
    DrawDeferred();
    }

const CFont* CSymgramAppView::ChatFont() const
    {
    return iChatFont ? iChatFont : iTextFont;
    }

void CSymgramAppView::LoadEmojiL()
    {
    TFileName fn;
    _LIT( KFile, "\\resource\\apps\\Symgram_emoji.mbm" );
    fn.Copy( KFile );
    User::LeaveIfError( CompleteWithAppPath( fn ) );
    TInt i = 0;
    for ( i = 0; i < KEmojiCount; i++ )
        {
        CFbsBitmap* bmp = new ( ELeave ) CFbsBitmap;
        CleanupStack::PushL( bmp );
        CFbsBitmap* mask = new ( ELeave ) CFbsBitmap;
        CleanupStack::PushL( mask );
        User::LeaveIfError( bmp->Load( fn, i * 2 ) );
        User::LeaveIfError( mask->Load( fn, i * 2 + 1 ) );
        iEmojiBmp[ i ] = bmp;
        iEmojiMask[ i ] = mask;
        CleanupStack::Pop( 2 );
        }
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

TInt CSymgramAppView::TabHeight() const
    {
    const TInt h = iTextFont ? iTextFont->HeightInPixels() : 12;
    return h + 10;
    }

TRect CSymgramAppView::ListRect() const
    {
    TRect list( Rect() );
    list.iTl.iY += HeaderHeight();
    if ( iSignedIn && !iInChat && iPane == 0 )
        {
        list.iBr.iY -= TabHeight();
        }
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
    TInt dir = 0;
    if ( aKeyEvent.iScanCode == EStdKeyUpArrow )
        {
        dir = 1;
        }
    else if ( aKeyEvent.iScanCode == EStdKeyDownArrow )
        {
        dir = 2;
        }
    else if ( aKeyEvent.iScanCode == EStdKeyLeftArrow )
        {
        dir = 3;
        }
    else if ( aKeyEvent.iScanCode == EStdKeyRightArrow )
        {
        dir = 4;
        }
    else if ( aKeyEvent.iCode == EKeyUpArrow )
        {
        dir = 1;
        }
    else if ( aKeyEvent.iCode == EKeyDownArrow )
        {
        dir = 2;
        }
    else if ( aKeyEvent.iCode == EKeyLeftArrow )
        {
        dir = 3;
        }
    else if ( aKeyEvent.iCode == EKeyRightArrow )
        {
        dir = 4;
        }

    if ( aType == EEventKeyDown )
        {
        if ( dir )
            {
            iNavDown = dir;
            HandleArrowL( dir );
            return EKeyWasConsumed;
            }
        if ( aKeyEvent.iScanCode == EStdKeyDevice3 ||
             aKeyEvent.iScanCode == EStdKeyEnter )
            {
            return EKeyWasConsumed;
            }
        return EKeyWasNotConsumed;
        }

    if ( aType == EEventKeyUp )
        {
        iNavDown = 0;
        return EKeyWasNotConsumed;
        }

    if ( aType != EEventKey )
        {
        return EKeyWasNotConsumed;
        }

    if ( dir )
        {
        if ( iNavDown != dir )
            {
            HandleArrowL( dir );
            }
        return EKeyWasConsumed;
        }

    if ( !iSignedIn )
        {
        if ( aKeyEvent.iCode >= '0' && aKeyEvent.iCode <= '9' )
            {
            if ( iAwaitingPassword )
                {
                if ( iCloudPwd.Length() < iCloudPwd.MaxLength() )
                    {
                    iCloudPwd.Append( (TText)aKeyEvent.iCode );
                    DrawDeferred();
                    }
                }
            else if ( iAwaitingCode )
                {
                if ( iSmsCode.Length() < iSmsCode.MaxLength() )
                    {
                    iSmsCode.Append( (TText)aKeyEvent.iCode );
                    DrawDeferred();
                    }
                }
            else if ( iPhone.Length() < iPhone.MaxLength() )
                {
                iPhone.Append( (TText)aKeyEvent.iCode );
                iFocus = 1;
                DrawDeferred();
                }
            return EKeyWasConsumed;
            }

        if ( iAwaitingPassword &&
             aKeyEvent.iCode >= 32 && aKeyEvent.iCode < 0xF800 )
            {
            if ( iCloudPwd.Length() < iCloudPwd.MaxLength() )
                {
                iCloudPwd.Append( (TText)aKeyEvent.iCode );
                DrawDeferred();
                }
            return EKeyWasConsumed;
            }

        switch ( aKeyEvent.iCode )
            {
            case EKeyBackspace:
            case EKeyDelete:
                HandleBackspace();
                return EKeyWasConsumed;

            case EKeyDevice3:
            case EKeyEnter:
                if ( iPickingCountry )
                    {
                    iPickingCountry = EFalse;
                    iFocus = 1;
                    DrawDeferred();
                    }
                else if ( !iAwaitingCode && !iAwaitingPassword && iFocus == 0 )
                    {
                    QueryCountryL();
                    }
                else
                    {
                    NextL();
                    }
                return EKeyWasConsumed;

            default:
                return EKeyWasNotConsumed;
            }
        }

    if ( iSignedIn )
        {
        switch ( aKeyEvent.iCode )
            {
            case EKeyDevice3:
            case EKeyEnter:
                if ( iInChat )
                    {
                    if ( iPick == 1 )
                        {
                        SendPickedEmojiL();
                        }
                    else if ( iPick == 2 )
                        {
                        SendPickedPhotoL();
                        }
                    else if ( SelectedAttach() >= 0 )
                        {
                        OpenAttachmentL();
                        }
                    else
                        {
                        ComposeL();
                        }
                    }
                else if ( iPane == 2 )
                    {
                    if ( iSelected == 1 )
                        {
                        LogoutAskL();
                        }
                    else if ( iSelected == 2 )
                        {
                        iStore->ClearAll();
                        _LIT( KOk, "Кэш очищен" );
                        SetStatusL( KOk );
                        }
                    else if ( iSelected == 3 )
                        {
                        CheckUpdateL();
                        }
                    else if ( iSelected == 4 )
                        {
                        TBuf<24> about;
                        about.Copy( KSymgramAppName );
                        about.Append( ' ' );
                        about.Append( KSymgramVersionName );
                        SetStatusL( about );
                        }
                    }
                else if ( iPane == 1 )
                    {
                    OpenContact();
                    }
                else
                    {
                    OpenSelectedChat();
                    }
                return EKeyWasConsumed;
            case EKeyBackspace:
            case EKeyDelete:
                if ( iInChat )
                    {
                    if ( iPick != 0 )
                        {
                        ClosePicker();
                        }
                    else
                        {
                        CloseChat();
                        }
                    return EKeyWasConsumed;
                    }
                if ( iPane != 0 )
                    {
                    ClosePane();
                    return EKeyWasConsumed;
                    }
                break;
            default:
                break;
            }
        }

    return EKeyWasNotConsumed;
    }

void CSymgramAppView::HandleArrowL( TInt aDir )
    {
    if ( iInChat )
        {
        if ( iPick == 1 )
            {
            const TInt cols = 6;
            if ( aDir == 1 && iPickSel >= cols )
                {
                iPickSel -= cols;
                }
            else if ( aDir == 2 && iPickSel + cols < KEmojiCount )
                {
                iPickSel += cols;
                }
            else if ( aDir == 3 && iPickSel > 0 )
                {
                iPickSel--;
                }
            else if ( aDir == 4 && iPickSel + 1 < KEmojiCount )
                {
                iPickSel++;
                }
            DrawDeferred();
            return;
            }
        if ( iPick == 2 )
            {
            const TInt n = iFiles ? iFiles->Count() : 0;
            if ( aDir == 1 && iPickSel > 0 )
                {
                iPickSel--;
                }
            else if ( aDir == 2 && iPickSel + 1 < n )
                {
                iPickSel++;
                }
            else if ( aDir == 3 )
                {
                ClosePicker();
                return;
                }
            DrawDeferred();
            return;
            }
        if ( aDir == 3 )
            {
            CloseChat();
            }
        else if ( aDir == 1 && iMsgSel > 0 )
            {
            iMsgSel--;
            EnsureMsgVisible();
            DrawDeferred();
            }
        else if ( aDir == 2 && iMsgSel + 1 < iMsgs.Count() )
            {
            iMsgSel++;
            EnsureMsgVisible();
            DrawDeferred();
            }
        return;
        }
    if ( !iSignedIn )
        {
        if ( iPickingCountry )
            {
            const TInt n = iCountries ? iCountries->Count() : KCountryCount;
            if ( n <= 0 )
                {
                return;
                }
            if ( aDir == 1 || aDir == 3 )
                {
                iCountry = ( iCountry + n - 1 ) % n;
                DrawDeferred();
                }
            else if ( aDir == 2 || aDir == 4 )
                {
                iCountry = ( iCountry + 1 ) % n;
                DrawDeferred();
                }
            return;
            }
        if ( iAwaitingCode || iAwaitingPassword )
            {
            return;
            }
        if ( aDir == 1 || aDir == 3 )
            {
            iFocus = 0;
            DrawDeferred();
            }
        else if ( aDir == 2 || aDir == 4 )
            {
            iFocus = 1;
            DrawDeferred();
            }
        return;
        }

    if ( iPane != 0 )
        {
        if ( aDir == 3 )
            {
            ClosePane();
            return;
            }
        }
    else if ( aDir == 3 )
        {
        iTab = ( iTab + 2 ) % 3;
        iSelected = 0;
        iTopRow = 0;
        DrawDeferred();
        return;
        }
    else if ( aDir == 4 )
        {
        iTab = ( iTab + 1 ) % 3;
        iSelected = 0;
        iTopRow = 0;
        DrawDeferred();
        return;
        }

    const TInt n = CurrentCount();
    if ( n <= 0 )
        {
        return;
        }
    if ( aDir == 1 && iSelected > 0 )
        {
        iSelected--;
        EnsureSelectionVisible();
        DrawDeferred();
        }
    else if ( aDir == 2 && iSelected < n - 1 )
        {
        iSelected++;
        EnsureSelectionVisible();
        DrawDeferred();
        }
    }

void CSymgramAppView::HandleBackspace()
    {
    if ( iPickingCountry )
        {
        iPickingCountry = EFalse;
        DrawDeferred();
        return;
        }
    if ( iAwaitingPassword )
        {
        if ( iCloudPwd.Length() > 0 )
            {
            iCloudPwd.SetLength( iCloudPwd.Length() - 1 );
            DrawDeferred();
            }
        return;
        }
    if ( iAwaitingCode )
        {
        if ( iSmsCode.Length() > 0 )
            {
            iSmsCode.SetLength( iSmsCode.Length() - 1 );
            DrawDeferred();
            }
        }
    else if ( iPhone.Length() > 0 )
        {
        iPhone.SetLength( iPhone.Length() - 1 );
        DrawDeferred();
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
    if ( iSignedIn && iInChat )
        {
        if ( iPick == 1 )
            {
            DrawEmojiPicker( gc, list );
            }
        else if ( iPick == 2 )
            {
            DrawFilePicker( gc, list );
            }
        else
            {
            DrawChat( gc, list );
            }
        return;
        }
    if ( !iSignedIn )
        {
        if ( iPickingCountry )
            {
            DrawCountryList( gc, list );
            }
        else
            {
            DrawSignIn( gc, list );
            }
        return;
        }

    if ( iPane == 2 )
        {
        DrawSettings( gc, list );
        }
    else if ( iPane == 1 )
        {
        if ( iContacts.Count() == 0 )
            {
            DrawEmptyState( gc, list );
            }
        else
            {
            const TInt rowH = RowHeight();
            const TInt rows = VisibleRows();
            TInt i = 0;
            for ( i = 0; i < rows; i++ )
                {
                const TInt index = iTopRow + i;
                if ( index >= iContacts.Count() )
                    {
                    break;
                    }
                const TRect row( list.iTl.iX,
                                 list.iTl.iY + i * rowH,
                                 list.iBr.iX,
                                 list.iTl.iY + ( i + 1 ) * rowH );
                DrawContactRow( gc, row, index );
                }
            }
        }
    else if ( ChatCount() == 0 )
        {
        DrawEmptyState( gc, list );
        }
    else
        {
        const TInt rowH = RowHeight();
        const TInt rows = VisibleRows();
        TInt i = 0;
        for ( i = 0; i < rows; i++ )
            {
            const TInt vis = iTopRow + i;
            const TInt index = ChatIndex( vis );
            if ( index < 0 )
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

    if ( iPane == 0 )
        {
        TRect tabs( rect );
        tabs.iTl.iY = rect.iBr.iY - TabHeight();
        DrawTabs( gc, tabs );
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
    if ( iSignedIn && iInChat )
        {
        TBuf<40> title;
        if ( iPick == 1 )
            {
            _LIT( KEm, "Эмодзи" );
            title.Copy( KEm );
            }
        else if ( iPick == 2 )
            {
            _LIT( KPh, "Фото" );
            title.Copy( KPh );
            }
        else
            {
            const TInt idx = FindChat( iOpenId );
            if ( idx >= 0 )
                {
                title.Copy( iChats[ idx ].iName );
                }
            }
        aGc.DrawText( title, left, baseline, CGraphicsContext::ELeft );
        }
    else if ( iSignedIn && iPane == 1 )
        {
        _LIT( KCtc, "Контакты" );
        aGc.DrawText( KCtc, left, baseline, CGraphicsContext::ELeft );
        }
    else if ( iSignedIn && iPane == 2 )
        {
        _LIT( KSet, "Настройки" );
        aGc.DrawText( KSet, left, baseline, CGraphicsContext::ELeft );
        }
    else if ( iSignedIn )
        {
        if ( iTab == 1 )
            {
            _LIT( KGrp, "Группы" );
            aGc.DrawText( KGrp, left, baseline, CGraphicsContext::ELeft );
            }
        else if ( iTab == 2 )
            {
            _LIT( KChn, "Каналы" );
            aGc.DrawText( KChn, left, baseline, CGraphicsContext::ELeft );
            }
        else
            {
            _LIT( KChats, "Чаты" );
            aGc.DrawText( KChats, left, baseline, CGraphicsContext::ELeft );
            }
        }
    else
        {
        aGc.DrawText( KSymgramAppName(), left, baseline, CGraphicsContext::ELeft );
        }

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
    aGc.SetBrushColor( aIndex == ChatIndex( iSelected ) ? Highlight() : Paper() );
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
    FoldVisible( name );
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
    FoldVisible( preview );
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

void CSymgramAppView::DrawSignInField( CWindowGc& aGc, const TRect& aRow,
                                       TBool aOn, const TDesC& aLeft,
                                       const TDesC& aRight ) const
    {
    aGc.SetPenStyle( CGraphicsContext::ENullPen );
    aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    aGc.SetBrushColor( aOn ? Brand() : Rule() );
    aGc.DrawRect( aRow );

    if ( !aOn )
        {
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( Muted() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
        aGc.DrawRect( aRow );
        }

    aGc.UseFont( iNameFont );
    aGc.SetPenStyle( CGraphicsContext::ESolidPen );
    aGc.SetPenColor( aOn ? Paper() : Ink() );
    aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
    TRect cleft( aRow );
    cleft.iTl.iX += 10;
    const TInt mid = ( aRow.Height() + iNameFont->AscentInPixels() ) / 2;
    aGc.DrawText( aLeft, cleft, mid, CGraphicsContext::ELeft );
    TRect cright( aRow );
    cright.iBr.iX -= 8;
    aGc.DrawText( aRight, cright, mid, CGraphicsContext::ERight );
    aGc.DiscardFont();
    }

void CSymgramAppView::DrawCountryList( CWindowGc& aGc, const TRect& aRect ) const
    {
    const TInt n = iCountries ? iCountries->Count() : 0;
    const TInt rowH = RowHeight();
    const TInt rows = aRect.Height() / rowH;
    TInt top = iCountry - rows / 2;
    if ( top < 0 )
        {
        top = 0;
        }
    if ( n > rows && top > n - rows )
        {
        top = n - rows;
        }
    TInt i = 0;
    for ( i = 0; i < rows; i++ )
        {
        const TInt index = top + i;
        if ( index >= n )
            {
            break;
            }
        const TRect row( aRect.iTl.iX,
                         aRect.iTl.iY + i * rowH,
                         aRect.iBr.iX,
                         aRect.iTl.iY + ( i + 1 ) * rowH );
        TBuf<32> name;
        name.Copy( ( *iCountries )[ index ] );
        TBuf<12> plus;
        plus.Append( '+' );
        TBuf<8> code;
        code.Num( ( index < KCountryCount ) ? KCallingCode[ index ] : 0 );
        plus.Append( code );
        DrawSignInField( aGc, row, index == iCountry, name, plus );
        }
    }

void CSymgramAppView::DrawSignIn( CWindowGc& aGc, const TRect& aRect ) const
    {
    if ( !iNameFont || !iTextFont )
        {
        return;
        }

    const TInt nameH = iNameFont->HeightInPixels();
    const TInt textH = iTextFont->HeightInPixels();
    TInt y = aRect.iTl.iY + 12;

    aGc.SetPenStyle( CGraphicsContext::ESolidPen );
    aGc.SetBrushStyle( CGraphicsContext::ENullBrush );

    if ( iAwaitingPassword )
        {
        const HBufC* title = iPasswordPrompt;
        if ( title )
            {
            aGc.UseFont( iNameFont );
            aGc.SetPenColor( Ink() );
            aGc.DrawText( *title,
                          TRect( aRect.iTl.iX + 8, y, aRect.iBr.iX - 8, y + nameH ),
                          iNameFont->AscentInPixels(), CGraphicsContext::ELeft );
            aGc.DiscardFont();
            }
        y += nameH + 10;
        const TInt rowH = nameH + 14;
        TBuf<40> shown;
        const TInt keep = shown.MaxLength() - 1;
        if ( iCloudPwd.Length() > keep )
            {
            shown.Copy( iCloudPwd.Right( keep ) );
            }
        else
            {
            shown.Copy( iCloudPwd );
            }
        shown.Append( '_' );
        _LIT( KEmptyRight, "" );
        TRect pwdRow( aRect.iTl.iX + 6, y, aRect.iBr.iX - 6, y + rowH );
        ClipText( shown, *iNameFont, pwdRow.Width() - 20 );
        DrawSignInField( aGc, pwdRow, ETrue, shown, KEmptyRight );
        y += rowH + 10;
        if ( iPasswordHint )
            {
            aGc.UseFont( iTextFont );
            aGc.SetPenColor( Muted() );
            aGc.DrawText( *iPasswordHint,
                          TRect( aRect.iTl.iX + 8, y, aRect.iBr.iX - 8, y + textH ),
                          iTextFont->AscentInPixels(), CGraphicsContext::ELeft );
            aGc.DiscardFont();
            y += textH + 4;
            }
        if ( iPwdHint.Length() > 0 )
            {
            aGc.UseFont( iTextFont );
            aGc.SetPenColor( Muted() );
            aGc.DrawText( iPwdHint,
                          TRect( aRect.iTl.iX + 8, y, aRect.iBr.iX - 8, y + textH * 2 ),
                          iTextFont->AscentInPixels(), CGraphicsContext::ELeft );
            aGc.DiscardFont();
            }
        return;
        }

    const HBufC* title = iAwaitingCode ? iCodeTitle : iSignInTitle;
    if ( title )
        {
        aGc.UseFont( iNameFont );
        aGc.SetPenColor( Ink() );
        aGc.DrawText( *title,
                      TRect( aRect.iTl.iX + 8, y, aRect.iBr.iX - 8, y + nameH ),
                      iNameFont->AscentInPixels(), CGraphicsContext::ELeft );
        aGc.DiscardFont();
        }
    y += nameH + 10;

    const TInt rowH = nameH + 14;

    if ( !iAwaitingCode )
        {
        if ( iFieldCountry )
            {
            aGc.UseFont( iTextFont );
            aGc.SetPenColor( iFocus == 0 ? Brand() : Muted() );
            aGc.DrawText( *iFieldCountry,
                          TRect( aRect.iTl.iX + 8, y, aRect.iBr.iX - 8, y + textH ),
                          iTextFont->AscentInPixels(), CGraphicsContext::ELeft );
            aGc.DiscardFont();
            y += textH + 2;
            }

        TRect countryRow( aRect.iTl.iX + 6, y, aRect.iBr.iX - 6, y + rowH );
        TBuf<32> country;
        CountryName( country );
        TBuf<16> plus;
        plus.Append( '+' );
        TBuf<12> code;
        code.Num( CallingCode() );
        plus.Append( code );
        _LIT( KOpen, ">" );
        if ( plus.Length() + KOpen().Length() <= plus.MaxLength() )
            {
            plus.Append( KOpen );
            }
        DrawSignInField( aGc, countryRow, iFocus == 0, country, plus );
        y += rowH + 8;
        }

    if ( iFieldPhone && !iAwaitingCode )
        {
        aGc.UseFont( iTextFont );
        aGc.SetPenColor( iFocus == 1 ? Brand() : Muted() );
        aGc.DrawText( *iFieldPhone,
                      TRect( aRect.iTl.iX + 8, y, aRect.iBr.iX - 8, y + textH ),
                      iTextFont->AscentInPixels(), CGraphicsContext::ELeft );
        aGc.DiscardFont();
        y += textH + 2;
        }

    TRect phoneRow( aRect.iTl.iX + 6, y, aRect.iBr.iX - 6, y + rowH );
    TBuf<28> shown;
    if ( iAwaitingCode )
        {
        shown.Copy( iSmsCode );
        }
    else
        {
        TBuf<12> code;
        code.Num( CallingCode() );
        shown.Append( '+' );
        shown.Append( code );
        shown.Append( ' ' );
        shown.Append( iPhone );
        }
    if ( ( iAwaitingCode || iFocus == 1 ) &&
         shown.Length() < shown.MaxLength() )
        {
        shown.Append( '_' );
        }
    _LIT( KEmptyRight, "" );
    DrawSignInField( aGc, phoneRow, ( iAwaitingCode || iFocus == 1 ),
                     shown, KEmptyRight );
    y += rowH + 10;

    const HBufC* hint = iAwaitingCode ? iCodeHint : iSignInHint;
    if ( hint )
        {
        aGc.UseFont( iTextFont );
        aGc.SetPenColor( Muted() );
        aGc.DrawText( *hint,
                      TRect( aRect.iTl.iX + 8, y, aRect.iBr.iX - 8, y + textH * 2 ),
                      iTextFont->AscentInPixels(), CGraphicsContext::ELeft );
        aGc.DiscardFont();
        }
    }

void CSymgramAppView::CountryName( TDes& aOut ) const
    {
    aOut.Zero();
    if ( iCountries && iCountry >= 0 && iCountry < iCountries->Count() )
        {
        aOut.Copy( ( *iCountries )[ iCountry ] );
        }
    }

TInt CSymgramAppView::CallingCode() const
    {
    if ( iCountry >= 0 && iCountry < KCountryCount )
        {
        return KCallingCode[ iCountry ];
        }
    return 7;
    }

void CSymgramAppView::CycleCountry( TInt /*aDelta*/ )
    {
    QueryCountryL();
    }

void CSymgramAppView::QueryCountryL()
    {
    if ( iSignedIn || iAwaitingCode || iAwaitingPassword )
        {
        return;
        }
    iPickingCountry = ETrue;
    DrawDeferred();
    }

TBool CSymgramAppView::ShowNextCommand() const
    {
    return !iSignedIn;
    }

TBool CSymgramAppView::ShowCountryCommand() const
    {
    return !iSignedIn && !iAwaitingCode && !iAwaitingPassword && !iPickingCountry;
    }

TBool CSymgramAppView::ShowRefreshCommand() const
    {
    return iSignedIn && !iInChat;
    }

TBool CSymgramAppView::ShowListCommand() const
    {
    return iSignedIn && !iInChat;
    }

TBool CSymgramAppView::ShowWriteCommand() const
    {
    return iSignedIn && iInChat;
    }

TBool CSymgramAppView::ShowAttachCommand() const
    {
    return iSignedIn && iInChat && iPick == 0 && SelectedAttach() >= 0;
    }

TBool CSymgramAppView::ShowLogoutCommand() const
    {
    return iSignedIn;
    }

void CSymgramAppView::NextL()
    {
    if ( iPickingCountry )
        {
        iPickingCountry = EFalse;
        iFocus = 1;
        DrawDeferred();
        return;
        }
    if ( iSignedIn || !iSession || iSession->IsBusy() )
        {
        return;
        }
    if ( iAwaitingPassword )
        {
        EditPasswordL();
        return;
        }
    if ( iAwaitingCode )
        {
        if ( iSmsCode.Length() < 4 )
            {
            return;
            }
        TBuf8<16> code;
        TInt i = 0;
        for ( i = 0; i < iSmsCode.Length(); i++ )
            {
            code.Append( (TUint8)iSmsCode[ i ] );
            }
        iSession->SubmitCodeL( code );
        return;
        }
    if ( iPhone.Length() < 6 )
        {
        return;
        }
    TBuf8<24> phone;
    phone.Append( '+' );
    TBuf<8> cc;
    cc.Num( CallingCode() );
    TInt i = 0;
    for ( i = 0; i < cc.Length(); i++ )
        {
        phone.Append( (TUint8)cc[ i ] );
        }
    for ( i = 0; i < iPhone.Length(); i++ )
        {
        phone.Append( (TUint8)iPhone[ i ] );
        }
    iSession->ConnectL( phone );
    }

void CSymgramAppView::EditPasswordL()
    {
    if ( !iSession || iSession->IsBusy() )
        {
        return;
        }

    TBuf<128> pwd;
    pwd.Copy( iCloudPwd );
    CAknTextQueryDialog* dlg = CAknTextQueryDialog::NewL( pwd );
    dlg->SetMaxLength( iCloudPwd.MaxLength() );
    const TBool ok = dlg->ExecuteLD( R_SYMGRAM_PASSWORD_QUERY );
    if ( ok )
        {
        iCloudPwd.Copy( pwd );
        DrawDeferred();
        QueryPasswordL();
        }
    pwd.FillZ();
    }

void CSymgramAppView::QueryPasswordL()
    {
    if ( !iSession || iSession->IsBusy() )
        {
        return;
        }
    if ( iCloudPwd.Length() < 1 )
        {
        _LIT( KEmpty, "Введите пароль" );
        SetStatusL( KEmpty );
        return;
        }
    TBuf8<192> utf;
    ToUtf8( iCloudPwd, utf );
    TInt leaveErr = KErrNone;
    TInt sendErr = KErrNone;
    TRAP( leaveErr, sendErr = iSession->SubmitPasswordL( utf ) );
    utf.FillZ();
    if ( leaveErr != KErrNone )
        {
        sendErr = leaveErr;
        }
    if ( sendErr != KErrNone )
        {
        TBuf<40> text;
        _LIT( KFail, "Пароль не принят " );
        text.Copy( KFail );
        text.AppendNum( sendErr );
        SetStatusL( text );
        }
    }

void CSymgramAppView::SessionStatusL( const TDesC& aText )
    {
    SetStatusL( aText );
    }

void CSymgramAppView::SessionFailedL( TInt aError )
    {
    TBuf<40> text;
    if ( aError == KErrDisconnected || aError == KErrEof ||
         aError == KErrSessionClosed )
        {
        _LIT( KDrop, "Связь оборвалась" );
        text.Copy( KDrop );
        }
    else
        {
        _LIT( KErr, "Ошибка " );
        text.Copy( KErr );
        text.AppendNum( aError );
        }
    SetStatusL( text );

    CAknErrorNote* note = new ( ELeave ) CAknErrorNote( ETrue );
    note->ExecuteLD( text );
    }

void CSymgramAppView::SessionErrorL( const TDesC& aText )
    {
    // A modal note from inside CActive::RunL panics on S60. The header
    // status line is enough; login errors remain visible on the sign-in screen.
    SetStatusL( aText );
    }

void CSymgramAppView::SessionCodeSentL()
    {
    iAwaitingCode = ETrue;
    iSmsCode.Zero();
    _LIT( KWait, "Введите код" );
    SetStatusL( KWait );
    }

void CSymgramAppView::SessionPasswordNeededL( const TDesC& aHint )
    {
    iSignedIn = EFalse;
    iAwaitingCode = EFalse;
    iAwaitingPassword = ETrue;
    iCloudPwd.Zero();
    iPwdHint.Copy( aHint.Left( iPwdHint.MaxLength() ) );
    _LIT( KWait, "Облачный пароль — OK" );
    SetStatusL( KWait );
    DrawDeferred();
    }

void CSymgramAppView::SessionSignedInL()
    {
    iSignedIn = ETrue;
    iAwaitingCode = EFalse;
    iAwaitingPassword = EFalse;
    iCloudPwd.FillZ();
    _LIT( KOn, "Загрузка чатов..." );
    SetStatusL( KOn );
    }

void CSymgramAppView::SessionLoggedOutL()
    {
    iSignedIn = EFalse;
    iAwaitingCode = EFalse;
    iAwaitingPassword = EFalse;
    iInChat = EFalse;
    iOpenId = 0;
    ClearMessages();
    iChats.Reset();
    iContacts.Reset();
    iSelected = 0;
    iTopRow = 0;
    iTab = 0;
    iPane = 0;
    if ( iStore )
        {
        iStore->ClearAll();
        }
    _LIT( KOut, "Войдите снова" );
    SetStatusL( KOut );
    DrawDeferred();
    }

void CSymgramAppView::SessionAddChatL( const TSymgramChat& aChat )
    {
    TInt i = 0;
    for ( i = 0; i < iChats.Count(); i++ )
        {
        if ( iChats[ i ].iId == aChat.iId )
            {
            if ( aChat.iPreview.Length() > 0 )
                {
                iChats[ i ].iPreview.Copy( aChat.iPreview );
                }
            if ( aChat.iTime.Length() > 0 )
                {
                iChats[ i ].iTime.Copy( aChat.iTime );
                }
            if ( aChat.iName.Length() > 0 )
                {
                _LIT( KDlg, "Диалог" );
                if ( aChat.iName != KDlg )
                    {
                    iChats[ i ].iName.Copy( aChat.iName );
                    }
                else if ( aChat.iPeerKind >= 2 &&
                          iChats[ i ].iPeerKind <= 1 )
                    {
                    iChats[ i ].iName.Copy( KDlg );
                    }
                }
            iChats[ i ].iUnread = aChat.iUnread;
            iChats[ i ].iDate = aChat.iDate;
            if ( aChat.iHash != 0 )
                {
                iChats[ i ].iHash = aChat.iHash;
                }
            if ( aChat.iPeerKind >= 2 )
                {
                iChats[ i ].iPeerKind = aChat.iPeerKind;
                }
            else if ( aChat.iPeerKind != 0 &&
                 !( iChats[ i ].iPeerKind >= 2 && aChat.iPeerKind == 1 ) )
                {
                iChats[ i ].iPeerKind = aChat.iPeerKind;
                }
            DrawDeferred();
            return;
            }
        }
    User::LeaveIfError( iChats.Append( aChat ) );
    DrawDeferred();
    }

void CSymgramAppView::SessionChatsReadyL()
    {
    if ( iStore )
        {
        iStore->SaveChats( iChats );
        }
    DrawDeferred();
    }

void CSymgramAppView::SessionClearContactsL()
    {
    iContacts.Reset();
    }

void CSymgramAppView::SessionAddContactL( const TSymgramContact& aContact )
    {
    TInt i = 0;
    for ( i = 0; i < iContacts.Count(); i++ )
        {
        if ( iContacts[ i ].iId == aContact.iId )
            {
            iContacts[ i ] = aContact;
            return;
            }
        }
    User::LeaveIfError( iContacts.Append( aContact ) );
    }

void CSymgramAppView::SessionContactsReadyL()
    {
    if ( iStore )
        {
        iStore->SaveContacts( iContacts );
        }
    DrawDeferred();
    }

void CSymgramAppView::SessionBeginMessagesL( TInt64 aPeer )
    {
    if ( aPeer != iOpenId )
        {
        return;
        }
    ClearMessages();
    }

void CSymgramAppView::SessionAddMessageL( const TSymgramMsg& aMsg )
    {
    TSymgramMsg m = aMsg;
    m.iBmp = NULL;
    User::LeaveIfError( iMsgs.Append( m ) );
    }

void CSymgramAppView::SessionMessagesReadyL( TInt64 aPeer )
    {
    if ( aPeer != iOpenId )
        {
        return;
        }
    if ( iStore )
        {
        iStore->SaveMessages( aPeer, iMsgs );
        }
    iTopRow = 0;
    iThumbAt = 0;
    iMsgSel = iMsgs.Count() > 0 ? iMsgs.Count() - 1 : 0;
    EnsureMsgVisible();
    RequestNextThumb();
    DrawDeferred();
    }

void CSymgramAppView::SessionThumbL( TInt aMsgId, const TDesC8& aJpeg )
    {
    if ( !iJpeg || aJpeg.Length() < 16 )
        {
        RequestNextThumb();
        return;
        }
    TRAPD( err, iJpeg->DecodeL( aMsgId, aJpeg ) );
    if ( err != KErrNone )
        {
        RequestNextThumb();
        }
    }

void CSymgramAppView::SessionSentL( TInt aId, TInt aDate )
    {
    if ( iMsgs.Count() > 0 )
        {
        TSymgramMsg& last = iMsgs[ iMsgs.Count() - 1 ];
        if ( last.iOut && last.iId == 0 && aId != 0 )
            {
            last.iId = aId;
            last.iDate = aDate;
            }
        }
    if ( iStore && iOpenId != 0 )
        {
        iStore->SaveMessages( iOpenId, iMsgs );
        }
    DrawDeferred();
    }

void CSymgramAppView::SessionFileSavedL( const TDesC& aPath, TBool aOpen )
    {
    _LIT( KOk, "Сохранено" );
    SetStatusL( KOk );
    if ( aOpen )
        {
        TRAP_IGNORE( OpenSystemL( aPath ) );
        }
    RequestNextThumb();
    }

void CSymgramAppView::JpegReady( CFbsBitmap* aBmp, TInt aMsgId )
    {
    if ( aBmp )
        {
        TInt i = 0;
        for ( i = 0; i < iMsgs.Count(); i++ )
            {
            if ( iMsgs[ i ].iId == aMsgId )
                {
                delete iMsgs[ i ].iBmp;
                iMsgs[ i ].iBmp = aBmp;
                aBmp = NULL;
                break;
                }
            }
        }
    delete aBmp;
    DrawDeferred();
    RequestNextThumb();
    }

void CSymgramAppView::RequestNextThumb()
    {
    if ( !iSession || !iInChat )
        {
        return;
        }
    while ( iThumbAt < iMsgs.Count() )
        {
        const TSymgramMsg& m = iMsgs[ iThumbAt++ ];
        if ( m.iBmp == NULL && m.iFileId != 0 &&
             ( m.iKind == ESgPhoto || m.iKind == ESgSticker ||
               m.iKind == ESgVideo ) )
            {
            TRAP_IGNORE( iSession->GetFileL( m.iId, m.iFileId, m.iFileHash,
                                             m.iFileRef, m.iPhotoLoc ) );
            return;
            }
        }
    }

void CSymgramAppView::OpenSelectedChat()
    {
    OpenChatAt( ChatIndex( iSelected ) );
    }

void CSymgramAppView::OpenChatAt( TInt aIndex )
    {
    if ( !iSignedIn || aIndex < 0 || aIndex >= iChats.Count() )
        {
        return;
        }
    const TSymgramChat& chat = iChats[ aIndex ];
    iOpenId = chat.iId;
    iInChat = ETrue;
    ClearMessages();
    if ( iStore )
        {
        TRAP_IGNORE( iStore->LoadMessagesL( iOpenId, iMsgs ) );
        }
    iTopRow = 0;
    iMsgSel = iMsgs.Count() > 0 ? iMsgs.Count() - 1 : 0;
    DrawDeferred();
    if ( iSession )
        {
        TRAP_IGNORE( iSession->GetHistoryL( chat.iId, chat.iPeerKind,
                                            chat.iHash ) );
        }
    }

void CSymgramAppView::OpenContact()
    {
    if ( iSelected < 0 || iSelected >= iContacts.Count() )
        {
        return;
        }
    const TSymgramContact& c = iContacts[ iSelected ];
    TInt idx = FindChat( c.iId );
    if ( idx < 0 )
        {
        TSymgramChat chat;
        chat.iId = c.iId;
        chat.iHash = c.iHash;
        chat.iPeerKind = 1;
        chat.iUnread = 0;
        chat.iDate = 0;
        chat.iName.Copy( c.iName );
        iChats.Append( chat );
        idx = iChats.Count() - 1;
        }
    iPane = 0;
    iTab = 0;
    TInt vis = 0;
    TInt i = 0;
    iSelected = 0;
    for ( i = 0; i < iChats.Count(); i++ )
        {
        if ( !ChatInTab( iChats[ i ] ) )
            {
            continue;
            }
        if ( i == idx )
            {
            iSelected = vis;
            break;
            }
        vis++;
        }
    OpenChatAt( idx );
    }

void CSymgramAppView::ClosePane()
    {
    iPane = 0;
    iSelected = 0;
    iTopRow = 0;
    DrawDeferred();
    }

void CSymgramAppView::CloseChat()
    {
    iPick = 0;
    iInChat = EFalse;
    iOpenId = 0;
    ClearMessages();
    DrawDeferred();
    }

void CSymgramAppView::DrawChat( CWindowGc& aGc, const TRect& aRect ) const
    {
    const CFont* font = ChatFont();
    if ( !font )
        {
        return;
        }
    const TInt line = font->HeightInPixels() + 6;
    TInt y = aRect.iBr.iY - line - 4;
    TInt i = iMsgs.Count() - 1 - iTopRow;
    for ( ; i >= 0 && y > aRect.iTl.iY + 4; i-- )
        {
        const TSymgramMsg& m = iMsgs[ i ];
        TInt h = line + 8;
        if ( m.iBmp )
            {
            h += m.iBmp->SizeInPixels().iHeight + 4;
            }
        y -= h;
        TRect box( aRect.iTl.iX + 6, y, aRect.iBr.iX - 6, y + h - 2 );
        if ( m.iOut )
            {
            box.iTl.iX = aRect.iTl.iX + aRect.Width() / 4;
            }
        else
            {
            box.iBr.iX = aRect.iBr.iX - aRect.Width() / 5;
            }
        DrawBubble( aGc, box, m, i == iMsgSel );
        }
    aGc.UseFont( iTextFont );
    aGc.SetPenColor( Muted() );
    aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
    _LIT( KHint, "Центр — открыть или написать. C — назад." );
    TRect hint( aRect.iTl.iX + 6, aRect.iBr.iY - line,
                aRect.iBr.iX - 6, aRect.iBr.iY - 2 );
    aGc.DrawText( KHint, hint, iTextFont->AscentInPixels(),
                  CGraphicsContext::ELeft );
    aGc.DiscardFont();
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

void CSymgramAppView::DrawTabs( CWindowGc& aGc, const TRect& aRect ) const
    {
    aGc.SetPenStyle( CGraphicsContext::ENullPen );
    aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    aGc.SetBrushColor( Rule() );
    aGc.DrawRect( aRect );
    if ( !iTextFont )
        {
        return;
        }
    const TInt w = aRect.Width() / 3;
    _LIT( K0, "Чаты" );
    _LIT( K1, "Группы" );
    _LIT( K2, "Каналы" );
    TInt t = 0;
    for ( t = 0; t < 3; t++ )
        {
        TRect cell( aRect.iTl.iX + t * w, aRect.iTl.iY,
                    aRect.iTl.iX + ( t + 1 ) * w, aRect.iBr.iY );
        if ( t == iTab )
            {
            aGc.SetBrushColor( Brand() );
            aGc.DrawRect( cell );
            }
        aGc.UseFont( iTextFont );
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( t == iTab ? Paper() : Ink() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
        const TInt base =
            ( cell.Height() + iTextFont->AscentInPixels() ) / 2;
        if ( t == 0 )
            {
            aGc.DrawText( K0, cell, base, CGraphicsContext::ECenter );
            }
        else if ( t == 1 )
            {
            aGc.DrawText( K1, cell, base, CGraphicsContext::ECenter );
            }
        else
            {
            aGc.DrawText( K2, cell, base, CGraphicsContext::ECenter );
            }
        aGc.DiscardFont();
        }
    }

void CSymgramAppView::DrawSettings( CWindowGc& aGc, const TRect& aRect ) const
    {
    TBuf<40> phone;
    PhoneText( phone );
    _LIT( KAcc, "Аккаунт" );
    _LIT( KOut, "Выйти" );
    _LIT( KCache, "Очистить кэш" );
    _LIT( KUpd, "Обновление" );
    _LIT( KAbout, "О программе" );
    const TInt rowH = RowHeight();
    TInt i = 0;
    for ( i = 0; i < 5; i++ )
        {
        TRect row( aRect.iTl.iX, aRect.iTl.iY + i * rowH,
                   aRect.iBr.iX, aRect.iTl.iY + ( i + 1 ) * rowH );
        aGc.SetPenStyle( CGraphicsContext::ENullPen );
        aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
        aGc.SetBrushColor( i == iSelected ? Highlight() : Paper() );
        aGc.DrawRect( row );
        if ( !iNameFont )
            {
            continue;
            }
        aGc.UseFont( iNameFont );
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( Ink() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
        TRect nameBox( row.iTl.iX + 8, row.iTl.iY + 4,
                       row.iBr.iX - 8, row.iTl.iY + 4 + iNameFont->HeightInPixels() );
        if ( i == 0 )
            {
            aGc.DrawText( KAcc, nameBox, iNameFont->AscentInPixels(),
                          CGraphicsContext::ELeft );
            }
        else if ( i == 1 )
            {
            aGc.DrawText( KOut, nameBox, iNameFont->AscentInPixels(),
                          CGraphicsContext::ELeft );
            }
        else if ( i == 2 )
            {
            aGc.DrawText( KCache, nameBox, iNameFont->AscentInPixels(),
                          CGraphicsContext::ELeft );
            }
        else if ( i == 3 )
            {
            aGc.DrawText( KUpd, nameBox, iNameFont->AscentInPixels(),
                          CGraphicsContext::ELeft );
            }
        else
            {
            aGc.DrawText( KAbout, nameBox, iNameFont->AscentInPixels(),
                          CGraphicsContext::ELeft );
            }
        aGc.DiscardFont();
        if ( i == 0 && iTextFont && phone.Length() > 0 )
            {
            aGc.UseFont( iTextFont );
            aGc.SetPenColor( Muted() );
            TRect d( row.iTl.iX + 8,
                     nameBox.iBr.iY + 2,
                     row.iBr.iX - 8, row.iBr.iY - 2 );
            aGc.DrawText( phone, d, iTextFont->AscentInPixels(),
                          CGraphicsContext::ELeft );
            aGc.DiscardFont();
            }
        else if ( i == 3 && iTextFont )
            {
            aGc.UseFont( iTextFont );
            aGc.SetPenColor( Muted() );
            TRect d( row.iTl.iX + 8,
                     nameBox.iBr.iY + 2,
                     row.iBr.iX - 8, row.iBr.iY - 2 );
            if ( iUpdate && iUpdate->RemoteTag().Length() > 0 )
                {
                aGc.DrawText( iUpdate->RemoteTag(), d,
                              iTextFont->AscentInPixels(),
                              CGraphicsContext::ELeft );
                }
            else
                {
                aGc.DrawText( KSymgramVersionName, d,
                              iTextFont->AscentInPixels(),
                              CGraphicsContext::ELeft );
                }
            aGc.DiscardFont();
            }
        }
    }

void CSymgramAppView::DrawContactRow( CWindowGc& aGc, const TRect& aRect,
                                      TInt aIndex ) const
    {
    const TSymgramContact& c = iContacts[ aIndex ];
    aGc.SetPenStyle( CGraphicsContext::ENullPen );
    aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    aGc.SetBrushColor( aIndex == iSelected ? Highlight() : Paper() );
    aGc.DrawRect( aRect );
    const TInt diameter = aRect.Height() - 12;
    const TRect avatar( aRect.iTl.iX + 6, aRect.iTl.iY + 6,
                        aRect.iTl.iX + 6 + diameter,
                        aRect.iTl.iY + 6 + diameter );
    aGc.SetBrushColor( AvatarColour( aIndex ) );
    aGc.DrawEllipse( avatar );
    if ( iNameFont && c.iName.Length() > 0 )
        {
        aGc.UseFont( iNameFont );
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( Paper() );
        TBuf<1> initial;
        initial.Append( c.iName[ 0 ] );
        aGc.DrawText( initial, avatar,
                      ( avatar.Height() + iNameFont->AscentInPixels() ) / 2,
                      CGraphicsContext::ECenter );
        aGc.DiscardFont();
        }
    if ( iNameFont )
        {
        aGc.UseFont( iNameFont );
        aGc.SetPenColor( Ink() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
        TRect nb( avatar.iBr.iX + 8, aRect.iTl.iY + 6,
                  aRect.iBr.iX - 6,
                  aRect.iTl.iY + 6 + iNameFont->HeightInPixels() );
        TBuf<40> name;
        name.Copy( c.iName );
        ClipText( name, *iNameFont, nb.Width() );
        aGc.DrawText( name, nb, iNameFont->AscentInPixels(),
                      CGraphicsContext::ELeft );
        aGc.DiscardFont();
        }
    }

void CSymgramAppView::DrawBubble( CWindowGc& aGc, const TRect& aBox,
                                  const TSymgramMsg& aMsg,
                                  TBool aSelected ) const
    {
    aGc.SetPenStyle( aSelected ? CGraphicsContext::ESolidPen
                               : CGraphicsContext::ENullPen );
    if ( aSelected )
        {
        aGc.SetPenColor( Brand() );
        aGc.SetPenSize( TSize( 2, 2 ) );
        }
    aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    aGc.SetBrushColor( aMsg.iOut ? TRgb( 0xE3, 0xF1, 0xFB ) : Rule() );
    aGc.DrawRoundRect( aBox, TSize( 6, 6 ) );
    aGc.SetPenSize( TSize( 1, 1 ) );
    TInt y = aBox.iTl.iY + 4;
    if ( aMsg.iBmp )
        {
        const TSize sz = aMsg.iBmp->SizeInPixels();
        TRect img( aBox.iTl.iX + 4, y,
                   aBox.iTl.iX + 4 + sz.iWidth,
                   y + sz.iHeight );
        aGc.DrawBitmap( img, aMsg.iBmp );
        y += sz.iHeight + 4;
        }
    if ( iTextFont && aMsg.iText.Length() > 0 )
        {
        const CFont* font = ChatFont();
        TRect tb( aBox.iTl.iX + 6, y, aBox.iBr.iX - 6,
                  y + font->HeightInPixels() );
        DrawMsgText( aGc, *font, tb, aMsg.iText, iEmojiBmp, iEmojiMask );
        }
    }

void CSymgramAppView::ClearMessages()
    {
    TInt i = 0;
    for ( i = 0; i < iMsgs.Count(); i++ )
        {
        delete iMsgs[ i ].iBmp;
        iMsgs[ i ].iBmp = NULL;
        }
    iMsgs.Reset();
    iThumbAt = 0;
    }

TInt CSymgramAppView::FindChat( TInt64 aId ) const
    {
    TInt i = 0;
    for ( i = 0; i < iChats.Count(); i++ )
        {
        if ( iChats[ i ].iId == aId )
            {
            return i;
            }
        }
    return -1;
    }

void CSymgramAppView::FormatHm( TInt aUnix, TDes& aOut ) const
    {
    aOut.Zero();
    if ( aUnix <= 0 )
        {
        return;
        }
    TTime epoch;
    _LIT( KEpoch, "19700000:000000.000000" );
    if ( epoch.Set( KEpoch ) != KErrNone )
        {
        return;
        }
    const TTime t = epoch + TTimeIntervalSeconds( aUnix );
    const TDateTime d = t.DateTime();
    if ( d.Hour() < 10 )
        {
        aOut.Append( '0' );
        }
    aOut.AppendNum( d.Hour() );
    aOut.Append( ':' );
    if ( d.Minute() < 10 )
        {
        aOut.Append( '0' );
        }
    aOut.AppendNum( d.Minute() );
    }

void CSymgramAppView::PhoneText( TDes& aOut ) const
    {
    aOut.Zero();
    if ( !iSession )
        {
        return;
        }
    const TDesC8& p = iSession->Phone();
    TInt i = 0;
    for ( i = 0; i < p.Length() && aOut.Length() < aOut.MaxLength(); i++ )
        {
        aOut.Append( (TText)p[ i ] );
        }
    }

TInt CSymgramAppView::CurrentCount() const
    {
    if ( iPane == 1 )
        {
        return iContacts.Count();
        }
    if ( iPane == 2 )
        {
        return 5;
        }
    return ChatCount();
    }

TBool CSymgramAppView::ChatInTab( const TSymgramChat& aChat ) const
    {
    if ( iTab == 1 )
        {
        return aChat.iPeerKind == 2 || aChat.iPeerKind == 4;
        }
    if ( iTab == 2 )
        {
        return aChat.iPeerKind == 3;
        }
    return aChat.iPeerKind == 1 || aChat.iPeerKind == 0;
    }

TInt CSymgramAppView::ChatCount() const
    {
    TInt n = 0;
    TInt i = 0;
    for ( i = 0; i < iChats.Count(); i++ )
        {
        if ( ChatInTab( iChats[ i ] ) )
            {
            n++;
            }
        }
    return n;
    }

TInt CSymgramAppView::ChatIndex( TInt aRow ) const
    {
    TInt vis = 0;
    TInt i = 0;
    for ( i = 0; i < iChats.Count(); i++ )
        {
        if ( !ChatInTab( iChats[ i ] ) )
            {
            continue;
            }
        if ( vis == aRow )
            {
            return i;
            }
        vis++;
        }
    return -1;
    }

void CSymgramAppView::RefreshL()
    {
    if ( iSession && iSignedIn )
        {
        iSession->RefreshDialogsL();
        }
    }

void CSymgramAppView::ShowContactsL()
    {
    if ( !iSignedIn || iInChat )
        {
        return;
        }
    iPane = 1;
    iSelected = 0;
    iTopRow = 0;
    DrawDeferred();
    }

void CSymgramAppView::ShowSettingsL()
    {
    if ( !iSignedIn || iInChat )
        {
        return;
        }
    iPane = 2;
    iSelected = 0;
    iTopRow = 0;
    if ( iUpdate )
        {
        iUpdate->PeekLocal();
        }
    DrawDeferred();
    }

void CSymgramAppView::ClosePicker()
    {
    iPick = 0;
    iPickSel = 0;
    DrawDeferred();
    }

void CSymgramAppView::ComposeL()
    {
    if ( !iInChat || !iSession )
        {
        return;
        }
    TBuf<140> text;
    CAknTextQueryDialog* dlg = CAknTextQueryDialog::NewL( text );
    dlg->SetMaxLength( 140 );
    if ( !dlg->ExecuteLD( R_SYMGRAM_SEND_QUERY ) )
        {
        return;
        }
    SendDraftL( text );
    }

void CSymgramAppView::SendDraftL( const TDesC& aText )
    {
    if ( aText.Length() == 0 || !iSession )
        {
        return;
        }
    const TInt idx = FindChat( iOpenId );
    if ( idx < 0 )
        {
        return;
        }
    TBuf8<400> utf;
    ToUtf8( aText, utf );
    if ( utf.Length() == 0 )
        {
        return;
        }
    TSymgramMsg m;
    m.iId = 0;
    m.iDate = 0;
    m.iOut = ETrue;
    m.iKind = (TUint8)ESgText;
    m.iDuration = 0;
    m.iPhotoLoc = EFalse;
    m.iFileId = 0;
    m.iFileHash = 0;
    m.iBmp = NULL;
    m.iText.Copy( aText.Left( m.iText.MaxLength() ) );
    iMsgs.Append( m );
    iChats[ idx ].iPreview.Copy( aText.Left( iChats[ idx ].iPreview.MaxLength() ) );
    DrawDeferred();
    iSession->SendTextL( iChats[ idx ].iId, iChats[ idx ].iPeerKind,
                         iChats[ idx ].iHash, utf );
    }

void CSymgramAppView::PickEmojiL()
    {
    if ( !iInChat )
        {
        return;
        }
    iPick = 1;
    iPickSel = 0;
    DrawDeferred();
    }

void CSymgramAppView::SendPickedEmojiL()
    {
    if ( iPickSel < 0 || iPickSel >= KEmojiCount )
        {
        return;
        }
    TBuf<8> text;
    AppendCp( text, KEmojiList[ iPickSel ] );
    iPick = 0;
    SendDraftL( text );
    }

void CSymgramAppView::ScanFolder( const TDesC& aDir )
    {
    if ( !iFiles )
        {
        return;
        }
    RFs& fs = iCoeEnv->FsSession();
    TFileName wild;
    wild.Copy( aDir );
    _LIT( KAll, "*.*" );
    wild.Append( KAll );
    CDir* dir = NULL;
    if ( fs.GetDir( wild, KEntryAttNormal, ESortByDate, dir ) != KErrNone )
        {
        return;
        }
    TInt i = 0;
    for ( i = 0; i < dir->Count() && iFiles->Count() < 40; i++ )
        {
        const TEntry& e = ( *dir )[ i ];
        if ( e.IsDir() || e.iSize < 32 || e.iSize > 524288 )
            {
            continue;
            }
        const TInt n = e.iName.Length();
        if ( n < 5 )
            {
            continue;
            }
        TBuf<8> ext;
        ext.Copy( e.iName.Right( 4 ) );
        ext.LowerCase();
        TBool ok = ext.Compare( _L( ".jpg" ) ) == 0 ||
                   ext.Compare( _L( ".png" ) ) == 0;
        if ( !ok && n >= 5 )
            {
            ext.Copy( e.iName.Right( 5 ) );
            ext.LowerCase();
            ok = ext.Compare( _L( ".jpeg" ) ) == 0;
            }
        if ( !ok )
            {
            continue;
            }
        TFileName full;
        full.Copy( aDir );
        full.Append( e.iName );
        TRAP_IGNORE( iFiles->AppendL( full ) );
        }
    delete dir;
    }

void CSymgramAppView::PickPhotoL()
    {
    if ( !iInChat )
        {
        return;
        }
    if ( !iFiles )
        {
        iFiles = new ( ELeave ) CDesCArrayFlat( 8 );
        }
    iFiles->Reset();
    _LIT( K0, "C:\\Data\\Images\\" );
    _LIT( K1, "C:\\Data\\Images\\Camera\\" );
    _LIT( K2, "E:\\Images\\" );
    _LIT( K3, "E:\\Images\\Camera\\" );
    _LIT( K4, "E:\\Data\\Images\\" );
    _LIT( K5, "C:\\Images\\" );
    ScanFolder( K0 );
    ScanFolder( K1 );
    ScanFolder( K2 );
    ScanFolder( K3 );
    ScanFolder( K4 );
    ScanFolder( K5 );
    if ( iFiles->Count() == 0 )
        {
        _LIT( KNone, "Нет фото на телефоне" );
        SetStatusL( KNone );
        return;
        }
    iPick = 2;
    iPickSel = 0;
    DrawDeferred();
    }

void CSymgramAppView::SendPickedPhotoL()
    {
    if ( !iFiles || iPickSel < 0 || iPickSel >= iFiles->Count() || !iSession )
        {
        return;
        }
    const TInt idx = FindChat( iOpenId );
    if ( idx < 0 )
        {
        return;
        }
    TFileName path;
    path.Copy( ( *iFiles )[ iPickSel ] );
    iPick = 0;
    TSymgramMsg m;
    m.iId = 0;
    m.iDate = 0;
    m.iOut = ETrue;
    m.iKind = (TUint8)ESgPhoto;
    m.iDuration = 0;
    m.iPhotoLoc = EFalse;
    m.iFileId = 0;
    m.iFileHash = 0;
    m.iBmp = NULL;
    _LIT( KPhoto, "фото" );
    m.iText.Copy( KPhoto );
    iMsgs.Append( m );
    iChats[ idx ].iPreview.Copy( KPhoto );
    DrawDeferred();
    TRAPD( err, iSession->SendFileL( iChats[ idx ].iId, iChats[ idx ].iPeerKind,
                                     iChats[ idx ].iHash, path ) );
    if ( err != KErrNone )
        {
        _LIT( KFail, "Не удалось открыть файл" );
        SetStatusL( KFail );
        }
    }

void CSymgramAppView::DrawEmojiPicker( CWindowGc& aGc, const TRect& aRect ) const
    {
    const TInt cols = 6;
    const TInt rows = ( KEmojiCount + cols - 1 ) / cols;
    const TInt cw = aRect.Width() / cols;
    TInt ch = aRect.Height() / ( rows + 1 );
    if ( ch > cw )
        {
        ch = cw;
        }
    TInt i = 0;
    for ( i = 0; i < KEmojiCount; i++ )
        {
        const TInt col = i % cols;
        const TInt row = i / cols;
        TRect cell( aRect.iTl.iX + col * cw, aRect.iTl.iY + row * ch,
                    aRect.iTl.iX + ( col + 1 ) * cw,
                    aRect.iTl.iY + ( row + 1 ) * ch );
        aGc.SetPenStyle( CGraphicsContext::ENullPen );
        aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
        aGc.SetBrushColor( i == iPickSel ? Highlight() : Paper() );
        aGc.DrawRect( cell );
        const TInt sz = ( cw < ch ? cw : ch ) - 8;
        if ( sz > 8 )
            {
            DrawEmojiIcon( aGc, cell.iTl.iX + ( cw - sz ) / 2,
                           cell.iTl.iY + ( ch - sz ) / 2, sz,
                           KEmojiList[ i ], iEmojiBmp[ i ], iEmojiMask[ i ] );
            }
        }
    if ( iTextFont )
        {
        aGc.UseFont( iTextFont );
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( Muted() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
        _LIT( KHint, "Центр — отправить. C — назад." );
        TRect hint( aRect.iTl.iX + 6, aRect.iBr.iY - iTextFont->HeightInPixels() - 4,
                    aRect.iBr.iX - 6, aRect.iBr.iY - 2 );
        aGc.DrawText( KHint, hint, iTextFont->AscentInPixels(),
                      CGraphicsContext::ELeft );
        aGc.DiscardFont();
        }
    }

void CSymgramAppView::DrawFilePicker( CWindowGc& aGc, const TRect& aRect ) const
    {
    const TInt n = iFiles ? iFiles->Count() : 0;
    const TInt rowH = RowHeight();
    const TInt vis = rowH > 0 ? aRect.Height() / rowH : 0;
    TInt top = 0;
    if ( vis > 0 && iPickSel >= vis )
        {
        top = iPickSel - vis + 1;
        }
    TInt i = 0;
    for ( i = 0; i < vis && top + i < n; i++ )
        {
        const TInt idx = top + i;
        TRect row( aRect.iTl.iX, aRect.iTl.iY + i * rowH,
                   aRect.iBr.iX, aRect.iTl.iY + ( i + 1 ) * rowH );
        aGc.SetPenStyle( CGraphicsContext::ENullPen );
        aGc.SetBrushStyle( CGraphicsContext::ESolidBrush );
        aGc.SetBrushColor( idx == iPickSel ? Highlight() : Paper() );
        aGc.DrawRect( row );
        if ( !iNameFont )
            {
            continue;
            }
        TParse parse;
        parse.Set( ( *iFiles )[ idx ], NULL, NULL );
        TBuf<40> name;
        name.Copy( parse.NameAndExt().Left( name.MaxLength() ) );
        aGc.UseFont( iNameFont );
        aGc.SetPenStyle( CGraphicsContext::ESolidPen );
        aGc.SetPenColor( Ink() );
        aGc.SetBrushStyle( CGraphicsContext::ENullBrush );
        TRect nb( row.iTl.iX + 8, row.iTl.iY + 4, row.iBr.iX - 8,
                  row.iTl.iY + 4 + iNameFont->HeightInPixels() );
        aGc.DrawText( name, nb, iNameFont->AscentInPixels(),
                      CGraphicsContext::ELeft );
        aGc.DiscardFont();
        }
    }

void CSymgramAppView::EnsureMsgVisible()
    {
    if ( iMsgs.Count() == 0 )
        {
        iMsgSel = 0;
        iTopRow = 0;
        return;
        }
    if ( iMsgSel < 0 )
        {
        iMsgSel = 0;
        }
    if ( iMsgSel >= iMsgs.Count() )
        {
        iMsgSel = iMsgs.Count() - 1;
        }
    const TInt pos = iMsgs.Count() - 1 - iMsgSel;
    TInt vis = 4;
    const TInt h = ChatFont() ? ChatFont()->HeightInPixels() + 14 : 28;
    const TInt rh = ListRect().Height();
    if ( h > 0 && rh > 0 )
        {
        vis = rh / h;
        }
    if ( vis < 1 )
        {
        vis = 1;
        }
    if ( pos < iTopRow )
        {
        iTopRow = pos;
        }
    else if ( pos >= iTopRow + vis )
        {
        iTopRow = pos - vis + 1;
        }
    if ( iTopRow < 0 )
        {
        iTopRow = 0;
        }
    }

TInt CSymgramAppView::SelectedAttach() const
    {
    if ( iMsgSel < 0 || iMsgSel >= iMsgs.Count() )
        {
        return -1;
        }
    if ( iMsgs[ iMsgSel ].iFileId == 0 )
        {
        return -1;
        }
    return iMsgSel;
    }

void CSymgramAppView::AttachmentName( const TSymgramMsg& aMsg, TDes& aOut ) const
    {
    aOut.Zero();
    TInt i = 0;
    if ( aMsg.iFileName.Length() > 0 )
        {
        for ( i = 0; i < aMsg.iFileName.Length() &&
                     aOut.Length() < aOut.MaxLength(); i++ )
            {
            const TText c = aMsg.iFileName[ i ];
            if ( ( c >= '0' && c <= '9' ) ||
                 ( c >= 'A' && c <= 'Z' ) ||
                 ( c >= 'a' && c <= 'z' ) ||
                 c == '.' || c == '-' || c == '_' )
                {
                aOut.Append( c );
                }
            }
        }
    if ( aOut.Length() == 0 )
        {
        _LIT( KPre, "sg" );
        aOut.Copy( KPre );
        aOut.AppendNum( aMsg.iId );
        switch ( aMsg.iKind )
            {
            case ESgPhoto:   aOut.Append( _L( ".jpg" ) ); break;
            case ESgVoice:   aOut.Append( _L( ".ogg" ) ); break;
            case ESgAudio:   aOut.Append( _L( ".mp3" ) ); break;
            case ESgVideo:   aOut.Append( _L( ".mp4" ) ); break;
            case ESgSticker: aOut.Append( _L( ".webp" ) ); break;
            default:         aOut.Append( _L( ".bin" ) ); break;
            }
        }
    }

TBool CSymgramAppView::MakeAttachPath( const TSymgramMsg& aMsg, TDes& aOut ) const
    {
    const TDesC* sub = &PathInfo::ImagesPath();
    if ( aMsg.iKind == ESgVoice || aMsg.iKind == ESgAudio )
        {
        sub = &PathInfo::SoundsPath();
        }
    else if ( aMsg.iKind == ESgVideo )
        {
        sub = &PathInfo::VideosPath();
        }
    else if ( aMsg.iKind == ESgFile )
        {
        sub = &PathInfo::OthersPath();
        }
    TBuf<40> name;
    AttachmentName( aMsg, name );
    TInt pass = 0;
    for ( pass = 0; pass < 2; pass++ )
        {
        aOut.Copy( pass == 0 ? PathInfo::MemoryCardRootPath()
                             : PathInfo::PhoneMemoryRootPath() );
        aOut.Append( *sub );
        iCoeEnv->FsSession().MkDirAll( aOut );
        TInt room = aOut.MaxLength() - aOut.Length();
        if ( room < name.Length() )
            {
            continue;
            }
        aOut.Append( name );
        return ETrue;
        }
    return EFalse;
    }

void CSymgramAppView::OpenSystemL( const TDesC& aPath )
    {
    CDocumentHandler* doc = CDocumentHandler::NewLC();
    TDataType type;
    const TInt err = doc->OpenFileL( aPath, type );
    CleanupStack::PopAndDestroy( doc );
    if ( err != KErrNone && err != KUserCancel )
        {
        TBuf<40> text;
        _LIT( KFail, "Нечем открыть " );
        text.Copy( KFail );
        text.AppendNum( err );
        SetStatusL( text );
        }
    }

void CSymgramAppView::StartAttachL( TBool aOpen )
    {
    const TInt idx = SelectedAttach();
    if ( idx < 0 || !iSession )
        {
        _LIT( KNone, "Нет вложения" );
        SetStatusL( KNone );
        return;
        }
    const TSymgramMsg& m = iMsgs[ idx ];
    TFileName path;
    if ( !MakeAttachPath( m, path ) )
        {
        _LIT( KPath, "Нет папки для файла" );
        SetStatusL( KPath );
        return;
        }
    TEntry entry;
    if ( iCoeEnv->FsSession().Entry( path, entry ) == KErrNone &&
         entry.iSize > 32 )
        {
        if ( aOpen )
            {
            OpenSystemL( path );
            }
        else
            {
            _LIT( KOk, "Уже сохранено" );
            SetStatusL( KOk );
            }
        return;
        }
    iSession->GetFileSaveL( m.iId, m.iFileId, m.iFileHash, m.iFileRef,
                            m.iPhotoLoc, path, aOpen );
    }

void CSymgramAppView::OpenAttachmentL()
    {
    StartAttachL( ETrue );
    }

void CSymgramAppView::SaveAttachmentL()
    {
    StartAttachL( EFalse );
    }

void CSymgramAppView::LogoutAskL()
    {
    if ( !iSession )
        {
        return;
        }
    iSession->LogoutL();
    }

void CSymgramAppView::CheckUpdateL()
    {
    if ( !iUpdate )
        {
        return;
        }
    iUpdate->PeekLocal();
    if ( iUpdate->PackageNewer() ||
         ( iUpdate->NetDone() && iUpdate->CanInstall() ) )
        {
        OpenSystemL( iUpdate->PackagePath() );
        return;
        }
    iUpdate->StartL();
    }

void CSymgramAppView::UpdateStatusL( const TDesC& aText )
    {
    SetStatusL( aText );
    }

void CSymgramAppView::UpdateReadyL()
    {
    DrawDeferred();
    }

void CSymgramAppView::SizeChanged()
    {
    EnsureSelectionVisible();
    }
