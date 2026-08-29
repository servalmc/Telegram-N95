#ifndef __SYMGRAMAPPVIEW_H__
#define __SYMGRAMAPPVIEW_H__

#include <coecntrl.h>
#include <e32base.h>
#include <badesca.h>
#include "SymgramSession.h"
#include "SymgramTypes.h"
#include "SymgramUpdate.h"

class CSymgramStore;
class CSymgramJpeg;
class CFbsBitmap;
class CSymgramUpdate;

enum { ESgEmojiCount = 40, ESgEmojiPickCount = 24 };

class CSymgramAppView : public CCoeControl, public MSymgramSessionObserver,
                        public MSymgramUpdateObserver
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
        void RefreshL();
        void ShowContactsL();
        void ShowSettingsL();
        void ComposeL();
        void PickEmojiL();
        void PickPhotoL();
        void OpenAttachmentL();
        void SaveAttachmentL();
        void LogoutAskL();
        void ShowUpdateL();
        void CheckUpdateL();
        void InstallUpdateL();
        TBool ShowNextCommand() const;
        TBool ShowCountryCommand() const;
        TBool ShowRefreshCommand() const;
        TBool ShowListCommand() const;
        TBool ShowWriteCommand() const;
        TBool ShowAttachCommand() const;
        TBool ShowLogoutCommand() const;
        TBool CanGoBack() const;
        TBool GoBack();
        void JpegReady( CFbsBitmap* aBmp, TInt aMsgId );

    public: // from CCoeControl
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

    private: // from MSymgramSessionObserver
        void SessionStatusL( const TDesC& aText );
        void SessionFailedL( TInt aError );
        void SessionErrorL( const TDesC& aText );
        void SessionCodeSentL();
        void SessionPasswordNeededL( const TDesC& aHint );
        void SessionSignedInL();
        void SessionLoggedOutL();
        void SessionAddChatL( const TSymgramChat& aChat );
        void SessionChatsReadyL();
        void SessionClearContactsL();
        void SessionAddContactL( const TSymgramContact& aContact );
        void SessionContactsReadyL();
        void SessionBeginMessagesL( TInt64 aPeer );
        void SessionAddMessageL( const TSymgramMsg& aMsg );
        void SessionMessagesReadyL( TInt64 aPeer );
        void SessionThumbL( TInt aMsgId, const TDesC8& aJpeg );
        void SessionFileSavedL( const TDesC& aPath, TBool aOpen );
        void SessionSentL( TInt aId, TInt aDate );
        void SessionPeerReadL( TInt64 aPeer );

    private: // from MSymgramUpdateObserver
        void UpdateStatusL( const TDesC& aText );
        void UpdateReadyL();

    private: // from CCoeControl
        void Draw( const TRect& aRect ) const;
        void SizeChanged();

    private:
        void ConstructL( const TRect& aRect );
        CSymgramAppView();

        void DrawHeader( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawRow( CWindowGc& aGc, const TRect& aRect, TInt aIndex ) const;
        void DrawSignIn( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawCountryList( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawSignInField( CWindowGc& aGc, const TRect& aRow, TBool aOn,
                              const TDesC& aLeft, const TDesC& aRight ) const;
        void DrawChat( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawEmptyState( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawCenteredPair( CWindowGc& aGc, const TRect& aRect,
                               const HBufC* aTitle, const HBufC* aDetail ) const;
        void DrawTabs( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawSettings( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawUpdate( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawSettingRow( CWindowGc& aGc, const TRect& aRow, TBool aOn,
                             const TDesC& aTitle, const TDesC& aDetail ) const;
        void DrawContactRow( CWindowGc& aGc, const TRect& aRect,
                             TInt aIndex ) const;
        void DrawBubble( CWindowGc& aGc, const TRect& aBox,
                         const TSymgramMsg& aMsg, TBool aSelected ) const;
        void DrawEmojiPicker( CWindowGc& aGc, const TRect& aRect ) const;
        void DrawFilePicker( CWindowGc& aGc, const TRect& aRect ) const;

        TInt HeaderHeight() const;
        TInt RowHeight() const;
        TInt TabHeight() const;
        TInt VisibleRows() const;
        TRect ListRect() const;
        void EnsureSelectionVisible();
        void CountryName( TDes& aOut ) const;
        TInt CallingCode() const;
        void QueryPasswordL();
        void EditPasswordL();
        void HandleArrowL( TInt aDir );
        void HandleBackspace();
        void OpenSelectedChat();
        void OpenChatAt( TInt aIndex );
        void OpenContact();
        void ClosePane();
        void SyncSoftkeys();
        TBool ChatInTab( const TSymgramChat& aChat ) const;
        TInt ChatCount() const;
        TInt ChatIndex( TInt aRow ) const;
        void CloseChat();
        void ClosePicker();
        void ScanFolder( const TDesC& aDir );
        void SendDraftL( const TDesC& aText );
        void SendPickedEmojiL();
        void SendPickedPhotoL();
        void EnsureMsgVisible();
        TInt SelectedAttach() const;
        void StartAttachL( TBool aOpen );
        void AttachmentName( const TSymgramMsg& aMsg, TDes& aOut ) const;
        TBool MakeAttachPath( const TSymgramMsg& aMsg, TDes& aOut ) const;
        void OpenSystemL( const TDesC& aPath );
        TInt FindChat( TInt64 aId ) const;
        void ClearMessages();
        void RequestNextThumb();
        void FormatHm( TInt aUnix, TDes& aOut ) const;
        void PhoneText( TDes& aOut ) const;
        TInt CurrentCount() const;
        const CFont* ChatFont() const;
        void LoadEmojiL();
        void TrimMessages();
        void MarkChatRead( TInt64 aPeer );

    private:
        const CFont* iTitleFont;
        const CFont* iNameFont;
        const CFont* iTextFont;
        CFont* iChatFont;
        CFbsBitmap* iEmojiBmp[ ESgEmojiCount ];
        CFbsBitmap* iEmojiMask[ ESgEmojiCount ];

        HBufC* iStatus;
        HBufC* iSignInTitle;
        HBufC* iSignInHint;
        HBufC* iEmptyTitle;
        HBufC* iEmptyDetail;
        HBufC* iCodeTitle;
        HBufC* iCodeHint;
        HBufC* iPasswordPrompt;
        HBufC* iPasswordHint;
        HBufC* iFieldCountry;
        HBufC* iFieldPhone;
        HBufC* iChatHint;

        CDesCArray* iCountries;

        TBool iSignedIn;
        TBool iAwaitingCode;
        TBool iAwaitingPassword;
        TBool iPickingCountry;
        TInt iFocus;
        TInt iCountry;
        TBuf<16> iPhone;
        TBuf<8> iSmsCode;
        TBuf<128> iCloudPwd;
        TBuf<80> iPwdHint;

        CSymgramSession* iSession;
        CSymgramStore* iStore;
        CSymgramJpeg* iJpeg;
        CSymgramUpdate* iUpdate;
        TInt iNavDown;

        RArray<TSymgramChat> iChats;
        RArray<TSymgramContact> iContacts;
        RArray<TSymgramMsg> iMsgs;
        TInt iTab;
        TInt iPane;
        TInt iSelected;
        TInt iTopRow;
        TBool iInChat;
        TInt64 iOpenId;
        TInt iThumbAt;
        TInt iSetSel;
        TInt iPick;
        TInt iPickSel;
        TInt iMsgSel;
        CDesCArray* iFiles;
    };

#endif
