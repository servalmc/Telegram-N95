#ifndef __SYMGRAMAPPVIEW_H__
#define __SYMGRAMAPPVIEW_H__

#include <coecntrl.h>

// One row of the chat list. Nothing fills this yet: rows will come from
// messages.getDialogs once the MTProto layer exists.
class TSymgramChat
    {
    public:
        TBuf<40> iName;
        TBuf<80> iPreview;
        TBuf<12> iTime;
        TInt     iUnread;
    };

class CSymgramAppView : public CCoeControl
    {
    public:
        static CSymgramAppView* NewL( const TRect& aRect );
        static CSymgramAppView* NewLC( const TRect& aRect );
        ~CSymgramAppView();

    public:
        void SetStatusL( const TDesC& aStatus );

    public: // from CCoeControl
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

    private: // from CCoeControl
        void Draw( const TRect& aRect ) const;
        void SizeChanged();

    private:
        void ConstructL( const TRect& aRect );
        CSymgramAppView();

        void DrawHeader( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawRow( CWindowGc& aGc, const TRect& aRect, TInt aIndex ) const;
        void DrawEmptyState( CWindowGc& aGc, const TRect& aRect ) const;

        TInt HeaderHeight() const;
        TInt RowHeight() const;
        TInt VisibleRows() const;
        TRect ListRect() const;
        void EnsureSelectionVisible();

    private:
        const CFont* iTitleFont;
        const CFont* iNameFont;
        const CFont* iTextFont;

        HBufC* iStatus;
        HBufC* iEmptyTitle;
        HBufC* iEmptyDetail;

        RArray<TSymgramChat> iChats;
        TInt iSelected;
        TInt iTopRow;
    };

#endif // __SYMGRAMAPPVIEW_H__
