#include <random.h>

#include "SymgramSession.h"
#include "SymgramApi.h"

namespace
    {
    const TUint32 KDc2 = INET_ADDR( 149, 154, 167, 51 );
    const TUint KDcPort = 443;
    const TUint32 KReqPqMulti = 0xbe7e8ef1u;
    const TUint32 KResPq = 0x05162463u;
    const TUint32 KReqDhParams = 0xd712e4beu;
    const TUint32 KServerDhOk = 0xd0e8075cu;
    const TUint32 KPqInnerDc = 0xa9f55f95u;
    const TUint32 KServerDhInner = 0xb5890dbau;
    const TUint32 KSetClientDh = 0xf5045f1fu;
    const TUint32 KClientDhInner = 0x6643b654u;
    const TUint32 KDhGenOk = 0x3bcbf734u;
    const TUint32 KInvokeWithLayer = 0xda9b0d0du;
    const TUint32 KInitConnection = 0xc1cd5ea9u;
    const TUint32 KSendCode = 0xa677244fu;
    const TUint32 KCodeSettings = 0xad253d78u;
    const TUint32 KSignIn = 0x8d52a951u;
    const TUint32 KSentCode = 0x5e002502u;
    const TUint32 KRpcResult = 0xf35c6d01u;
    const TUint32 KRpcError = 0x2144ca19u;
    const TUint32 KMsgContainer = 0x73f1f8dcu;
    const TUint32 KNewSession = 0x9ec20908u;
    const TUint32 KBadSalt = 0xedab447bu;
    const TUint32 KGzipPacked = 0x3072cfa1u;
    const TUint32 KAuthOk = 0x2ea2c0d4u;
    const TUint32 KAuthOkOld = 0xcd050916u;
    const TUint32 KGetPassword = 0x548a30f5u;
    const TUint32 KAccountPwd = 0x957b50fbu;
    const TUint32 KAccountPwdOld = 0x185b184fu;
    const TUint32 KAccountPwdSrp = 0xad2641f8u;
    const TUint32 KKdfAlgo = 0x3a912d4au;
    const TUint32 KCheckPassword = 0xd18b4d16u;
    const TUint32 KInputSrp = 0xd27ff082u;
    const TInt KDcId = 2;
    const TInt KLayer = 158;

    TUint32 GetU32( const TUint8* aP )
        {
        return (TUint32)aP[ 0 ]
             | ( (TUint32)aP[ 1 ] << 8 )
             | ( (TUint32)aP[ 2 ] << 16 )
             | ( (TUint32)aP[ 3 ] << 24 );
        }

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

    TUint64 GetU64( const TUint8* aP )
        {
        return (TUint64)GetU32( aP ) | ( (TUint64)GetU32( aP + 4 ) << 32 );
        }

    TInt32 GetI32( const TUint8* aP )
        {
        return (TInt32)GetU32( aP );
        }

    void PutBe( TDes8& aOut, TUint64 aValue, TInt aBytes )
        {
        for ( TInt i = aBytes - 1; i >= 0; i-- )
            {
            aOut.Append( (TUint8)( aValue >> ( 8 * i ) ) );
            }
        }

    void ToUnicode( const TDesC8& aIn, TDes& aOut )
        {
        aOut.Zero();
        const TInt n = aIn.Length() < aOut.MaxLength() ? aIn.Length() : aOut.MaxLength();
        for ( TInt i = 0; i < n; i++ )
            {
            aOut.Append( (TText)aIn[ i ] );
            }
        }

    void PadBe( TUint8* aDst, TInt aDstLen, const TUint8* aSrc, TInt aSrcLen )
        {
        Mem::FillZ( aDst, aDstLen );
        if ( aSrcLen > aDstLen )
            {
            aSrc += aSrcLen - aDstLen;
            aSrcLen = aDstLen;
            }
        Mem::Copy( aDst + aDstLen - aSrcLen, aSrc, aSrcLen );
        }

    _LIT8( KDevice, "Nokia N95 8GB" );
    _LIT8( KSystem, "Symbian OS 9.2" );
    _LIT8( KAppVer, "0.1.0" );
    _LIT8( KLang, "ru" );
    _LIT8( KLangPack, "" );

    _LIT( KStatusIap, "Точка доступа..." );
    _LIT( KStatusConn, "Соединение..." );
    _LIT( KStatusSend, "Запрос к Telegram..." );
    _LIT( KStatusWait, "Ответ Telegram..." );
    _LIT( KStatusKey, "Ключ сессии..." );
    _LIT( KStatusCode, "Запрос кода..." );
    _LIT( KStatusPwd, "Проверка пароля..." );
    _LIT( KNeedApi, "Впишите api_id и api_hash в SymgramApi.h" );
    _LIT8( KNeedPwdErr, "SESSION_PASSWORD_NEEDED" );
    _LIT8( KBadPwdErr, "PASSWORD_HASH_INVALID" );
    _LIT8( KSrpIdErr, "SRP_ID_INVALID" );
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
      iBusy( EFalse ),
      iSentAbridged( EFalse ),
      iRsaN( KTelegramRsaN ),
      iAuthKeyId( 0 ),
      iSalt( 0 ),
      iSessionId( 0 ),
      iLastMsgId( 0 ),
      iSeq( 0 ),
      iTimeOffset( 0 ),
      iPhase( 0 ),
      iHaveSrp( EFalse ),
      iSrpG( 0 ),
      iSrpId( 0 ),
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
    return iBusy;
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
    iBusy = EFalse;
    TRAP_IGNORE( iObserver.SessionFailedL( aError ) );
    return KErrNone;
    }

void CSymgramSession::FailL( TInt aError )
    {
    CloseSocket();
    iState = EIdle;
    iBusy = EFalse;
    iObserver.SessionFailedL( aError );
    }

void CSymgramSession::FailTextL( const TDesC& aText )
    {
    CloseSocket();
    iState = EIdle;
    iBusy = EFalse;
    iObserver.SessionErrorL( aText );
    }

void CSymgramSession::RpcFailL( const TDesC8& aMsg )
    {
    TBuf<64> text;
    ToUnicode( aMsg, text );
    iBusy = EFalse;
    iState = EIdle;
    iObserver.SessionErrorL( text );
    }

void CSymgramSession::ConnectL( const TDesC8& aPhone )
    {
    if ( iBusy )
        {
        return;
        }
    if ( KSymgramApiId <= 0 || KSymgramApiHash().Length() == 0 )
        {
        iObserver.SessionErrorL( KNeedApi );
        return;
        }

    Cancel();
    CloseSocket();
    iConn.Close();
    iHave = 0;
    iPhase = 0;
    iBusy = ETrue;
    iSentAbridged = EFalse;
    iSeq = 0;
    iLastMsgId = 0;
    iTimeOffset = 0;
    iIn.Zero();
    iOut.Zero();
    iPhone.Copy( aPhone );
    iPhoneCodeHash.Zero();
    iHaveSrp = EFalse;

    User::LeaveIfError( iConn.Open( iServ ) );

    iPref = TCommDbConnPref();
    iPref.SetDialogPreference( ECommDbDialogPrefPrompt );
    iPref.SetDirection( ECommDbConnectionDirectionOutgoing );

    iObserver.SessionStatusL( KStatusIap );
    iState = EStarting;
    iConn.Start( iPref, iStatus );
    SetActive();
    }

