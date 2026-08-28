#ifndef __SYMGRAMSESSION_H__
#define __SYMGRAMSESSION_H__

#include <e32base.h>
#include <f32file.h>
#include <es_sock.h>
#include <in_sock.h>
#include <commdbconnpref.h>
#include "SymgramCrypto.h"
#include "SymgramTypes.h"

class MSymgramSessionObserver
    {
    public:
        virtual void SessionStatusL( const TDesC& aText ) = 0;
        virtual void SessionFailedL( TInt aError ) = 0;
        virtual void SessionErrorL( const TDesC& aText ) = 0;
        virtual void SessionCodeSentL() = 0;
        virtual void SessionPasswordNeededL( const TDesC& aHint ) = 0;
        virtual void SessionSignedInL() = 0;
        virtual void SessionLoggedOutL() = 0;
        virtual void SessionAddChatL( const TSymgramChat& aChat ) = 0;
        virtual void SessionChatsReadyL() = 0;
        virtual void SessionClearContactsL() = 0;
        virtual void SessionAddContactL( const TSymgramContact& aContact ) = 0;
        virtual void SessionContactsReadyL() = 0;
        virtual void SessionBeginMessagesL( TInt64 aPeer ) = 0;
        virtual void SessionAddMessageL( const TSymgramMsg& aMsg ) = 0;
        virtual void SessionMessagesReadyL( TInt64 aPeer ) = 0;
        virtual void SessionThumbL( TInt aMsgId, const TDesC8& aJpeg ) = 0;
        virtual void SessionFileSavedL( const TDesC& aPath, TBool aOpen ) = 0;
        virtual void SessionSentL( TInt aId, TInt aDate ) = 0;
    };

