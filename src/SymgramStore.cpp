#include "SymgramStore.h"

namespace
    {
    const TUint32 KChatsMagic = 0x31434853u;
    const TUint32 KContMagic  = 0x314B4353u;
    const TUint32 KMsgMagic   = 0x314D4853u;

    TUint32 GetU32( const TUint8* aP )
        {
        return (TUint32)aP[ 0 ]
             | ( (TUint32)aP[ 1 ] << 8 )
             | ( (TUint32)aP[ 2 ] << 16 )
             | ( (TUint32)aP[ 3 ] << 24 );
        }

    TUint64 GetU64( const TUint8* aP )
        {
        return (TUint64)GetU32( aP ) | ( (TUint64)GetU32( aP + 4 ) << 32 );
        }

    void PutU32( TDes8& aOut, TUint32 aValue )
        {
        aOut.Append( (TUint8)aValue );
        aOut.Append( (TUint8)( aValue >> 8 ) );
        aOut.Append( (TUint8)( aValue >> 16 ) );
        aOut.Append( (TUint8)( aValue >> 24 ) );
        }

    void PutU64( TDes8& aOut, TUint64 aValue )
        {
        PutU32( aOut, (TUint32)aValue );
        PutU32( aOut, (TUint32)( aValue >> 32 ) );
        }

    void PutI32( TDes8& aOut, TInt aValue )
        {
        PutU32( aOut, (TUint32)aValue );
        }

    void PutText( TDes8& aOut, const TDesC& aText, TInt aMax )
        {
        const TInt n = aText.Length() < aMax ? aText.Length() : aMax;
        aOut.Append( (TUint8)n );
        TInt i = 0;
        for ( i = 0; i < n; i++ )
            {
            const TUint c = aText[ i ];
            aOut.Append( (TUint8)c );
            aOut.Append( (TUint8)( c >> 8 ) );
            }
        }

    TInt ReadText( const TUint8* aP, TInt aN, TInt& aO, TDes& aOut, TInt aMax )
        {
        aOut.Zero();
        if ( aO >= aN )
            {
            return KErrCorrupt;
            }
        const TInt n = aP[ aO++ ];
        if ( n < 0 || n > aMax || aO + n * 2 > aN )
            {
            return KErrCorrupt;
            }
        TInt i = 0;
        for ( i = 0; i < n; i++ )
            {
            const TUint lo = aP[ aO++ ];
            const TUint hi = aP[ aO++ ];
            aOut.Append( (TText)( lo | ( hi << 8 ) ) );
            }
        return KErrNone;
        }
    }