void CSymgramSession::SubmitCodeL( const TDesC8& aCode )
    {
    if ( iBusy || iPhase != 3 || iPhoneCodeHash.Length() == 0 )
        {
        return;
        }
    if ( IsActive() )
        {
        Cancel();
        }
    iCode.Copy( aCode );
    iBusy = ETrue;
    iLastRpc.Zero();
    SendSignInL();
    }

void CSymgramSession::SubmitPasswordL( const TDesC8& aPassword )
    {
    if ( iBusy || iPhase != 3 || !iHaveSrp )
        {
        return;
        }
    if ( IsActive() )
        {
        Cancel();
        }
    iBusy = ETrue;
    iObserver.SessionStatusL( KStatusPwd );
    TBuf8<256> A;
    TBuf8<32> M1;
    TInt err = ComputeSrpL( aPassword, A, M1 );
    if ( err != KErrNone )
        {
        FailL( err );
        return;
        }

    TBuf8<384> query;
    PutU32( query, KCheckPassword );
    PutU32( query, KInputSrp );
    PutU64( query, (TUint64)iSrpId );
    TlAppendBytes( query, A.Ptr(), A.Length() );
    TlAppendBytes( query, M1.Ptr(), M1.Length() );

    TBuf8<512> wrapped;
    WrapInitL( query, wrapped );
    iLastRpc.Copy( wrapped );
    SendEncryptedL( wrapped );
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
            if ( iHaveSrp )
                {
                iObserver.SessionStatusL( KStatusPwd );
                }
            else if ( iPhoneCodeHash.Length() )
                {
                iObserver.SessionStatusL( KStatusWait );
                }
            else
                {
                iObserver.SessionStatusL( KStatusCode );
                }
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

TInt CSymgramSession::UnixNow() const
    {
    TTime now;
    now.UniversalTime();
    _LIT( KEpoch, "19700000:000000.000000" );
    TTime epoch;
    epoch.Set( KEpoch );
    const TInt64 us = now.MicroSecondsFrom( epoch ).Int64();
    if ( us <= 0 )
        {
        return 0;
        }
    return (TInt)( us / 1000000 );
    }

TUint64 CSymgramSession::MessageId()
    {
    TUint32 sec = (TUint32)( UnixNow() + iTimeOffset );
    TTime now;
    now.UniversalTime();
    _LIT( KEpoch, "19700000:000000.000000" );
    TTime epoch;
    epoch.Set( KEpoch );
    const TInt64 us = now.MicroSecondsFrom( epoch ).Int64();
    TUint32 frac = 0;
    if ( us > 0 )
        {
        frac = (TUint32)( ( ( us % 1000000 ) << 12 ) / 15625 );
        }
    frac &= ~3u;
    TUint64 id = ( (TUint64)sec << 32 ) | frac;
    if ( id <= iLastMsgId )
        {
        id = iLastMsgId + 4;
        }
    iLastMsgId = id;
    return id;
    }

TUint32 CSymgramSession::SeqNo()
    {
    const TUint32 seq = (TUint32)( iSeq * 2 + 1 );
    iSeq++;
    return seq;
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
    iObserver.SessionStatusL( KStatusSend );
    WritePacketL( payload );
    }

void CSymgramSession::WritePacketL( const TDesC8& aPayload )
    {
    iOut.Zero();
    if ( !iSentAbridged )
        {
        iOut.Append( (TUint8)0xef );
        iSentAbridged = ETrue;
        }
    const TInt words = aPayload.Length() / 4;
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
    iOut.Append( aPayload );

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
    for ( ;; )
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
        if ( iPhase < 3 )
            {
            if ( len < 24 )
                {
                FailL( KErrCorrupt );
                return;
                }
            const TUint32 bodyLen = GetU32( p + 16 );
            if ( bodyLen < 4 || (TInt)( 20 + bodyLen ) > len )
                {
                FailL( KErrCorrupt );
                return;
                }
            HandleUnencryptedL( p + 20, (TInt)bodyLen );
            }
        else
            {
            HandleEncryptedL( p, len );
            }

        const TInt left = iHave - need;
        if ( left > 0 )
            {
            TBuf8<4096> tmp;
            tmp.Copy( iIn.Mid( need, left ) );
            iIn.Copy( tmp );
            }
        iHave = left;
        iIn.SetLength( iHave );

        if ( iState == EWriting || iState == EIdle )
            {
            return;
            }
        }
    }

void CSymgramSession::HandleUnencryptedL( const TUint8* aBody, TInt aLen )
    {
    if ( iPhase == 0 )
        {
        HandlePqL( aBody, aLen );
        }
    else if ( iPhase == 1 )
        {
        HandleServerDhL( aBody, aLen );
        }
    else if ( iPhase == 2 )
        {
        HandleDhGenL( aBody, aLen );
        }
    else
        {
        FailL( KErrNotSupported );
        }
    }

void CSymgramSession::HandlePqL( const TUint8* aBody, TInt aLen )
    {
    if ( aLen < 36 || GetU32( aBody ) != KResPq )
        {
        FailL( KErrCorrupt );
        return;
        }
    if ( Mem::Compare( aBody + 4, 16, iNonce.Ptr(), 16 ) != 0 )
        {
        FailL( KErrCorrupt );
        return;
        }

    iServerNonce.Copy( aBody + 20, 16 );

    const TUint8* pqBytes = NULL;
    TInt pqLen = 0;
    const TInt pqSkip = TlReadBytes( aBody + 36, aLen - 36, pqBytes, pqLen );
    if ( pqSkip < 0 || pqLen < 1 || pqLen > 8 )
        {
        FailL( KErrCorrupt );
        return;
        }
    iPq.Zero();
    iPq.Append( pqBytes, pqLen );

    TUint64 pq = 0;
    TInt pi = 0;
    for ( pi = 0; pi < pqLen; pi++ )
        {
        pq = ( pq << 8 ) | pqBytes[ pi ];
        }
    TUint64 p = 0, q = 0;
    if ( !FactorPq( pq, p, q ) )
        {
        FailL( KErrNotFound );
        return;
        }

    iP.Zero();
    iQ.Zero();
    PutBe( iP, p, 4 );
    PutBe( iQ, q, 4 );

    const TUint8* v = aBody + 36 + pqSkip;
    TInt remain = aLen - 36 - pqSkip;
    if ( remain < 8 || GetU32( v ) != 0x1cb5c415u )
        {
        FailL( KErrCorrupt );
        return;
        }
    const TUint32 nfp = GetU32( v + 4 );
    if ( remain < (TInt)( 8 + 8 * nfp ) )
        {
        FailL( KErrCorrupt );
        return;
        }

    TBuf8<300> ser;
    TlAppendBytes( ser, KTelegramRsaN, 256 );
    const TUint8 exp[ 3 ] = { 0x01, 0x00, 0x01 };
    TlAppendBytes( ser, exp, 3 );
    TUint8 dig[ 20 ];
    Sha1( ser.Ptr(), ser.Length(), dig );
    iFingerprint = (TInt64)GetU64( dig + 12 );
    iRsaN = KTelegramRsaN;

    TBool found = EFalse;
    TUint32 fi = 0;
    for ( fi = 0; fi < nfp; fi++ )
        {
        const TInt64 fp = (TInt64)GetU64( v + 8 + 8 * fi );
        if ( fp == iFingerprint )
            {
            found = ETrue;
            break;
            }
        }
    if ( !found )
        {
        FailL( KErrNotSupported );
        return;
        }

    iObserver.SessionStatusL( KStatusKey );
    SendDhParamsL();
    }

