#ifndef __SYMGRAMAPPVIEW_H__
#define __SYMGRAMAPPVIEW_H__

#include <coecntrl.h>
#include <e32base.h>
#include <badesca.h>
#include "SymgramSession.h"

class TSymgramChat
    {
    public:
        TBuf<40> iName;
        TBuf<80> iPreview;
        TBuf<12> iTime;
        TInt     iUnread;
    };

class CSymgramAppView : public CCoeControl, public MSymgramSessionObserver
    {
    public:
        static CSymgramAppView* NewL( const TRect& aRect );
        static CSymgramAppView* NewLC( const TRect& aRect );
        ~CSymgramAppView();

    public:
        void SetStatusL( const TDesC& aStatus );
        void NextL();
        void CycleCountry( TInt aDelta );
        void QueryCountryL();

    public: // from CCoeControl
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

    private: // from MSymgramSessionObserver
        void SessionStatusL( const TDesC& aText );
        void SessionFailedL( TInt aError );
        void SessionErrorL( const TDesC& aText );
        void SessionCodeSentL();
        void SessionPasswordNeededL( const TDesC& aHint );
        void SessionSignedInL();

    private: // from CCoeControl
        void Draw( const TRect& aRect ) const;
        void SizeChanged();

    private:
        void ConstructL( const TRect& aRect );
        CSymgramAppView();

        void DrawHeader( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawRow( CWindowGc& aGc, const TRect& aRect, TInt aIndex ) const;
        void DrawSignIn( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawSignInField( CWindowGc& aGc, const TRect& aRow, TBool aOn,
                              const TDesC& aLeft, const TDesC& aRight ) const;
        void DrawEmptyState( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawCenteredPair( CWindowGc& aGc, const TRect& aRect,
                               const HBufC* aTitle, const HBufC* aDetail ) const;

        TInt HeaderHeight() const;
        TInt RowHeight() const;
        TInt VisibleRows() const;
        TRect ListRect() const;
        void EnsureSelectionVisible();
        void CountryName( TDes& aOut ) const;
        TInt CallingCode() const;
        void QueryPasswordL();
        void HandleArrowL( TInt aDir );
        void HandleBackspace();
        static TInt PasswordIdleCb( TAny* aPtr );

    private:
        const CFont* iTitleFont;
        const CFont* iNameFont;
        const CFont* iTextFont;

        HBufC* iStatus;
        HBufC* iSignInTitle;
        HBufC* iSignInHint;
        HBufC* iEmptyTitle;
        HBufC* iEmptyDetail;
        HBufC* iCodeTitle;
        HBufC* iCodeHint;
        HBufC* iPasswordPrompt;
        HBufC* iFieldCountry;
        HBufC* iFieldPhone;

        CDesCArray* iCountries;

        TBool iSignedIn;
        TBool iAwaitingCode;
        TBool iAwaitingPassword;
        TInt iFocus;
        TInt iCountry;
        TBuf<16> iPhone;
        TBuf<8> iSmsCode;
        TBuf<80> iPwdHint;

        CSymgramSession* iSession;
        CIdle* iPasswordIdle;
        TInt iNavDown;

        RArray<TSymgramChat> iChats;
        TInt iSelected;
        TInt iTopRow;
    };

#endif