class CSymgramSession : public CActive
    {
    public:
        static CSymgramSession* NewL( MSymgramSessionObserver& aObserver );
        ~CSymgramSession();

        void ConnectL( const TDesC8& aPhone );
        void StartSavedL();
        TBool HasSavedSession() const;
        void SubmitCodeL( const TDesC8& aCode );
        TInt SubmitPasswordL( const TDesC8& aPassword );
        TBool IsBusy() const;
        const TDesC8& Phone() const;
        void LogoutL();
        void RefreshDialogsL();
        void GetHistoryL( TInt64 aId, TInt aKind, TInt64 aHash );
        void SendTextL( TInt64 aId, TInt aKind, TInt64 aHash,
                        const TDesC8& aText );
        void SendFileL( TInt64 aId, TInt aKind, TInt64 aHash,
                        const TDesC& aPath );
        void GetFileL( TInt aMsgId, TInt64 aFileId, TInt64 aHash,
                       const TDesC8& aRef, TBool aPhoto );
        void GetFileSaveL( TInt aMsgId, TInt64 aFileId, TInt64 aHash,
                           const TDesC8& aRef, TBool aPhoto,
                           const TDesC& aPath, TBool aOpen );

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

        enum TRpc
            {
            ERpcNone,
            ERpcDialogs,
            ERpcContacts,
            ERpcHistory,
            ERpcSend,
            ERpcFile,
            ERpcSavePart,
            ERpcSaveFile
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
        void MigrateToDcL( TInt aDc );
        TUint32 DcAddr() const;
        TInt OpenTcpSocket();
        void BeginTcpConnectL();
        TBool ShouldRetryAddrSwap( TInt aError ) const;
        void RpcFailL( const TDesC8& aMsg );
        void CloseSocket();
        void SendPqL();
        void SendDhParamsL();
        void SendSendCodeL();
        void SendSignInL();
        void SendGetPasswordL();
        void SendGetDialogsL();
        void SendGetContactsL();
        void SendGetHistoryNowL();
        void SendTextNowL();
        void SendGetFileNowL();
        void SendSavePartNowL();
        void SendMediaNowL();
        void ClearUpload();
        void ClearSave();
        void SendRpcL( const TDesC8& aWrapped );
        void FlushPendingL();
        void HandleDialogsL( const TUint8* aP, TInt aLen );
        void HandleContactsL( const TUint8* aP, TInt aLen );
        void HandleHistoryL( const TUint8* aP, TInt aLen );
        void HandleFileL( const TUint8* aP, TInt aLen );
        void HandleSentL( const TUint8* aP, TInt aLen );
        void BeginIapL();
        TInt SessionFileName( TFileName& aOut );
        TInt LoadSession();
        void SaveSession();
        void DropSavedSession();
        TBool IsDeadAuth( const TDesC8& aMsg ) const;
        TBool IsFileMigrate( const TDesC8& aMsg ) const;
        void WrapInitL( const TDesC8& aQuery, TDes8& aOut );
        TInt StartPbkdfL( const TDesC8& aPassword );
        void StartSrpL();
        TInt SrpStepL();
        TInt SrpOnExpDoneL();
        TInt SrpBeginSharedL();
        void BeginSrpExp( const TBn& aBase, const TUint32* aExp, TInt aLimbs );
        void FinishSrpSendL();
        void ReportSrpProgressL();
        void ClearSrp();
        static TInt PbkdfCb( TAny* aPtr );
        TInt PbkdfStep();
        void ReportPbkdfProgressL();
        void StopPbkdf();
        void HandlePasswordL( const TUint8* aP, TInt aLen );
        void SendEncryptedL( const TDesC8& aMsg );
        void ReadMoreL();
        void EnsureInL( TInt aNeed );
        void HandleIncomingL();
        void HandleUnencryptedL( const TUint8* aBody, TInt aLen );
        void HandleEncryptedL( const TUint8* aPacket, TInt aLen );
        void DispatchInnerL( const TUint8* aP, TInt aLen );
        void HandlePqL( const TUint8* aP, TInt aLen );
        void HandleServerDhL( const TUint8* aP, TInt aLen );
        void HandleDhGenL( const TUint8* aP, TInt aLen );
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
        TBool iAuthed;
        TRpc iRpc;

        RFs iFs;
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
        TInt iDcId;
        TBuf8<24> iPhone;
        TBuf8<64> iPhoneCodeHash;
        TBuf8<16> iCode;
        TBuf8<768> iLastRpc;
        TBool iHaveSrp;
        TUint32 iSrpG;
        TInt64 iSrpId;
        TBuf8<256> iSalt1;
        TBuf8<256> iSalt2;
        TBuf8<256> iSrpP;
        TBuf8<256> iSrpB;
        TBuf8<192> iPwdUtf;
        TInt iSrpTries;

        TInt64 iPeerId;
        TInt iPeerKind;
        TInt64 iPeerHash;
        TBuf8<400> iSendUtf;
        TInt iFileMsgId;
        TInt64 iFileId;
        TInt64 iFileHash;
        TBuf8<96> iFileRef;
        TBool iFilePhoto;
        TRpc iPendRpc;
        TBool iSaveFull;
        TBool iSaveOpen;
        TInt iSaveOff;
        TFileName iSavePath;
        RFile iSaveFile;
        HBufC8* iUpload;
        TInt64 iUploadId;
        TInt iUploadPart;
        TInt iUploadParts;
        TBool iUploadPhoto;
        TBuf8<32> iUploadName;
        TBuf8<32> iUploadMd5;

        TBuf8<16384> iOut;
        HBufC8* iIn;
        TPtr8 iRead;
        TInt iHave;
        TBool iResume;
        TInt iResumeTries;
        TBool iSwapDcAddr;
        TBool iTriedAddrSwap;

        CIdle* iPbkdfIdle;
        TUint8 iPbkdfPass[ 32 ];
        TUint8 iPbkdfU[ 64 ];
        TUint8 iPbkdfT[ 64 ];
        TInt iPbkdfN;
        TInt iPbkdfLastPct;

        struct TSrpWork
            {
            TInt iStage;
            TBn iP;
            TBn iG;
            TBn iX;
            TBn iGx;
            TBn iA;
            TBn iAexp;
            TBn iAcc;
            TBn iBaseM;
            TBn iU;
            TBn iB;
            TBn iK;
            TBn iS;
            TUint32 iN0inv;
            TUint32 iExp[ 80 ];
            TInt iExpLimbs;
            TInt iLimb;
            TInt iBit;
            TInt iBitsDone;
            TInt iBitsTotal;
            TUint8 iAp[ 256 ];
            };
        TSrpWork* iSrp;
    };

#endif