TInt CSymgramSession::RsaPad( const TDesC8& aData, TDes8& aOut )
    {
    if ( aData.Length() > 144 )
        {
        return KErrOverflow;
        }

    TUint8 padded[ 192 ];
    Mem::Copy( padded, aData.Ptr(), aData.Length() );
    TPtr8 rest( padded + aData.Length(), 192 - aData.Length(), 192 - aData.Length() );
    rest.SetLength( 192 - aData.Length() );
    TRAPD( err, GenerateRandomBytesL( rest ) );
    if ( err != KErrNone )
        {
        return err;
        }

    TUint8 reversed[ 192 ];
    TInt ri = 0;
    for ( ri = 0; ri < 192; ri++ )
        {
        reversed[ ri ] = padded[ 191 - ri ];
        }

    TUint8 tempKey[ 32 ];
    TPtr8 tk( tempKey, 32, 32 );
    tk.SetLength( 32 );
    TRAP( err, GenerateRandomBytesL( tk ) );
    if ( err != KErrNone )
        {
        return err;
        }

    TUint8 cat[ 224 ];
    Mem::Copy( cat, tempKey, 32 );
    Mem::Copy( cat + 32, padded, 192 );
    TUint8 hash[ 32 ];
    Sha256( cat, 224, hash );

    TUint8 withHash[ 224 ];
    Mem::Copy( withHash, reversed, 192 );
    Mem::Copy( withHash + 192, hash, 32 );

    TUint8 iv[ 32 ];
    Mem::FillZ( iv, 32 );
    TUint8 aesEnc[ 224 ];
    Aes256IgeEncrypt( withHash, aesEnc, 224, tempKey, iv );

    TUint8 hash2[ 32 ];
    Sha256( aesEnc, 224, hash2 );
    TUint8 keyAes[ 256 ];
    TInt xi = 0;
    for ( xi = 0; xi < 32; xi++ )
        {
        keyAes[ xi ] = (TUint8)( tempKey[ xi ] ^ hash2[ xi ] );
        }
    Mem::Copy( keyAes + 32, aesEnc, 224 );

    TBn x, n;
    x.FromBe( keyAes, 256 );
    n.FromBe( iRsaN, 256 );
    if ( x.Cmp( n ) >= 0 )
        {
        return KErrAbort;
        }

    TUint8 enc[ 256 ];
    BnRsaPublic( enc, keyAes, iRsaN );
    aOut.Zero();
    aOut.Append( enc, 256 );
    return KErrNone;
    }

void CSymgramSession::SendDhParamsL()
    {
    iNewNonce.SetLength( 32 );
    GenerateRandomBytesL( iNewNonce );

    TBuf8<160> inner;
    PutU32( inner, KPqInnerDc );
    TlAppendBytes( inner, iPq.Ptr(), iPq.Length() );
    TlAppendBytes( inner, iP.Ptr(), iP.Length() );
    TlAppendBytes( inner, iQ.Ptr(), iQ.Length() );
    inner.Append( iNonce );
    inner.Append( iServerNonce );
    inner.Append( iNewNonce );
    PutU32( inner, KDcId );

    TBuf8<256> encrypted;
    TInt err = KErrAbort;
    TInt tryNo = 0;
    for ( tryNo = 0; tryNo < 8 && err != KErrNone; tryNo++ )
        {
        err = RsaPad( inner, encrypted );
        }
    if ( err != KErrNone )
        {
        FailL( err );
        return;
        }

    TBuf8<400> body;
    PutU32( body, KReqDhParams );
    body.Append( iNonce );
    body.Append( iServerNonce );
    TlAppendBytes( body, iP.Ptr(), iP.Length() );
    TlAppendBytes( body, iQ.Ptr(), iQ.Length() );
    const TUint64 fp = (TUint64)iFingerprint;
    PutU32( body, (TUint32)fp );
    PutU32( body, (TUint32)( fp >> 32 ) );
    TlAppendBytes( body, encrypted.Ptr(), encrypted.Length() );

    TBuf8<512> payload;
    BuildUnencryptedL( body, payload );
    iPhase = 1;
    WritePacketL( payload );
    }

void CSymgramSession::TmpAes( TUint8 aKey[ 32 ], TUint8 aIv[ 32 ] ) const
    {
    TUint8 buf[ 64 ];
    TUint8 hash1[ 20 ];
    TUint8 hash2[ 20 ];
    TUint8 hash3[ 20 ];

    Mem::Copy( buf, iNewNonce.Ptr(), 32 );
    Mem::Copy( buf + 32, iServerNonce.Ptr(), 16 );
    Sha1( buf, 48, hash1 );

    Mem::Copy( buf, iServerNonce.Ptr(), 16 );
    Mem::Copy( buf + 16, iNewNonce.Ptr(), 32 );
    Sha1( buf, 48, hash2 );

    Mem::Copy( buf, iNewNonce.Ptr(), 32 );
    Mem::Copy( buf + 32, iNewNonce.Ptr(), 32 );
    Sha1( buf, 64, hash3 );

    Mem::Copy( aKey, hash1, 20 );
    Mem::Copy( aKey + 20, hash2, 12 );
    Mem::Copy( aIv, hash2 + 12, 8 );
    Mem::Copy( aIv + 8, hash3, 20 );
    Mem::Copy( aIv + 28, iNewNonce.Ptr(), 4 );
    }

