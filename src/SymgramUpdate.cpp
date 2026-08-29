#include "SymgramUpdate.h"
#include "SymgramVersion.h"

#include <PathInfo.h>

namespace
    {
    const TInt KHostCount = 3;
    const TInt KMaxBody = 4096;
    const TInt KHttpPort = 80;

    _LIT( KDirName, "Symgram\\" );
    _LIT( KSisx, "*.sisx" );
    _LIT( KSis, "*.sis" );
    _LIT( KNeedle, "Symgram" );
    _LIT8( KUa, "Symgram-N95" );

    void AppendRoot( TDes& aOut, const TDesC& aRoot, const TDesC& aSub )
        {
        aOut.Zero();
        if ( aRoot.Length() + aSub.Length() > aOut.MaxLength() )
            {
            return;
            }
        aOut.Copy( aRoot );
        aOut.Append( aSub );
        }
    }

CSymgramUpdate* CSymgramUpdate::NewL( MSymgramUpdateObserver& aObserver )
    {
    CSymgramUpdate* self = new ( ELeave ) CSymgramUpdate( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

CSymgramUpdate::CSymgramUpdate( MSymgramUpdateObserver& aObserver )
    : CActive( EPriorityStandard ),
      iObserver( aObserver ),
      iState( EIdle ),
      iHost( 0 ),
      iRead( NULL, 0, 0 ),
      iBody( NULL ),
      iHave( 0 ),
      iPkgPack( 0 ),
      iRemotePack( 0 ),
      iRemoteNewer( EFalse ),
      iNetDone( EFalse )
    {
    }

void CSymgramUpdate::ConstructL()
    {
    CActiveScheduler::Add( this );
    User::LeaveIfError( iFs.Connect() );
    User::LeaveIfError( iServ.Connect() );
    iBody = HBufC8::NewL( KMaxBody );
    }

CSymgramUpdate::~CSymgramUpdate()
    {
    Cancel();
    CloseNet();
    delete iBody;
    iBody = NULL;
    iServ.Close();
    iFs.Close();
    }

TBool CSymgramUpdate::CanInstall() const
    {
    return iPkgPath.Length() > 0;
    }

TBool CSymgramUpdate::PackageNewer() const
    {
    return iPkgPath.Length() > 0 && iPkgPack > CurrentPack();
    }

TBool CSymgramUpdate::NetDone() const
    {
    return iNetDone;
    }

const TDesC& CSymgramUpdate::PackagePath() const
    {
    return iPkgPath;
    }

const TDesC& CSymgramUpdate::RemoteTag() const
    {
    return iRemoteTag;
    }

TBool CSymgramUpdate::RemoteIsNewer() const
    {
    return iRemoteNewer;
    }

TInt CSymgramUpdate::PackVer( TInt aMaj, TInt aMin, TInt aBld )
    {
    if ( aMaj < 0 )
        {
        aMaj = 0;
        }
    if ( aMin < 0 )
        {
        aMin = 0;
        }
    if ( aBld < 0 )
        {
        aBld = 0;
        }
    return aMaj * 1000000 + aMin * 1000 + aBld;
    }

TInt CSymgramUpdate::CurrentPack()
    {
    return PackVer( SYMGRAM_VERSION_MAJOR, SYMGRAM_VERSION_MINOR,
                    SYMGRAM_VERSION_BUILD );
    }

TBool CSymgramUpdate::ParseVer8( const TDesC8& aText, TInt& aMaj,
                                 TInt& aMin, TInt& aBld )
    {
    TInt i = 0;
    if ( aText.Length() == 0 )
        {
        return EFalse;
        }
    if ( aText[ 0 ] == 'v' || aText[ 0 ] == 'V' )
        {
        i = 1;
        }
    TInt part[ 3 ] = { 0, 0, 0 };
    TInt n = 0;
    TInt val = 0;
    TBool digit = EFalse;
    for ( ; i < aText.Length() && n < 3; i++ )
        {
        const TUint8 c = aText[ i ];
        if ( c >= '0' && c <= '9' )
            {
            digit = ETrue;
            val = val * 10 + ( c - '0' );
            if ( val > 999 )
                {
                return EFalse;
                }
            }
        else if ( c == '.' && digit )
            {
            part[ n++ ] = val;
            val = 0;
            digit = EFalse;
            }
        else
            {
            break;
            }
        }
    if ( digit && n < 3 )
        {
        part[ n++ ] = val;
        }
    if ( n != 3 )
        {
        return EFalse;
        }
    aMaj = part[ 0 ];
    aMin = part[ 1 ];
    aBld = part[ 2 ];
    return ETrue;
    }

TBool CSymgramUpdate::ParseVer16( const TDesC& aText, TInt& aMaj,
                                  TInt& aMin, TInt& aBld )
    {
    TBuf8<40> utf;
    TInt i = 0;
    for ( i = 0; i < aText.Length() && utf.Length() < utf.MaxLength(); i++ )
        {
        const TUint c = aText[ i ];
        if ( c < 0x80 )
            {
            utf.Append( (TUint8)c );
            }
        }
    return ParseVer8( utf, aMaj, aMin, aBld );
    }

TBool CSymgramUpdate::FindVer8( const TDesC8& aText, TInt& aMaj,
                                TInt& aMin, TInt& aBld )
    {
    _LIT8( KTag, "tag_name" );
    TInt at = aText.Find( KTag );
    if ( at >= 0 )
        {
        const TPtrC8 rest( aText.Mid( at ) );
        TInt i = 0;
        for ( i = 0; i < rest.Length(); i++ )
            {
            const TUint8 c = rest[ i ];
            if ( c == 'v' || c == 'V' || ( c >= '0' && c <= '9' ) )
                {
                if ( ParseVer8( rest.Mid( i ), aMaj, aMin, aBld ) )
                    {
                    return ETrue;
                    }
                }
            }
        }
    TInt i = 0;
    for ( i = 0; i < aText.Length(); i++ )
        {
        const TUint8 c = aText[ i ];
        if ( c == 'v' || c == 'V' || ( c >= '1' && c <= '9' ) )
            {
            if ( ParseVer8( aText.Mid( i ), aMaj, aMin, aBld ) )
                {
                return ETrue;
                }
            }
        }
    return EFalse;
    }

void CSymgramUpdate::ScanDir( const TDesC& aDir )
    {
    if ( aDir.Length() == 0 )
        {
        return;
        }
    TInt pass = 0;
    for ( pass = 0; pass < 2; pass++ )
        {
        const TDesC& wild = pass == 0 ? KSisx() : KSis();
        TFileName spec;
        if ( aDir.Length() + wild.Length() > spec.MaxLength() )
            {
            continue;
            }
        spec.Copy( aDir );
        spec.Append( wild );
        CDir* list = NULL;
        if ( iFs.GetDir( spec, KEntryAttNormal, ESortByName, list ) != KErrNone )
            {
            continue;
            }
        CleanupStack::PushL( list );
        TInt i = 0;
        for ( i = 0; i < list->Count(); i++ )
            {
            const TEntry& e = ( *list )[ i ];
            if ( e.IsDir() )
                {
                continue;
                }
            TFileName path;
            if ( aDir.Length() + e.iName.Length() > path.MaxLength() )
                {
                continue;
                }
            path.Copy( aDir );
            path.Append( e.iName );
            ConsiderFile( path, e.iName );
            }
        CleanupStack::PopAndDestroy( list );
        }
    }

void CSymgramUpdate::ConsiderFile( const TDesC& aPath, const TDesC& aName )
    {
    if ( aName.FindF( KNeedle ) < 0 )
        {
        return;
        }
    TInt maj = 0;
    TInt min = 0;
    TInt bld = 0;
    TInt pack = 0;
    TInt i = 0;
    for ( i = 0; i < aName.Length(); i++ )
        {
        const TText c = aName[ i ];
        if ( c == 'v' || c == 'V' || ( c >= '0' && c <= '9' ) )
            {
            if ( ParseVer16( aName.Mid( i ), maj, min, bld ) )
                {
                pack = PackVer( maj, min, bld );
                break;
                }
            }
        }
    if ( aPath.Length() > iPkgPath.MaxLength() )
        {
        return;
        }
    if ( pack > iPkgPack || ( pack == 0 && iPkgPath.Length() == 0 ) )
        {
        iPkgPath.Copy( aPath );
        iPkgPack = pack;
        }
    }

void CSymgramUpdate::ScanLocal()
    {
    TFileName dir;
    AppendRoot( dir, PathInfo::PhoneMemoryRootPath(), KDirName );
    ScanDir( dir );
    AppendRoot( dir, PathInfo::PhoneMemoryRootPath(), PathInfo::InstallsPath() );
    ScanDir( dir );
    AppendRoot( dir, PathInfo::MemoryCardRootPath(), KDirName );
    ScanDir( dir );
    AppendRoot( dir, PathInfo::MemoryCardRootPath(), PathInfo::InstallsPath() );
    ScanDir( dir );
    }

void CSymgramUpdate::PeekLocal()
    {
    iPkgPath.Zero();
    iPkgPack = 0;
    ScanLocal();
    }

void CSymgramUpdate::StartL()
    {
    if ( IsActive() )
        {
        return;
        }
    iRemoteTag.Zero();
    iRemotePack = 0;
    iRemoteNewer = EFalse;
    iNetDone = EFalse;
    iPkgPath.Zero();
    iPkgPack = 0;
    ScanLocal();
    _LIT( KWait, "Проверка GitHub..." );
    iObserver.UpdateStatusL( KWait );
    BeginNetL();
    }

void CSymgramUpdate::BeginNetL()
    {
    CloseNet();
    iHost = 0;
    iHave = 0;
    if ( iBody )
        {
        iBody->Des().Zero();
        }
    // A second RConnection::Start on S60 3rd FP1 panics if Telegram
    // already holds the IAP. Use the existing interface.
    ResolveNextL();
    }

void CSymgramUpdate::CloseNet()
    {
    iSocket.Close();
    iResolver.Close();
    iConn.Close();
    iState = EIdle;
    }

TInt CSymgramUpdate::OpenTcp()
    {
    iSocket.Close();
    return iSocket.Open( iServ, KAfInet, KSockStream, KProtocolInetTcp );
    }

void CSymgramUpdate::ResolveNextL()
    {
    iSocket.Close();
    iResolver.Close();
    if ( iHost >= KHostCount )
        {
        if ( iPkgPack > CurrentPack() )
            {
            _LIT( KLocal, "На диске есть пакет" );
            FinishL( KLocal );
            }
        else if ( iPkgPath.Length() > 0 )
            {
            _LIT( KHave, "Пакет на диске — поставьте" );
            FinishL( KHave );
            }
        else
            {
            _LIT( KNet, "Скачайте с GitHub на карту" );
            FinishL( KNet );
            }
        return;
        }
    TBuf<40> host;
    switch ( iHost )
        {
        case 0:
            {
            _LIT( K0, "cdn.jsdelivr.net" );
            host.Copy( K0 );
            break;
            }
        case 1:
            {
            _LIT( K1, "raw.githubusercontent.com" );
            host.Copy( K1 );
            break;
            }
        default:
            {
            _LIT( K2, "api.github.com" );
            host.Copy( K2 );
            break;
            }
        }
    TInt err = iResolver.Open( iServ, KAfInet, KProtocolInetUdp );
    if ( err != KErrNone )
        {
        err = iResolver.Open( iServ, KAfInet, KProtocolInetTcp );
        }
    if ( err != KErrNone )
        {
        iHost++;
        ResolveNextL();
        return;
        }
    iState = EResolving;
    iResolver.GetByName( host, iNameEntry, iStatus );
    SetActive();
    }

void CSymgramUpdate::ConnectL()
    {
    if ( OpenTcp() != KErrNone )
        {
        iHost++;
        ResolveNextL();
        return;
        }
    const TSockAddr& raw = iNameEntry().iAddr;
    if ( raw.Family() != KAfInet )
        {
        iHost++;
        ResolveNextL();
        return;
        }
    iAddr = TInetAddr::Cast( raw );
    iAddr.SetPort( KHttpPort );
    iState = EConnecting;
    iSocket.Connect( iAddr, iStatus );
    SetActive();
    }

void CSymgramUpdate::SendGetL()
    {
    TBuf8<64> host;
    TBuf8<96> path;
    switch ( iHost )
        {
        case 0:
            {
            _LIT8( KH, "cdn.jsdelivr.net" );
            _LIT8( KP, "/gh/servalmc/Telegram-N95@main/update/latest.txt" );
            host.Copy( KH );
            path.Copy( KP );
            break;
            }
        case 1:
            {
            _LIT8( KH, "raw.githubusercontent.com" );
            _LIT8( KP, "/servalmc/Telegram-N95/main/update/latest.txt" );
            host.Copy( KH );
            path.Copy( KP );
            break;
            }
        default:
            {
            _LIT8( KH, "api.github.com" );
            _LIT8( KP, "/repos/servalmc/Telegram-N95/releases/latest" );
            host.Copy( KH );
            path.Copy( KP );
            break;
            }
        }
    iReq.Zero();
    _LIT8( KGet, "GET " );
    _LIT8( KHttp, " HTTP/1.0\r\nHost: " );
    _LIT8( KHdr, "\r\nUser-Agent: " );
    _LIT8( KEnd, "\r\nAccept: */*\r\nConnection: close\r\n\r\n" );
    iReq.Append( KGet );
    iReq.Append( path );
    iReq.Append( KHttp );
    iReq.Append( host );
    iReq.Append( KHdr );
    iReq.Append( KUa );
    iReq.Append( KEnd );
    iHave = 0;
    iBody->Des().Zero();
    iState = EWriting;
    iSocket.Write( iReq, iStatus );
    SetActive();
    }

void CSymgramUpdate::ReadMoreL()
    {
    const TInt room = iBody->Des().MaxLength() - iHave;
    if ( room <= 0 )
        {
        HandleBodyL();
        return;
        }
    iRead.Set( const_cast<TUint8*>( iBody->Ptr() ) + iHave, room, 0 );
    iState = EReading;
    iSocket.RecvOneOrMore( iRead, 0, iStatus, iXfrLen );
    SetActive();
    }

void CSymgramUpdate::HandleBodyL()
    {
    TPtr8 body( iBody->Des() );
    body.SetLength( iHave );
    TInt maj = 0;
    TInt min = 0;
    TInt bld = 0;
    if ( FindVer8( body, maj, min, bld ) )
        {
        iRemotePack = PackVer( maj, min, bld );
        iRemoteTag.Zero();
        iRemoteTag.Append( 'v' );
        iRemoteTag.AppendNum( maj );
        iRemoteTag.Append( '.' );
        iRemoteTag.AppendNum( min );
        iRemoteTag.Append( '.' );
        iRemoteTag.AppendNum( bld );
        if ( iRemotePack > CurrentPack() )
            {
            iRemoteNewer = ETrue;
            }
        if ( iRemoteNewer )
            {
            if ( iPkgPath.Length() > 0 )
                {
                _LIT( KGo, "Есть " );
                TBuf<40> text;
                text.Copy( KGo );
                text.Append( iRemoteTag );
                _LIT( KTail, " — поставьте" );
                text.Append( KTail );
                FinishL( text );
                }
            else
                {
                _LIT( KDl, "Скачайте " );
                TBuf<48> text;
                text.Copy( KDl );
                text.Append( iRemoteTag );
                _LIT( KTail, " с GitHub" );
                text.Append( KTail );
                FinishL( text );
                }
            return;
            }
        _LIT( KOk, "Версия актуальная" );
        FinishL( KOk );
        return;
        }
    iHost++;
    ResolveNextL();
    }

void CSymgramUpdate::FinishL( const TDesC& aText )
    {
    iNetDone = ETrue;
    CloseNet();
    iObserver.UpdateStatusL( aText );
    iObserver.UpdateReadyL();
    }

void CSymgramUpdate::RunL()
    {
    const TInt err = iStatus.Int();
    if ( err != KErrNone )
        {
        if ( iState == EReading && iHave > 0 )
            {
            HandleBodyL();
            return;
            }
        iHost++;
        ResolveNextL();
        return;
        }
    switch ( iState )
        {
        case EResolving:
            ConnectL();
            break;
        case EConnecting:
            SendGetL();
            break;
        case EWriting:
            ReadMoreL();
            break;
        case EReading:
            iHave += iRead.Length();
            if ( iRead.Length() == 0 )
                {
                HandleBodyL();
                }
            else
                {
                ReadMoreL();
                }
            break;
        default:
            CloseNet();
            break;
        }
    }

void CSymgramUpdate::DoCancel()
    {
    switch ( iState )
        {
        case EResolving:
            iResolver.Cancel();
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

TInt CSymgramUpdate::RunError( TInt aError )
    {
    CloseNet();
    iNetDone = ETrue;
    TBuf<32> text;
    _LIT( KErr, "Обновление " );
    text.Copy( KErr );
    text.AppendNum( aError );
    TRAP_IGNORE( iObserver.UpdateStatusL( text ) );
    TRAP_IGNORE( iObserver.UpdateReadyL() );
    return KErrNone;
    }
