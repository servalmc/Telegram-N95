#ifndef __SYMGRAMSESSION_H__
#define __SYMGRAMSESSION_H__

#include <e32base.h>
#include <es_sock.h>
#include <in_sock.h>
#include <commdbconnpref.h>
#include "SymgramCrypto.h"

class MSymgramSessionObserver
    {
    public:
        virtual void SessionStatusL( const TDesC& aText ) = 0;
        virtual void SessionFailedL( TInt aError ) = 0;
        virtual void SessionErrorL( const TDesC& aText ) = 0;
        virtual void SessionCodeSentL() = 0;
        virtual void SessionPasswordNeededL( const TDesC& aHint ) = 0;
        virtual void SessionSignedInL() = 0;
    };

class CSymgramSession : public CActive
    {
    public:
        static CSymgramSession* NewL( MSymgramSessionObserver& aObserver );
        ~CSymgramSession();

        void ConnectL( const TDesC8& aPhone );
        void SubmitCodeL( const TDesC8& aCode );
        TInt SubmitPasswordL( const TDesC8& aPassword );
        TBool IsBusy() const;

    private:
        enum TState
            {
            EIdle,
            EStarting,
            EConnecting,
            EWriting,
            EReading,
            EResuming
            };

        CSymgramSession( MSymgramSessionObserver& aObserver );
        void ConstructL();

        void RunL();
        void DoCancel();
        TInt RunError( TInt aError );

        void FailL( TInt aError );
        void FailTextL( const TDesC& aText );
        TBool CanResume( TInt aError ) const;
        void ResumeConnectL();
        void RpcFailL( const TDesC8& aMsg );
        void CloseSocket();
        void SendPqL();
        void SendDhParamsL();
        void SendSendCodeL();
        void SendSignInL();
        void SendGetPasswordL();
        void WrapInitL( const TDesC8& aQuery, TDes8& aOut );
        TInt ComputeSrpL( const TDesC8& aPassword, TDes8& aA, TDes8& aM1 );
        void HandlePasswordL( const TUint8* aP, TInt aLen );
        void SendEncryptedL( const TDesC8& aMsg );
        void ReadMoreL();
        void HandleIncomingL();
        void HandleUnencryptedL( const TUint8* aBody, TInt aLen );
        void HandleEncryptedL( const TUint8* aPacket, TInt aLen );
        void DispatchInnerL( const TUint8* aP, TInt aLen );
        void HandlePqL( const TUint8* aBody, TInt aLen );
        void HandleServerDhL( const TUint8* aBody, TInt aLen );
        void HandleDhGenL( const TUint8* aBody, TInt aLen );
        void HandleRpcResultL( const TUint8* aP, TInt aLen );
        HBufC8* UnzipPackedLC( const TUint8* aObj, TInt aLen );
        TInt SkipSentCodeType( const TUint8* aP, TInt aRemain, TInt& aSkip );
        TInt RsaPad( const TDesC8& aData, TDes8& aOut );
        void BuildUnencryptedL( const TDesC8& aBody, TDes8& aOut );
        void WritePacketL( const TDesC8& aPayload );
        void DeriveAes( TInt aX, const TUint8 aMsgKey[ 16 ],
                        TUint8 aAesKey[ 32 ], TUint8 aAesIv[ 32 ] ) const;
        void TmpAes( TUint8 aKey[ 32 ], TUint8 aIv[ 32 ] ) const;
        TUint64 MessageId();
        TUint32 SeqNo();
        TInt UnixNow() const;

    private:
        MSymgramSessionObserver& iObserver;
        TState iState;
        TBool iBusy;
        TBool iSentAbridged;

        RSocketServ iServ;
        RConnection iConn;
        RSocket iSocket;
        TInetAddr iAddr;
        TCommDbConnPref iPref;
        TSockXfrLength iXfrLen;

        TBuf8<16> iNonce;
        TBuf8<16> iServerNonce;
        TBuf8<32> iNewNonce;
        TBuf8<16> iPq;
        TBuf8<8> iP;
        TBuf8<8> iQ;
        TInt64 iFingerprint;
        const TUint8* iRsaN;
        TBuf8<32> iTmpKey;
        TBuf8<32> iTmpIv;
        TBuf8<256> iAuthKey;
        TUint64 iAuthKeyId;
        TUint64 iSalt;
        TUint64 iSessionId;
        TUint64 iLastMsgId;
        TInt iSeq;
        TInt iTimeOffset;
        TInt iPhase;
        TBuf8<24> iPhone;
        TBuf8<64> iPhoneCodeHash;
        TBuf8<16> iCode;
        TBuf8<512> iLastRpc;
        TBool iHaveSrp;
        TUint32 iSrpG;
        TInt64 iSrpId;
        TBuf8<256> iSalt1;
        TBuf8<256> iSalt2;
        TBuf8<256> iSrpP;
        TBuf8<256> iSrpB;

        TBuf8<2048> iOut;
        TBuf8<4096> iIn;
        TPtr8 iRead;
        TInt iHave;
        TBool iResume;
        TInt iResumeTries;
    };

#endif