void CSymgramSession::HandleServerDhL( const TUint8* aBody, TInt aLen )
    {
    if ( aLen < 36 || GetU32( aBody ) != KServerDhOk )
        {
        FailL( KErrCorrupt );
        return;
        }
    if ( Mem::Compare( aBody + 4, 16, iNonce.Ptr(), 16 ) != 0 ||
         Mem::Compare( aBody + 20, 16, iServerNonce.Ptr(), 16 ) != 0 )
        {
        FailL( KErrCorrupt );
        return;
        }

    const TUint8* enc = NULL;
    TInt encLen = 0;
    if ( TlReadBytes( aBody + 36, aLen - 36, enc, encLen ) < 0 ||
         encLen < 32 || ( encLen & 15 ) != 0 )
        {
        FailL( KErrCorrupt );
        return;
        }

    iTmpKey.SetLength( 32 );
    iTmpIv.SetLength( 32 );
    TmpAes( const_cast<TUint8*>( iTmpKey.Ptr() ),
            const_cast<TUint8*>( iTmpIv.Ptr() ) );

    HBufC8* dec = HBufC8::NewLC( encLen );
    TPtr8 decp = dec->Des();
    decp.SetLength( encLen );
    Aes256IgeDecrypt( enc, const_cast<TUint8*>( decp.Ptr() ), encLen,
                      iTmpKey.Ptr(), iTmpIv.Ptr() );

    const TUint8* ans = decp.Ptr() + 20;
    const TInt ansMax = encLen - 20;
    if ( ansMax < 40 || GetU32( ans ) != KServerDhInner )
        {
        CleanupStack::PopAndDestroy( dec );
        FailL( KErrCorrupt );
        return;
        }
    if ( Mem::Compare( ans + 4, 16, iNonce.Ptr(), 16 ) != 0 ||
         Mem::Compare( ans + 20, 16, iServerNonce.Ptr(), 16 ) != 0 )
        {
        CleanupStack::PopAndDestroy( dec );
        FailL( KErrCorrupt );
        return;
        }

    const TUint32 g = GetU32( ans + 36 );
    const TUint8* prime = NULL;
    TInt primeLen = 0;
    TInt used = 40;
    TInt skip = TlReadBytes( ans + used, ansMax - used, prime, primeLen );
    if ( skip < 0 || primeLen < 64 || primeLen > 256 )
        {
        CleanupStack::PopAndDestroy( dec );
        FailL( KErrCorrupt );
        return;
        }
    used += skip;
    const TUint8* ga = NULL;
    TInt gaLen = 0;
    skip = TlReadBytes( ans + used, ansMax - used, ga, gaLen );
    if ( skip < 0 || gaLen < 64 || gaLen > 256 )
        {
        CleanupStack::PopAndDestroy( dec );
        FailL( KErrCorrupt );
        return;
        }
    used += skip;
    if ( ansMax - used < 4 )
        {
        CleanupStack::PopAndDestroy( dec );
        FailL( KErrCorrupt );
        return;
        }
    const TInt serverTime = GetI32( ans + used );
    used += 4;

    TUint8 check[ 20 ];
    Sha1( ans, used, check );
    if ( Mem::Compare( check, 20, decp.Ptr(), 20 ) != 0 )
        {
        CleanupStack::PopAndDestroy( dec );
        FailL( KErrCorrupt );
        return;
        }

    iTimeOffset = serverTime - UnixNow();

    TBn dhP, dhG, dhB, dhGa, dhGb, dhAk;
    dhP.FromBe( prime, primeLen );
    dhG.Zero();
    dhG.iD[ 0 ] = g;
    dhGa.FromBe( ga, gaLen );
    if ( dhGa.IsZero() || dhGa.Cmp( dhP ) >= 0 )
        {
        CleanupStack::PopAndDestroy( dec );
        FailL( KErrCorrupt );
        return;
        }

    TUint8 bbytes[ 256 ];
    TPtr8 bb( bbytes, 256, 256 );
    bb.SetLength( 256 );
    GenerateRandomBytesL( bb );
    dhB.FromBe( bbytes, 256 );

    BnModExp( dhGb, dhG, dhB, dhP );
    BnModExp( dhAk, dhGa, dhB, dhP );

    TUint8 akb[ 256 ];
    dhAk.ToBe( akb, 256 );
    iAuthKey.Copy( akb, 256 );

    TUint8 gb[ 256 ];
    dhGb.ToBe( gb, 256 );
    TInt goff = 0;
    while ( goff < 255 && gb[ goff ] == 0 )
        {
        goff++;
        }

    TBuf8<400> inner;
    PutU32( inner, KClientDhInner );
    inner.Append( iNonce );
    inner.Append( iServerNonce );
    PutU64( inner, 0 );
    TlAppendBytes( inner, gb + goff, 256 - goff );

    TUint8 innerHash[ 20 ];
    Sha1( inner.Ptr(), inner.Length(), innerHash );
    TBuf8<448> hashed;
    hashed.Append( innerHash, 20 );
    hashed.Append( inner );
    TInt pad = ( 16 - ( hashed.Length() % 16 ) ) % 16;
    if ( pad )
        {
        TBuf8<16> r;
        r.SetLength( pad );
        GenerateRandomBytesL( r );
        hashed.Append( r );
        }

    TBuf8<448> encInner;
    encInner.SetLength( hashed.Length() );
    Aes256IgeEncrypt( hashed.Ptr(), const_cast<TUint8*>( encInner.Ptr() ),
                      hashed.Length(), iTmpKey.Ptr(), iTmpIv.Ptr() );

    TBuf8<512> body;
    PutU32( body, KSetClientDh );
    body.Append( iNonce );
    body.Append( iServerNonce );
    TlAppendBytes( body, encInner.Ptr(), encInner.Length() );

    TBuf8<600> payload;
    BuildUnencryptedL( body, payload );
    CleanupStack::PopAndDestroy( dec );
    iPhase = 2;
    WritePacketL( payload );
    }

void CSymgramSession::HandleDhGenL( const TUint8* aBody, TInt aLen )
    {
    if ( aLen < 52 || GetU32( aBody ) != KDhGenOk )
        {
        FailL( KErrCorrupt );
        return;
        }
    if ( Mem::Compare( aBody + 4, 16, iNonce.Ptr(), 16 ) != 0 ||
         Mem::Compare( aBody + 20, 16, iServerNonce.Ptr(), 16 ) != 0 )
        {
        FailL( KErrCorrupt );
        return;
        }

    TUint8 akhash[ 20 ];
    Sha1( iAuthKey.Ptr(), iAuthKey.Length(), akhash );
    TUint8 nn[ 41 ];
    Mem::Copy( nn, iNewNonce.Ptr(), 32 );
    nn[ 32 ] = 1;
    Mem::Copy( nn + 33, akhash, 8 );
    TUint8 h[ 20 ];
    Sha1( nn, 41, h );
    if ( Mem::Compare( h + 4, 16, aBody + 36, 16 ) != 0 )
        {
        FailL( KErrCorrupt );
        return;
        }

    iAuthKeyId = GetU64( akhash + 12 );
    TUint8 salt[ 8 ];
    TInt si = 0;
    for ( si = 0; si < 8; si++ )
        {
        salt[ si ] = (TUint8)( iNewNonce[ si ] ^ iServerNonce[ si ] );
        }
    iSalt = GetU64( salt );

    TBuf8<8> sid;
    sid.SetLength( 8 );
    GenerateRandomBytesL( sid );
    iSessionId = GetU64( sid.Ptr() );
    iSeq = 0;
    iPhase = 3;

    if ( KSymgramApiId <= 0 || KSymgramApiHash().Length() == 0 )
        {
        iBusy = EFalse;
        iState = EIdle;
        iObserver.SessionErrorL( KNeedApi );
        return;
        }

    SendSendCodeL();
    }

