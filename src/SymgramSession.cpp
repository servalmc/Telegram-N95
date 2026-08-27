#include <random.h>

#include "SymgramSession.h"

namespace
    {
    const TUint32 KDc2 = INET_ADDR( 149, 154, 167, 51 );
    const TUint KDcPort = 443;
    const TUint32 KReqPqMulti = 0xbe7e8ef1u;
    const TUint32 KResPq = 0x05162463u;

    void PutU32( TDes8& aOut, TUint32 aValue )
        {
        aOut.Append( (TUint8)( aValue ) );
        aOut.Append( (TUint8)( aValue >> 8 ) );
        aOut.Append( (TUint8)( aValue >> 16 ) );
        aOut.Append( (TUint8)( aValue >> 24 ) );
        }

    void PutU64( TDes8& aOut, TUint64 aValue )
        {
        PutU32( aOut, (TUint32)aValue );
        PutU32( aOut, (TUint32)( aValue >> 32 ) );
        }

    TUint32 GetU32( const TUint8* aP )
        {
        return (TUint32)aP[ 0 ]
             | ( (TUint32)aP[ 1 ] << 8 )
             | ( (TUint32)aP[ 2 ] << 16 )
             | ( (TUint32)aP[ 3 ] << 24 );
        }

    _LIT( KStatusIap, "Точка доступа..." );
    _LIT( KStatusConn, "Соединение..." );
    _LIT( KStatusSend, "Запрос к Telegram..." );
    _LIT( KStatusWait, "Ответ Telegram..." );
    }

