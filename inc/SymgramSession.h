#ifndef __SYMGRAMSESSION_H__
#define __SYMGRAMSESSION_H__

#include <e32base.h>
#include <es_sock.h>
#include <in_sock.h>
#include <commdbconnpref.h>

class MSymgramSessionObserver
    {
    public:
        virtual void SessionStatusL( const TDesC& aText ) = 0;
        virtual void SessionFailedL( TInt aError ) = 0;
        virtual void SessionPqOkL() = 0;
    };

// Opens a packet data connection, connects to a Telegram DC and sends the
// unencrypted req_pq_multi handshake. Further steps (RSA, DH, auth.sendCode)
// need extra crypto and api_id; this class stops at a verified resPQ.
class CSymgramSession : public CActive
    {
    public:
        static CSymgramSession* NewL( MSymgramSessionObserver& aObserver );
        ~CSymgramSession();

        void ConnectL();
        TBool IsBusy() const;

    private:
        enum TState
            {
            EIdle,
            EStarting,
            EConnecting,
            EWriting,
            EReading
            };

        CSymgramSession( MSymgramSessionObserver& aObserver );
        void ConstructL();

        void RunL();
        void DoCancel();
        TInt RunError( TInt aError );

        void FailL( TInt aError );
        void CloseSocket();
        void SendPqL();
        void ReadMoreL();
        void HandleIncomingL();
        void BuildUnencryptedL( const TDesC8& aBody, TDes8& aOut );
        TUint64 MessageId() const;

    private:
        MSymgramSessionObserver& iObserver;
        TState iState;

        RSocketServ iServ;
        RConnection iConn;
        RSocket iSocket;
        TInetAddr iAddr;
        TCommDbConnPref iPref;
        TSockXfrLength iXfrLen;

        TBuf8<16> iNonce;
        TBuf8<512> iOut;
        TBuf8<512> iIn;
        TPtr8 iRead;
        TInt iHave;
    };

#endif