void CSymgramSession::DeriveAes( TInt aX, const TUint8 aMsgKey[ 16 ],
                                 TUint8 aAesKey[ 32 ], TUint8 aAesIv[ 32 ] ) const
    {
    TUint8 buf[ 52 ];
    TUint8 shaA[ 32 ];
    TUint8 shaB[ 32 ];
    Mem::Copy( buf, aMsgKey, 16 );
    Mem::Copy( buf + 16, iAuthKey.Ptr() + aX, 36 );
    Sha256( buf, 52, shaA );
    Mem::Copy( buf, iAuthKey.Ptr() + 40 + aX, 36 );
    Mem::Copy( buf + 36, aMsgKey, 16 );
    Sha256( buf, 52, shaB );
    Mem::Copy( aAesKey, shaA, 8 );
    Mem::Copy( aAesKey + 8, shaB + 8, 16 );
    Mem::Copy( aAesKey + 24, shaA + 24, 8 );
    Mem::Copy( aAesIv, shaB, 8 );
    Mem::Copy( aAesIv + 8, shaA + 8, 16 );
    Mem::Copy( aAesIv + 24, shaB + 24, 8 );
    }

void CSymgramSession::SendEncryptedL( const TDesC8& aMsg )
    {
    TBuf8<1024> plain;
    PutU64( plain, iSalt );
    PutU64( plain, iSessionId );
    PutU64( plain, MessageId() );
    PutU32( plain, SeqNo() );
    PutU32( plain, aMsg.Length() );
    plain.Append( aMsg );

    TInt pad = 16 - ( plain.Length() % 16 );
    if ( pad < 12 )
        {
        pad += 16;
        }
    TBuf8<32> padding;
    padding.SetLength( pad );
    GenerateRandomBytesL( padding );
    plain.Append( padding );

    TBuf8<1088> mk;
    mk.Append( iAuthKey.Ptr() + 88, 32 );
    mk.Append( plain );
    TUint8 sha[ 32 ];
    Sha256( mk.Ptr(), mk.Length(), sha );
    TUint8 msgKey[ 16 ];
    Mem::Copy( msgKey, sha + 8, 16 );

    TUint8 aesKey[ 32 ];
    TUint8 aesIv[ 32 ];
    DeriveAes( 0, msgKey, aesKey, aesIv );

    TBuf8<1024> enc;
    enc.SetLength( plain.Length() );
    Aes256IgeEncrypt( plain.Ptr(), const_cast<TUint8*>( enc.Ptr() ),
                      plain.Length(), aesKey, aesIv );

    TBuf8<1100> packet;
    PutU64( packet, iAuthKeyId );
    packet.Append( msgKey, 16 );
    packet.Append( enc );
    WritePacketL( packet );
    }

void CSymgramSession::SendSendCodeL()
    {
    TBuf8<256> query;
    PutU32( query, KSendCode );
    TlAppendBytes( query, iPhone.Ptr(), iPhone.Length() );
    PutU32( query, (TUint32)KSymgramApiId );
    TlAppendBytes( query, KSymgramApiHash().Ptr(), KSymgramApiHash().Length() );
    PutU32( query, KCodeSettings );
    PutU32( query, 0 );

    TBuf8<384> wrapped;
    PutU32( wrapped, KInvokeWithLayer );
    PutU32( wrapped, KLayer );
    PutU32( wrapped, KInitConnection );
    PutU32( wrapped, 0 );
    PutU32( wrapped, (TUint32)KSymgramApiId );
    TlAppendBytes( wrapped, KDevice().Ptr(), KDevice().Length() );
    TlAppendBytes( wrapped, KSystem().Ptr(), KSystem().Length() );
    TlAppendBytes( wrapped, KAppVer().Ptr(), KAppVer().Length() );
    TlAppendBytes( wrapped, KLang().Ptr(), KLang().Length() );
    TlAppendBytes( wrapped, KLangPack().Ptr(), KLangPack().Length() );
    TlAppendBytes( wrapped, KLang().Ptr(), KLang().Length() );
    wrapped.Append( query );

    iLastRpc.Copy( wrapped );
    iObserver.SessionStatusL( KStatusCode );
    SendEncryptedL( wrapped );
    }

void CSymgramSession::SendSignInL()
    {
    TBuf8<192> query;
    PutU32( query, KSignIn );
    PutU32( query, 1 );
    TlAppendBytes( query, iPhone.Ptr(), iPhone.Length() );
    TlAppendBytes( query, iPhoneCodeHash.Ptr(), iPhoneCodeHash.Length() );
    TlAppendBytes( query, iCode.Ptr(), iCode.Length() );

    TBuf8<320> wrapped;
    PutU32( wrapped, KInvokeWithLayer );
    PutU32( wrapped, KLayer );
    PutU32( wrapped, KInitConnection );
    PutU32( wrapped, 0 );
    PutU32( wrapped, (TUint32)KSymgramApiId );
    TlAppendBytes( wrapped, KDevice().Ptr(), KDevice().Length() );
    TlAppendBytes( wrapped, KSystem().Ptr(), KSystem().Length() );
    TlAppendBytes( wrapped, KAppVer().Ptr(), KAppVer().Length() );
    TlAppendBytes( wrapped, KLang().Ptr(), KLang().Length() );
    TlAppendBytes( wrapped, KLangPack().Ptr(), KLangPack().Length() );
    TlAppendBytes( wrapped, KLang().Ptr(), KLang().Length() );
    wrapped.Append( query );

    iLastRpc.Copy( wrapped );
    SendEncryptedL( wrapped );
    }