CSymgramSession* CSymgramSession::NewL( MSymgramSessionObserver& aObserver )
    {
    CSymgramSession* self = new ( ELeave ) CSymgramSession( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

CSymgramSession::CSymgramSession( MSymgramSessionObserver& aObserver )
    : CActive( EPriorityStandard ),
      iObserver( aObserver ),
      iState( EIdle ),
      iRead( NULL, 0, 0 ),
      iHave( 0 )
    {
    CActiveScheduler::Add( this );
    }

void CSymgramSession::ConstructL()
    {
    User::LeaveIfError( iServ.Connect() );
    }

CSymgramSession::~CSymgramSession()
    {
    Cancel();
    CloseSocket();
    iConn.Close();
    iServ.Close();
    }

TBool CSymgramSession::IsBusy() const
    {
    return iState != EIdle;
    }

void CSymgramSession::CloseSocket()
    {
    if ( iSocket.SubSessionHandle() )
        {
        iSocket.Close();
        }
    }

void CSymgramSession::DoCancel()
    {
    switch ( iState )
        {
        case EStarting:
            iConn.Close();
            break;
        case EConnecting:
        case EWriting:
        case EReading:
            iSocket.CancelAll();
            break;
        default:
            break;
        }
    iState = EIdle;
    }

TInt CSymgramSession::RunError( TInt aError )
    {
    iState = EIdle;
    TRAP_IGNORE( iObserver.SessionFailedL( aError ) );
    return KErrNone;
    }

void CSymgramSession::FailL( TInt aError )
    {
    CloseSocket();
    iState = EIdle;
    iObserver.SessionFailedL( aError );
    }

void CSymgramSession::ConnectL()
    {
    if ( iState != EIdle )
        {
        return;
        }

    CloseSocket();
    iConn.Close();
    iHave = 0;
    iIn.Zero();
    iOut.Zero();

    User::LeaveIfError( iConn.Open( iServ ) );

    iPref = TCommDbConnPref();
    iPref.SetDialogPreference( ECommDbDialogPrefPrompt );
    iPref.SetDirection( ECommDbConnectionDirectionOutgoing );

    iObserver.SessionStatusL( KStatusIap );
    iState = EStarting;
    iConn.Start( iPref, iStatus );
    SetActive();
    }

void CSymgramSession::RunL()
    {
    if ( iStatus.Int() != KErrNone )
        {
        FailL( iStatus.Int() );
        return;
        }

    switch ( iState )
        {
        case EStarting:
            {
            User::LeaveIfError( iSocket.Open( iServ, KAfInet, KSockStream,
                                              KProtocolInetTcp, iConn ) );
            iAddr.SetAddress( KDc2 );
            iAddr.SetPort( KDcPort );
            iObserver.SessionStatusL( KStatusConn );
            iState = EConnecting;
            iSocket.Connect( iAddr, iStatus );
            SetActive();
            break;
            }
        case EConnecting:
            SendPqL();
            break;
        case EWriting:
            iHave = 0;
            iIn.Zero();
            iObserver.SessionStatusL( KStatusWait );
            ReadMoreL();
            break;
        case EReading:
            iHave += iRead.Length();
            iIn.SetLength( iHave );
            HandleIncomingL();
            break;
        default:
            FailL( KErrGeneral );
            break;
        }
    }

TUint64 CSymgramSession::MessageId() const
    {
    TTime now;
    now.UniversalTime();
    _LIT( KEpoch, "19700000:000000.000000" );
    TTime epoch;
    epoch.Set( KEpoch );
    const TInt64 us = now.MicroSecondsFrom( epoch ).Int64();
    TUint32 sec = 0;
    TUint32 frac = 0;
    if ( us > 0 )
        {
        sec = (TUint32)( us / 1000000 );
        frac = (TUint32)( ( ( us % 1000000 ) << 12 ) / 15625 );
        }
    frac &= ~3u;
    return ( (TUint64)sec << 32 ) | frac;
    }

void CSymgramSession::BuildUnencryptedL( const TDesC8& aBody, TDes8& aOut )
    {
    aOut.Zero();
    PutU64( aOut, 0 );
    PutU64( aOut, MessageId() );
    PutU32( aOut, aBody.Length() );
    aOut.Append( aBody );
    }

void CSymgramSession::SendPqL()
    {
    iNonce.SetLength( 16 );
    GenerateRandomBytesL( iNonce );

    TBuf8<20> body;
    PutU32( body, KReqPqMulti );
    body.Append( iNonce );

    TBuf8<64> payload;
    BuildUnencryptedL( body, payload );

    iOut.Zero();
    iOut.Append( (TUint8)0xef );
    const TInt words = payload.Length() / 4;
    if ( words < 0x7f )
        {
        iOut.Append( (TUint8)words );
        }
    else
        {
        iOut.Append( (TUint8)0x7f );
        iOut.Append( (TUint8)words );
        iOut.Append( (TUint8)( words >> 8 ) );
        iOut.Append( (TUint8)( words >> 16 ) );
        }
    iOut.Append( payload );

    iObserver.SessionStatusL( KStatusSend );
    iState = EWriting;
    iSocket.Write( iOut, iStatus );
    SetActive();
    }

void CSymgramSession::ReadMoreL()
    {
    if ( iHave >= iIn.MaxLength() )
        {
        FailL( KErrOverflow );
        return;
        }

    iRead.Set( const_cast<TUint8*>( iIn.Ptr() ) + iHave,
               0,
               iIn.MaxLength() - iHave );
    iState = EReading;
    iSocket.RecvOneOrMore( iRead, 0, iStatus, iXfrLen );
    SetActive();
    }

void CSymgramSession::HandleIncomingL()
    {
    if ( iHave < 1 )
        {
        ReadMoreL();
        return;
        }

    TInt header = 1;
    TInt words = 0;
    const TUint8 first = iIn[ 0 ];
    if ( first < 0x7f )
        {
        words = first;
        }
    else if ( first == 0x7f )
        {
        if ( iHave < 4 )
            {
            ReadMoreL();
            return;
            }
        words = iIn[ 1 ]
              | ( iIn[ 2 ] << 8 )
              | ( iIn[ 3 ] << 16 );
        header = 4;
        }
    else
        {
        words = first;
        }

    const TInt need = header + words * 4;
    if ( need <= header || need > iIn.MaxLength() )
        {
        FailL( KErrCorrupt );
        return;
        }
    if ( iHave < need )
        {
        ReadMoreL();
        return;
        }

    const TUint8* p = iIn.Ptr() + header;
    const TInt len = words * 4;
    if ( len < 24 )
        {
        FailL( KErrCorrupt );
        return;
        }

    const TUint32 bodyLen = GetU32( p + 16 );
    if ( bodyLen < 20 || (TInt)( 20 + bodyLen ) > len )
        {
        FailL( KErrCorrupt );
        return;
        }

    const TUint8* body = p + 20;
    if ( GetU32( body ) != KResPq )
        {
        FailL( KErrCorrupt );
        return;
        }
    if ( Mem::Compare( body + 4, 16, iNonce.Ptr(), 16 ) != 0 )
        {
        FailL( KErrCorrupt );
        return;
        }

    CloseSocket();
    iState = EIdle;
    iObserver.SessionPqOkL();
    }
