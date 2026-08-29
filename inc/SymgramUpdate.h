#ifndef __SYMGRAMUPDATE_H__
#define __SYMGRAMUPDATE_H__

#include <e32base.h>
#include <f32file.h>
#include <es_sock.h>
#include <in_sock.h>
#include <commdbconnpref.h>

class MSymgramUpdateObserver
    {
    public:
        virtual void UpdateStatusL( const TDesC& aText ) = 0;
        virtual void UpdateReadyL() = 0;
    };

class CSymgramUpdate : public CActive
    {
    public:
        static CSymgramUpdate* NewL( MSymgramUpdateObserver& aObserver );
        ~CSymgramUpdate();

        void StartL();
        void PeekLocal();
        TBool CanInstall() const;
        TBool PackageNewer() const;
        TBool NetDone() const;
        const TDesC& PackagePath() const;
        const TDesC& RemoteTag() const;
        TBool RemoteIsNewer() const;

    private:
        enum TState
            {
            EIdle,
            EStarting,
            EResolving,
            EConnecting,
            EWriting,
            EReading
            };

        CSymgramUpdate( MSymgramUpdateObserver& aObserver );
        void ConstructL();

        void RunL();
        void DoCancel();
        TInt RunError( TInt aError );

        void ScanLocal();
        void ScanDir( const TDesC& aDir );
        void ConsiderFile( const TDesC& aPath, const TDesC& aName );
        void BeginNetL();
        void ResolveNextL();
        void ConnectL();
        void SendGetL();
        void ReadMoreL();
        void HandleBodyL();
        void FinishL( const TDesC& aText );
        void CloseNet();
        TInt OpenTcp();

        static TInt PackVer( TInt aMaj, TInt aMin, TInt aBld );
        static TInt CurrentPack();
        static TBool ParseVer8( const TDesC8& aText, TInt& aMaj,
                                TInt& aMin, TInt& aBld );
        static TBool ParseVer16( const TDesC& aText, TInt& aMaj,
                                 TInt& aMin, TInt& aBld );
        static TBool FindVer8( const TDesC8& aText, TInt& aMaj,
                               TInt& aMin, TInt& aBld );

    private:
        MSymgramUpdateObserver& iObserver;
        TState iState;
        TInt iHost;

        RFs iFs;
        RSocketServ iServ;
        RConnection iConn;
        RHostResolver iResolver;
        RSocket iSocket;
        TNameEntry iNameEntry;
        TInetAddr iAddr;
        TCommDbConnPref iPref;
        TSockXfrLength iXfrLen;
        TBuf8<384> iReq;
        TPtr8 iRead;
        HBufC8* iBody;
        TInt iHave;

        TFileName iPkgPath;
        TInt iPkgPack;
        TBuf<16> iRemoteTag;
        TInt iRemotePack;
        TBool iRemoteNewer;
        TBool iNetDone;
    };

#endif