void CSymgramSession::WrapInitL( const TDesC8& aQuery, TDes8& aOut )
    {
    aOut.Zero();
    PutU32( aOut, KInvokeWithLayer );
    PutU32( aOut, KLayer );
    PutU32( aOut, KInitConnection );
    PutU32( aOut, 0 );
    PutU32( aOut, (TUint32)KSymgramApiId );
    TlAppendBytes( aOut, KDevice().Ptr(), KDevice().Length() );
    TlAppendBytes( aOut, KSystem().Ptr(), KSystem().Length() );
    TlAppendBytes( aOut, KAppVer().Ptr(), KAppVer().Length() );
    TlAppendBytes( aOut, KLang().Ptr(), KLang().Length() );
    TlAppendBytes( aOut, KLangPack().Ptr(), KLangPack().Length() );
    TlAppendBytes( aOut, KLang().Ptr(), KLang().Length() );
    aOut.Append( aQuery );
    }

void CSymgramSession::SendGetPasswordL()
    {
    TBuf8<8> query;
    PutU32( query, KGetPassword );
    TBuf8<384> wrapped;
    WrapInitL( query, wrapped );
    iLastRpc.Copy( wrapped );
    iObserver.SessionStatusL( KStatusPwd );
    SendEncryptedL( wrapped );
    }

TInt CSymgramSession::ComputeSrpL( const TDesC8& aPassword, TDes8& aA, TDes8& aM1 )
    {
    const TInt pLen = iSrpP.Length();
    if ( pLen < 64 || pLen > 256 || iSrpB.Length() < 1 )
        {
        return KErrCorrupt;
        }

    TUint8 cat[ 900 ];
    TInt n = 0;
    Mem::Copy( cat + n, iSalt1.Ptr(), iSalt1.Length() );
    n += iSalt1.Length();
    Mem::Copy( cat + n, aPassword.Ptr(), aPassword.Length() );
    n += aPassword.Length();
    Mem::Copy( cat + n, iSalt1.Ptr(), iSalt1.Length() );
    n += iSalt1.Length();
    TUint8 hash1[ 32 ];
    Sha256( cat, n, hash1 );

    n = 0;
    Mem::Copy( cat + n, iSalt2.Ptr(), iSalt2.Length() );
    n += iSalt2.Length();
    Mem::Copy( cat + n, hash1, 32 );
    n += 32;
    Mem::Copy( cat + n, iSalt2.Ptr(), iSalt2.Length() );
    n += iSalt2.Length();
    TUint8 hash2[ 32 ];
    Sha256( cat, n, hash2 );

    TUint8 hash3[ 64 ];
    Pbkdf2HmacSha512( hash2, 32, iSalt1.Ptr(), iSalt1.Length(), 100000, hash3 );

    n = 0;
    Mem::Copy( cat + n, iSalt2.Ptr(), iSalt2.Length() );
    n += iSalt2.Length();
    Mem::Copy( cat + n, hash3, 64 );
    n += 64;
    Mem::Copy( cat + n, iSalt2.Ptr(), iSalt2.Length() );
    n += iSalt2.Length();
    TUint8 xbytes[ 32 ];
    Sha256( cat, n, xbytes );

    TBn p, g, x, gx, kbn, B, A, a, u, kgx, base, S;
    p.FromBe( iSrpP.Ptr(), pLen );
    g.Zero();
    g.iD[ 0 ] = iSrpG;
    x.FromBe( xbytes, 32 );
    BnModExp( gx, g, x, p );

    TUint8 gp[ 256 ];
    g.ToBe( gp, pLen );
    TUint8 pg[ 512 ];
    Mem::Copy( pg, iSrpP.Ptr(), pLen );
    Mem::Copy( pg + pLen, gp, pLen );
    TUint8 kbytes[ 32 ];
    Sha256( pg, pLen * 2, kbytes );
    kbn.FromBe( kbytes, 32 );

    TUint8 abytes[ 256 ];
    TPtr8 abuf( abytes, 256, 256 );
    abuf.SetLength( 256 );
    TRAPD( err, GenerateRandomBytesL( abuf ) );
    if ( err != KErrNone )
        {
        return err;
        }
    a.FromBe( abytes, 256 );
    BnModExp( A, g, a, p );

    TBn one;
    one.Zero();
    one.iD[ 0 ] = 1;
    if ( A.IsZero() || A.Cmp( one ) <= 0 || A.Cmp( p ) >= 0 )
        {
        return KErrCorrupt;
        }

    TUint8 Ap[ 256 ];
    TUint8 Bp[ 256 ];
    A.ToBe( Ap, pLen );
    PadBe( Bp, pLen, iSrpB.Ptr(), iSrpB.Length() );
    TUint8 au[ 512 ];
    Mem::Copy( au, Ap, pLen );
    Mem::Copy( au + pLen, Bp, pLen );
    TUint8 ubytes[ 32 ];
    Sha256( au, pLen * 2, ubytes );
    u.FromBe( ubytes, 32 );
    if ( u.IsZero() )
        {
        return KErrCorrupt;
        }

    B.FromBe( iSrpB.Ptr(), iSrpB.Length() );
    if ( B.IsZero() || B.Cmp( one ) <= 0 || B.Cmp( p ) >= 0 )
        {
        return KErrCorrupt;
        }

    BnMulMod( kgx, kbn, gx, p );
    BnSubMod( base, B, kgx, p );

    TUint32 ux[ 16 ];
    TInt i = 0;
    for ( i = 0; i < 16; i++ )
        {
        ux[ i ] = 0;
        }
    TInt ii = 0;
    for ( ii = 0; ii < 8; ii++ )
        {
        TUint64 c = 0;
        TInt jj = 0;
        for ( jj = 0; jj < 8; jj++ )
            {
            c += (TUint64)ux[ ii + jj ] + (TUint64)u.iD[ ii ] * x.iD[ jj ];
            ux[ ii + jj ] = (TUint32)c;
            c >>= 32;
            }
        ux[ ii + 8 ] += (TUint32)c;
        }

    TUint32 exp[ 80 ];
    for ( i = 0; i < 80; i++ )
        {
        exp[ i ] = ( i < KBnLimbs ) ? a.iD[ i ] : 0;
        }
    TUint64 carry = 0;
    for ( i = 0; i < 16; i++ )
        {
        carry += (TUint64)exp[ i ] + ux[ i ];
        exp[ i ] = (TUint32)carry;
        carry >>= 32;
        }
    for ( i = 16; i < 80 && carry; i++ )
        {
        carry += exp[ i ];
        exp[ i ] = (TUint32)carry;
        carry >>= 32;
        }

    BnModExpN( S, base, exp, 80, p );

    TUint8 Sp[ 256 ];
    S.ToBe( Sp, pLen );
    TUint8 Khash[ 32 ];
    Sha256( Sp, pLen, Khash );

    TUint8 hp[ 32 ];
    TUint8 hg[ 32 ];
    Sha256( iSrpP.Ptr(), pLen, hp );
    Sha256( gp, pLen, hg );
    TUint8 xh[ 32 ];
    for ( i = 0; i < 32; i++ )
        {
        xh[ i ] = (TUint8)( hp[ i ] ^ hg[ i ] );
        }
    TUint8 hs1[ 32 ];
    TUint8 hs2[ 32 ];
    Sha256( iSalt1.Ptr(), iSalt1.Length(), hs1 );
    Sha256( iSalt2.Ptr(), iSalt2.Length(), hs2 );

    TUint8 m1in[ 640 ];
    n = 0;
    Mem::Copy( m1in + n, xh, 32 );
    n += 32;
    Mem::Copy( m1in + n, hs1, 32 );
    n += 32;
    Mem::Copy( m1in + n, hs2, 32 );
    n += 32;
    Mem::Copy( m1in + n, Ap, pLen );
    n += pLen;
    Mem::Copy( m1in + n, Bp, pLen );
    n += pLen;
    Mem::Copy( m1in + n, Khash, 32 );
    n += 32;
    TUint8 m1[ 32 ];
    Sha256( m1in, n, m1 );

    aA.Zero();
    TInt off = 0;
    while ( off < pLen - 1 && Ap[ off ] == 0 )
        {
        off++;
        }
    aA.Append( Ap + off, pLen - off );
    aM1.Zero();
    aM1.Append( m1, 32 );
    return KErrNone;
    }