CSymgramStore* CSymgramStore::NewL()
    {
    CSymgramStore* self = new ( ELeave ) CSymgramStore;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

CSymgramStore::CSymgramStore()
    {
    }

void CSymgramStore::ConstructL()
    {
    User::LeaveIfError( iFs.Connect() );
    }

CSymgramStore::~CSymgramStore()
    {
    iFs.Close();
    }

TInt CSymgramStore::PrivateFile( const TDesC& aName, TFileName& aOut )
    {
    RProcess proc;
    TParse parse;
    parse.Set( proc.FileName(), NULL, NULL );
    TInt drive = EDriveC;
    if ( parse.Drive().Length() > 0 )
        {
        TInt d = 0;
        if ( iFs.CharToDrive( parse.Drive()[ 0 ], d ) == KErrNone )
            {
            drive = d;
            }
        }
    iFs.CreatePrivatePath( drive );
    TFileName priv;
    iFs.PrivatePath( priv );
    aOut.Zero();
    aOut.Append( parse.Drive() );
    aOut.Append( priv );
    aOut.Append( aName );
    return KErrNone;
    }

void CSymgramStore::MsgFile( TInt64 aPeer, TFileName& aOut )
    {
    TBuf<28> name;
    _LIT( KFmt, "m%08x%08x.bin" );
    name.Format( KFmt, (TUint32)( aPeer >> 32 ), (TUint32)aPeer );
    PrivateFile( name, aOut );
    }

void CSymgramStore::LoadChatsL( RArray<TSymgramChat>& aChats )
    {
    TFileName fn;
    _LIT( KFn, "chats.bin" );
    if ( PrivateFile( KFn, fn ) != KErrNone )
        {
        return;
        }
    RFile file;
    if ( file.Open( iFs, fn, EFileRead | EFileStream | EFileShareReadersOnly )
         != KErrNone )
        {
        return;
        }
    TInt size = 0;
    file.Size( size );
    if ( size < 8 || size > 32768 )
        {
        file.Close();
        return;
        }
    HBufC8* raw = HBufC8::NewLC( size );
    TPtr8 p = raw->Des();
    if ( file.Read( p ) != KErrNone )
        {
        CleanupStack::PopAndDestroy( raw );
        file.Close();
        return;
        }
    file.Close();
    if ( p.Length() < 8 || GetU32( p.Ptr() ) != KChatsMagic )
        {
        CleanupStack::PopAndDestroy( raw );
        return;
        }
    const TInt n = (TInt)GetU32( p.Ptr() + 4 );
    TInt o = 8;
    TInt i = 0;
    for ( i = 0; i < n && i < 80; i++ )
        {
        if ( o + 28 > p.Length() )
            {
            break;
            }
        TSymgramChat chat;
        chat.iId = (TInt64)GetU64( p.Ptr() + o ); o += 8;
        chat.iHash = (TInt64)GetU64( p.Ptr() + o ); o += 8;
        chat.iPeerKind = (TInt)GetU32( p.Ptr() + o ); o += 4;
        chat.iUnread = (TInt)GetU32( p.Ptr() + o ); o += 4;
        chat.iDate = (TInt)GetU32( p.Ptr() + o ); o += 4;
        if ( ReadText( p.Ptr(), p.Length(), o, chat.iName, 40 ) != KErrNone ||
             ReadText( p.Ptr(), p.Length(), o, chat.iPreview, 80 ) != KErrNone ||
             ReadText( p.Ptr(), p.Length(), o, chat.iTime, 12 ) != KErrNone )
            {
            break;
            }
        aChats.Append( chat );
        }
    CleanupStack::PopAndDestroy( raw );
    }

void CSymgramStore::SaveChats( const RArray<TSymgramChat>& aChats )
    {
    TFileName fn;
    _LIT( KFn, "chats.bin" );
    if ( PrivateFile( KFn, fn ) != KErrNone )
        {
        return;
        }
    TInt n = aChats.Count();
    if ( n > 80 )
        {
        n = 80;
        }
    TInt cap = 8 + n * 400;
    HBufC8* raw = HBufC8::New( cap );
    if ( !raw )
        {
        return;
        }
    TPtr8 p = raw->Des();
    PutU32( p, KChatsMagic );
    PutU32( p, (TUint32)n );
    TInt i = 0;
    for ( i = 0; i < n; i++ )
        {
        const TSymgramChat& c = aChats[ i ];
        PutU64( p, (TUint64)c.iId );
        PutU64( p, (TUint64)c.iHash );
        PutI32( p, c.iPeerKind );
        PutI32( p, c.iUnread );
        PutI32( p, c.iDate );
        PutText( p, c.iName, 40 );
        PutText( p, c.iPreview, 80 );
        PutText( p, c.iTime, 12 );
        }
    RFile file;
    if ( file.Replace( iFs, fn, EFileWrite | EFileStream ) == KErrNone )
        {
        file.Write( p );
        file.Close();
        }
    delete raw;
    }

void CSymgramStore::LoadContactsL( RArray<TSymgramContact>& aContacts )
    {
    TFileName fn;
    _LIT( KFn, "contacts.bin" );
    if ( PrivateFile( KFn, fn ) != KErrNone )
        {
        return;
        }
    RFile file;
    if ( file.Open( iFs, fn, EFileRead | EFileStream | EFileShareReadersOnly )
         != KErrNone )
        {
        return;
        }
    TInt size = 0;
    file.Size( size );
    if ( size < 8 || size > 32768 )
        {
        file.Close();
        return;
        }
    HBufC8* raw = HBufC8::NewLC( size );
    TPtr8 p = raw->Des();
    if ( file.Read( p ) != KErrNone )
        {
        CleanupStack::PopAndDestroy( raw );
        file.Close();
        return;
        }
    file.Close();
    if ( p.Length() < 8 || GetU32( p.Ptr() ) != KContMagic )
        {
        CleanupStack::PopAndDestroy( raw );
        return;
        }
    const TInt n = (TInt)GetU32( p.Ptr() + 4 );
    TInt o = 8;
    TInt i = 0;
    for ( i = 0; i < n && i < 80; i++ )
        {
        if ( o + 16 > p.Length() )
            {
            break;
            }
        TSymgramContact one;
        one.iId = (TInt64)GetU64( p.Ptr() + o ); o += 8;
        one.iHash = (TInt64)GetU64( p.Ptr() + o ); o += 8;
        if ( ReadText( p.Ptr(), p.Length(), o, one.iName, 40 ) != KErrNone ||
             ReadText( p.Ptr(), p.Length(), o, one.iPhone, 24 ) != KErrNone )
            {
            break;
            }
        aContacts.Append( one );
        }
    CleanupStack::PopAndDestroy( raw );
    }

void CSymgramStore::SaveContacts( const RArray<TSymgramContact>& aContacts )
    {
    TFileName fn;
    _LIT( KFn, "contacts.bin" );
    if ( PrivateFile( KFn, fn ) != KErrNone )
        {
        return;
        }
    TInt n = aContacts.Count();
    if ( n > 80 )
        {
        n = 80;
        }
    HBufC8* raw = HBufC8::New( 8 + n * 200 );
    if ( !raw )
        {
        return;
        }
    TPtr8 p = raw->Des();
    PutU32( p, KContMagic );
    PutU32( p, (TUint32)n );
    TInt i = 0;
    for ( i = 0; i < n; i++ )
        {
        const TSymgramContact& c = aContacts[ i ];
        PutU64( p, (TUint64)c.iId );
        PutU64( p, (TUint64)c.iHash );
        PutText( p, c.iName, 40 );
        PutText( p, c.iPhone, 24 );
        }
    RFile file;
    if ( file.Replace( iFs, fn, EFileWrite | EFileStream ) == KErrNone )
        {
        file.Write( p );
        file.Close();
        }
    delete raw;
    }

void CSymgramStore::LoadMessagesL( TInt64 aPeer, RArray<TSymgramMsg>& aMsgs )
    {
    TFileName fn;
    MsgFile( aPeer, fn );
    RFile file;
    if ( file.Open( iFs, fn, EFileRead | EFileStream | EFileShareReadersOnly )
         != KErrNone )
        {
        return;
        }
    TInt size = 0;
    file.Size( size );
    if ( size < 8 || size > 65536 )
        {
        file.Close();
        return;
        }
    HBufC8* raw = HBufC8::NewLC( size );
    TPtr8 p = raw->Des();
    if ( file.Read( p ) != KErrNone )
        {
        CleanupStack::PopAndDestroy( raw );
        file.Close();
        return;
        }
    file.Close();
    if ( p.Length() < 8 || GetU32( p.Ptr() ) != KMsgMagic )
        {
        CleanupStack::PopAndDestroy( raw );
        return;
        }
    const TInt n = (TInt)GetU32( p.Ptr() + 4 );
    TInt o = 8;
    TInt i = 0;
    for ( i = 0; i < n && i < 50; i++ )
        {
        if ( o + 42 > p.Length() )
            {
            break;
            }
        TSymgramMsg m;
        m.iBmp = NULL;
        m.iId = (TInt)GetU32( p.Ptr() + o ); o += 4;
        m.iDate = (TInt)GetU32( p.Ptr() + o ); o += 4;
        m.iOut = p[ o++ ] ? ETrue : EFalse;
        m.iKind = p[ o++ ];
        m.iDuration = (TInt)GetU32( p.Ptr() + o ); o += 4;
        m.iPhotoLoc = p[ o++ ] ? ETrue : EFalse;
        m.iFileId = (TInt64)GetU64( p.Ptr() + o ); o += 8;
        m.iFileHash = (TInt64)GetU64( p.Ptr() + o ); o += 8;
        const TInt rlen = p[ o++ ];
        m.iFileRef.Zero();
        if ( rlen < 0 || rlen > 96 || o + rlen > p.Length() )
            {
            break;
            }
        m.iFileRef.Copy( p.Ptr() + o, rlen );
        o += rlen;
        m.iFileName.Zero();
        if ( ReadText( p.Ptr(), p.Length(), o, m.iText, 140 ) != KErrNone )
            {
            break;
            }
        if ( o < p.Length() )
            {
            ReadText( p.Ptr(), p.Length(), o, m.iFileName, 40 );
            }
        aMsgs.Append( m );
        }
    CleanupStack::PopAndDestroy( raw );
    }

void CSymgramStore::SaveMessages( TInt64 aPeer, const RArray<TSymgramMsg>& aMsgs )
    {
    TFileName fn;
    MsgFile( aPeer, fn );
    TInt n = aMsgs.Count();
    TInt skip = 0;
    if ( n > 50 )
        {
        skip = n - 50;
        n = 50;
        }
    HBufC8* raw = HBufC8::New( 8 + n * 500 );
    if ( !raw )
        {
        return;
        }
    TPtr8 p = raw->Des();
    PutU32( p, KMsgMagic );
    PutU32( p, (TUint32)n );
    TInt i = 0;
    for ( i = 0; i < n; i++ )
        {
        const TSymgramMsg& m = aMsgs[ skip + i ];
        PutI32( p, m.iId );
        PutI32( p, m.iDate );
        p.Append( m.iOut ? 1 : 0 );
        p.Append( m.iKind );
        PutI32( p, m.iDuration );
        p.Append( m.iPhotoLoc ? 1 : 0 );
        PutU64( p, (TUint64)m.iFileId );
        PutU64( p, (TUint64)m.iFileHash );
        const TInt rlen = m.iFileRef.Length();
        p.Append( (TUint8)rlen );
        p.Append( m.iFileRef );
        PutText( p, m.iText, 140 );
        PutText( p, m.iFileName, 40 );
        }
    RFile file;
    if ( file.Replace( iFs, fn, EFileWrite | EFileStream ) == KErrNone )
        {
        file.Write( p );
        file.Close();
        }
    delete raw;
    }

void CSymgramStore::ClearAll()
    {
    TFileName fn;
    _LIT( KChats, "chats.bin" );
    _LIT( KCont, "contacts.bin" );
    if ( PrivateFile( KChats, fn ) == KErrNone )
        {
        iFs.Delete( fn );
        }
    if ( PrivateFile( KCont, fn ) == KErrNone )
        {
        iFs.Delete( fn );
        }
    }