void CSymgramSession::HandlePasswordL( const TUint8* aP, TInt aLen )
    {
    if ( aLen < 12 )
        {
        FailL( KErrCorrupt );
        return;
        }
    const TUint32 flags = GetU32( aP + 4 );
    if ( ( flags & 4 ) == 0 )
        {
        _LIT( KNoPwd, "Нет облачного пароля" );
        FailTextL( KNoPwd );
        return;
        }
    const TUint8* q = aP + 8;
    TInt left = aLen - 8;
    if ( left < 4 )
        {
        FailL( KErrCorrupt );
        return;
        }
    const TUint32 algo = GetU32( q );
    if ( algo != KKdfAlgo )
        {
        TBuf<48> text;
        _LIT( KAlgo, "Алгоритм " );
        text.Copy( KAlgo );
        text.AppendNum( (TInt64)algo, EHex );
        FailTextL( text );
        return;
        }
    q += 4;
    left -= 4;
    const TUint8* s1 = NULL;
    TInt n1 = 0;
    TInt skip = TlReadBytes( q, left, s1, n1 );
    if ( skip < 0 || n1 < 1 || n1 > iSalt1.MaxLength() )
        {
        FailL( KErrCorrupt );
        return;
        }
    iSalt1.Copy( s1, n1 );
    q += skip;
    left -= skip;
    const TUint8* s2 = NULL;
    TInt n2 = 0;
    skip = TlReadBytes( q, left, s2, n2 );
    if ( skip < 0 || n2 < 1 || n2 > iSalt2.MaxLength() )
        {
        FailL( KErrCorrupt );
        return;
        }
    iSalt2.Copy( s2, n2 );
    q += skip;
    left -= skip;
    if ( left < 4 )
        {
        FailL( KErrCorrupt );
        return;
        }
    iSrpG = GetU32( q );
    q += 4;
    left -= 4;
    const TUint8* pp = NULL;
    TInt np = 0;
    skip = TlReadBytes( q, left, pp, np );
    if ( skip < 0 || np < 64 || np > iSrpP.MaxLength() )
        {
        FailL( KErrCorrupt );
        return;
        }
    iSrpP.Copy( pp, np );
    q += skip;
    left -= skip;
    const TUint8* bb = NULL;
    TInt nb = 0;
    skip = TlReadBytes( q, left, bb, nb );
    if ( skip < 0 || nb < 1 || nb > iSrpB.MaxLength() )
        {
        FailL( KErrCorrupt );
        return;
        }
    iSrpB.Copy( bb, nb );
    q += skip;
    left -= skip;
    if ( left < 8 )
        {
        FailL( KErrCorrupt );
        return;
        }
    iSrpId = (TInt64)GetU64( q );

    TBuf<40> hint;
    if ( ( flags & 8 ) != 0 )
        {
        q += 8;
        left -= 8;
        const TUint8* hs = NULL;
        TInt hn = 0;
        if ( left > 0 && TlReadBytes( q, left, hs, hn ) >= 0 && hn > 0 )
            {
            TPtrC8 h8( hs, hn );
            ToUnicode( h8, hint );
            }
        }

    iHaveSrp = ETrue;
    iBusy = EFalse;
    iState = EIdle;
    iObserver.SessionPasswordNeededL( hint );
    }

void CSymgramSession::HandleEncryptedL( const TUint8* aPacket, TInt aLen )
    {
    if ( aLen < 24 + 16 )
        {
        FailL( KErrCorrupt );
        return;
        }
    if ( GetU64( aPacket ) != iAuthKeyId )
        {
        FailL( KErrCorrupt );
        return;
        }
    const TUint8* msgKey = aPacket + 8;
    const TUint8* enc = aPacket + 24;
    const TInt encLen = aLen - 24;
    if ( encLen <= 0 || ( encLen & 15 ) != 0 )
        {
        FailL( KErrCorrupt );
        return;
        }

    TUint8 aesKey[ 32 ];
    TUint8 aesIv[ 32 ];
    DeriveAes( 8, msgKey, aesKey, aesIv );

    HBufC8* plain = HBufC8::NewLC( encLen );
    TPtr8 pp = plain->Des();
    pp.SetLength( encLen );
    Aes256IgeDecrypt( enc, const_cast<TUint8*>( pp.Ptr() ), encLen, aesKey, aesIv );

    HBufC8* mkbuf = HBufC8::NewLC( 32 + encLen );
    TPtr8 mkp = mkbuf->Des();
    mkp.Append( iAuthKey.Ptr() + 96, 32 );
    mkp.Append( *plain );
    TUint8 sha[ 32 ];
    Sha256( mkp.Ptr(), mkp.Length(), sha );
    CleanupStack::PopAndDestroy( mkbuf );
    if ( Mem::Compare( sha + 8, 16, msgKey, 16 ) != 0 )
        {
        CleanupStack::PopAndDestroy( plain );
        FailL( KErrCorrupt );
        return;
        }

    if ( encLen < 32 )
        {
        CleanupStack::PopAndDestroy( plain );
        FailL( KErrCorrupt );
        return;
        }
    iSalt = GetU64( pp.Ptr() );
    const TUint32 msgLen = GetU32( pp.Ptr() + 28 );
    if ( msgLen < 4 || (TInt)( 32 + msgLen ) > encLen )
        {
        CleanupStack::PopAndDestroy( plain );
        FailL( KErrCorrupt );
        return;
        }
    DispatchInnerL( pp.Ptr() + 32, (TInt)msgLen );
    CleanupStack::PopAndDestroy( plain );
    }

void CSymgramSession::DispatchInnerL( const TUint8* aP, TInt aLen )
    {
    if ( aLen < 4 )
        {
        FailL( KErrCorrupt );
        return;
        }
    const TUint32 c = GetU32( aP );
    if ( c == KMsgContainer )
        {
        if ( aLen < 8 )
            {
            FailL( KErrCorrupt );
            return;
            }
        const TUint32 n = GetU32( aP + 4 );
        TInt o = 8;
        TUint32 mi = 0;
        for ( mi = 0; mi < n; mi++ )
            {
            if ( o + 16 > aLen )
                {
                FailL( KErrCorrupt );
                return;
                }
            const TUint32 blen = GetU32( aP + o + 12 );
            o += 16;
            if ( o + (TInt)blen > aLen )
                {
                FailL( KErrCorrupt );
                return;
                }
            DispatchInnerL( aP + o, (TInt)blen );
            o += ( (TInt)blen + 3 ) & ~3;
            if ( iState == EWriting || iState == EIdle )
                {
                return;
                }
            }
        return;
        }
    if ( c == KGzipPacked )
        {
        HBufC8* raw = UnzipPackedLC( aP, aLen );
        if ( !raw )
            {
            return;
            }
        DispatchInnerL( raw->Ptr(), raw->Length() );
        CleanupStack::PopAndDestroy( raw );
        return;
        }
    if ( c == KNewSession )
        {
        if ( aLen >= 28 )
            {
            iSalt = GetU64( aP + 20 );
            }
        return;
        }
    if ( c == KBadSalt )
        {
        if ( aLen >= 28 )
            {
            iSalt = GetU64( aP + 20 );
            }
        if ( iLastRpc.Length() > 0 )
            {
            SendEncryptedL( iLastRpc );
            }
        return;
        }
    if ( c == KRpcResult )
        {
        HandleRpcResultL( aP + 4, aLen - 4 );
        return;
        }
    }

HBufC8* CSymgramSession::UnzipPackedLC( const TUint8* aObj, TInt aLen )
    {
    const TUint8* packed = NULL;
    TInt plen = 0;
    if ( aLen < 5 || TlReadBytes( aObj + 4, aLen - 4, packed, plen ) < 0 )
        {
        FailL( KErrCorrupt );
        return NULL;
        }
    HBufC8* raw = HBufC8::NewLC( 16384 );
    TPtr8 rp = raw->Des();
    rp.SetLength( 16384 );
    TInt n = 0;
    if ( InflateTlGzip( packed, plen, const_cast<TUint8*>( rp.Ptr() ),
                        16384, n ) != KErrNone || n < 4 )
        {
        CleanupStack::PopAndDestroy( raw );
        _LIT( KGz, "Сжатый ответ" );
        FailTextL( KGz );
        return NULL;
        }
    rp.SetLength( n );
    return raw;
    }

void CSymgramSession::HandleRpcResultL( const TUint8* aP, TInt aLen )
    {
    if ( aLen < 12 )
        {
        FailL( KErrCorrupt );
        return;
        }
    const TUint8* p = aP + 8;
    TInt n = aLen - 8;
    if ( n < 4 )
        {
        FailL( KErrCorrupt );
        return;
        }
    const TUint32 c = GetU32( p );
    if ( c == KGzipPacked )
        {
        HBufC8* raw = UnzipPackedLC( p, n );
        if ( !raw )
            {
            return;
            }
        HBufC8* wrap = HBufC8::NewLC( 8 + raw->Length() );
        TPtr8 wp = wrap->Des();
        wp.FillZ( 8 );
        wp.Append( *raw );
        HandleRpcResultL( wp.Ptr(), wp.Length() );
        CleanupStack::PopAndDestroy( 2, raw );
        return;
        }
    if ( c == KRpcError )
        {
        if ( n < 8 )
            {
            FailL( KErrCorrupt );
            return;
            }
        const TUint8* msg = NULL;
        TInt mlen = 0;
        if ( TlReadBytes( p + 8, n - 8, msg, mlen ) < 0 )
            {
            FailL( KErrCorrupt );
            return;
            }
        TPtrC8 text( msg, mlen );
        if ( text == KNeedPwdErr )
            {
            SendGetPasswordL();
            return;
            }
        if ( text == KSrpIdErr )
            {
            SendGetPasswordL();
            return;
            }
        RpcFailL( text );
        return;
        }
    if ( c == KSentCode )
        {
        if ( n < 12 )
            {
            FailL( KErrCorrupt );
            return;
            }
        TInt skip = 0;
        if ( SkipSentCodeType( p + 8, n - 8, skip ) != KErrNone )
            {
            FailL( KErrNotSupported );
            return;
            }
        const TUint8* hash = NULL;
        TInt hlen = 0;
        if ( TlReadBytes( p + 8 + skip, n - 8 - skip, hash, hlen ) < 0 ||
             hlen < 1 || hlen > iPhoneCodeHash.MaxLength() )
            {
            FailL( KErrCorrupt );
            return;
            }
        iPhoneCodeHash.Copy( hash, hlen );
        iBusy = EFalse;
        iState = EIdle;
        iObserver.SessionCodeSentL();
        return;
        }
    if ( c == KAuthOk || c == KAuthOkOld )
        {
        iBusy = EFalse;
        iState = EIdle;
        iObserver.SessionSignedInL();
        return;
        }
    if ( c == KAccountPwd || c == KAccountPwdOld || c == KAccountPwdSrp )
        {
        HandlePasswordL( p, n );
        return;
        }
    TBuf<48> text;
    _LIT( KUnk, "Ответ " );
    text.Copy( KUnk );
    text.AppendNum( (TInt64)c, EHex );
    FailTextL( text );
    }

TInt CSymgramSession::SkipSentCodeType( const TUint8* aP, TInt aRemain, TInt& aSkip )
    {
    if ( aRemain < 4 )
        {
        return KErrCorrupt;
        }
    const TUint32 c = GetU32( aP );
    TInt o = 4;
    if ( c == 0x3dbb5986u || c == 0xc000bba2u || c == 0x5353e5a7u )
        {
        if ( aRemain < 8 )
            {
            return KErrCorrupt;
            }
        o = 8;
        }
    else if ( c == 0xab5c020eu )
        {
        const TUint8* d = NULL;
        TInt n = 0;
        const TInt s = TlReadBytes( aP + 4, aRemain - 4, d, n );
        if ( s < 0 )
            {
            return KErrCorrupt;
            }
        o = 4 + s;
        }
    else if ( c == 0x0820c4d6u )
        {
        if ( aRemain < 8 )
            {
            return KErrCorrupt;
            }
        o = 8;
        const TUint8* d = NULL;
        TInt n = 0;
        const TInt s = TlReadBytes( aP + o, aRemain - o, d, n );
        if ( s < 0 )
            {
            return KErrCorrupt;
            }
        o += s;
        }
    else
        {
        return KErrNotSupported;
        }
    aSkip = o;
    return KErrNone;
    }
