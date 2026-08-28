#include <random.h>
#include <e32err.h>
#include <e32std.h>
#include <hal.h>
#include <hal_data.h>

#include "SymgramSession.h"
#include "SymgramApi.h"

namespace
    {
    const TUint32 KDcAddr[] =
        {
        0,
        INET_ADDR( 149, 154, 175, 53 ),
        INET_ADDR( 149, 154, 167, 51 ),
        INET_ADDR( 149, 154, 175, 100 ),
        INET_ADDR( 149, 154, 167, 91 ),
        INET_ADDR( 91, 108, 56, 165 )
        };
    const TInt KDcMax = 5;
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
    const TUint32 KGetDialogs = 0xa0f4cb4fu;
    const TUint32 KGetHistory = 0x4423e6c5u;
    const TUint32 KSendMessage = 0x1cc20387u;
    const TUint32 KGetFile = 0xbe5335beu;
    const TUint32 KSaveFilePart = 0xb304a621u;
    const TUint32 KSendMedia = 0x7547c966u;
    const TUint32 KInputFile = 0xf52ff27fu;
    const TUint32 KInputMediaPhoto = 0x1e287d04u;
    const TUint32 KInputMediaDoc = 0x5b38c6c1u;
    const TUint32 KDocAttrName = 0x15590068u;
    const TUint32 KBoolTrue = 0x997275b5u;
    const TUint32 KBoolFalse = 0xbc799737u;
    const TInt KUploadChunk = 8192;
    const TInt KUploadMax = 524288;
    const TUint32 KMessages = 0x8c718e87u;
    const TUint32 KMessagesSlice = 0x3a54685eu;
    const TUint32 KChannelMsgs = 0xc776ba4eu;
    const TUint32 KUploadFile = 0x096a18d5u;
    const TUint32 KUploadFileCdn = 0xf18cda44u;
    const TInt KDownloadChunk = 32768;
    const TInt KDownloadMax = 2097152;
    // LAYER 158 (tdesktop v4.8.3). A wrong Updates ctor after sendMessage
    // used to hit FailTextL, which closed the socket from RunL.
    const TUint32 KUpdShortSent = 0x9015e101u;
    const TUint32 KUpdates = 0x74ae4240u;
    const TUint32 KUpdatesComb = 0x725b04c3u;
    const TUint32 KUpdateShort = 0x78d4dec1u;
    const TUint32 KUpdShortMsg = 0x313bc7f8u;
    const TUint32 KUpdShortChat = 0x4d6deea5u;
    const TUint32 KUpdatesTooLong = 0xe317af7eu;
    const TUint32 KInputPeerUser = 0xdde8a54cu;
    const TUint32 KInputPeerChat = 0x35a95cb9u;
    const TUint32 KInputPeerChan = 0x27bcbbfcu;
    const TUint32 KInputPhotoLoc = 0x40181ffeu;
    const TUint32 KInputDocLoc = 0xbad07584u;
    const TUint32 KGetContacts = 0x5dd69e12u;
    const TUint32 KContacts = 0xeae87e42u;
    const TUint32 KContactsOld = 0x6f8b8cb4u;
    const TUint32 KContactsNotMod = 0xb74ba9d2u;
    const TUint32 KContact = 0x145ade0cu;
    const TUint32 KContactOld = 0xf911c994u;
    const TUint32 KInputPeerEmpty = 0x7f3b18eau;
    const TUint32 KDialogs = 0x15ba6c40u;
    const TUint32 KDialogsSlice = 0x71e094f3u;
    const TUint32 KTlVector = 0x1cb5c415u;
    const TUint32 KSessionMagic = 0x01534753u;
    const TInt KLayer = 158;
    const TInt KPbkdfIters = 100000;
    const TInt KPbkdfChunk = 48;
    const TInt KSrpBitBudget = 12;
    const TInt KMaxIn = 65536;

    struct TDhWork
        {
        TBn iP;
        TBn iG;
        TBn iB;
        TBn iGa;
        TBn iGb;
        TBn iAk;
        TUint8 iBbytes[ 256 ];
        TUint8 iAkb[ 256 ];
        TUint8 iGbb[ 256 ];
        };

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

    void Md5( const TUint8* aData, TInt aLen, TUint8 aOut[ 16 ] )
        {
        static const TUint32 K[ 64 ] =
            {
            0xd76aa478u, 0xe8c7b756u, 0x242070dbu, 0xc1bdceeeu,
            0xf57c0fafu, 0x4787c62au, 0xa8304613u, 0xfd469501u,
            0x698098d8u, 0x8b44f7afu, 0xffff5bb1u, 0x895cd7beu,
            0x6b901122u, 0xfd987193u, 0xa679438eu, 0x49b40821u,
            0xf61e2562u, 0xc040b340u, 0x265e5a51u, 0xe9b6c7aau,
            0xd62f105du, 0x02441453u, 0xd8a1e681u, 0xe7d3fbc8u,
            0x21e1cde6u, 0xc33707d6u, 0xf4d50d87u, 0x455a14edu,
            0xa9e3e905u, 0xfcefa3f8u, 0x676f02d9u, 0x8d2a4c8au,
            0xfffa3942u, 0x8771f681u, 0x6d9d6122u, 0xfde5380cu,
            0xa4beea44u, 0x4bdecfa9u, 0xf6bb4b60u, 0xbebfbc70u,
            0x289b7ec6u, 0xeaa127fau, 0xd4ef3085u, 0x04881d05u,
            0xd9d4d039u, 0xe6db99e5u, 0x1fa27cf8u, 0xc4ac5665u,
            0xf4292244u, 0x432aff97u, 0xab9423a7u, 0xfc93a039u,
            0x655b59c3u, 0x8f0ccc92u, 0xffeff47du, 0x85845dd1u,
            0x6fa87e4fu, 0xfe2ce6e0u, 0xa3014314u, 0x4e0811a1u,
            0xf7537e82u, 0xbd3af235u, 0x2ad7d2bbu, 0xeb86d391u
            };
        static const TInt S[ 64 ] =
            {
            7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
            };
        TUint32 a0 = 0x67452301u;
        TUint32 b0 = 0xefcdab89u;
        TUint32 c0 = 0x98badcfeu;
        TUint32 d0 = 0x10325476u;
        const TInt bitLen = aLen * 8;
        const TInt padLen = ( ( aLen + 8 ) / 64 + 1 ) * 64;
        HBufC8* buf = HBufC8::New( padLen );
        if ( !buf )
            {
            Mem::FillZ( aOut, 16 );
            return;
            }
        TPtr8 p = buf->Des();
        p.FillZ( padLen );
        Mem::Copy( const_cast<TUint8*>( p.Ptr() ), aData, aLen );
        const_cast<TUint8*>( p.Ptr() )[ aLen ] = 0x80;
        TUint8* raw = const_cast<TUint8*>( p.Ptr() );
        raw[ padLen - 8 ] = (TUint8)bitLen;
        raw[ padLen - 7 ] = (TUint8)( bitLen >> 8 );
        raw[ padLen - 6 ] = (TUint8)( bitLen >> 16 );
        raw[ padLen - 5 ] = (TUint8)( bitLen >> 24 );
        TInt off = 0;
        for ( off = 0; off < padLen; off += 64 )
            {
            TUint32 w[ 16 ];
            TInt i = 0;
            for ( i = 0; i < 16; i++ )
                {
                w[ i ] = GetU32( raw + off + i * 4 );
                }
            TUint32 a = a0;
            TUint32 b = b0;
            TUint32 c = c0;
            TUint32 d = d0;
            for ( i = 0; i < 64; i++ )
                {
                TUint32 f = 0;
                TInt g = 0;
                if ( i < 16 )
                    {
                    f = ( b & c ) | ( ( ~b ) & d );
                    g = i;
                    }
                else if ( i < 32 )
                    {
                    f = ( d & b ) | ( ( ~d ) & c );
                    g = ( 5 * i + 1 ) % 16;
                    }
                else if ( i < 48 )
                    {
                    f = b ^ c ^ d;
                    g = ( 3 * i + 5 ) % 16;
                    }
                else
                    {
                    f = c ^ ( b | ( ~d ) );
                    g = ( 7 * i ) % 16;
                    }
                f = f + a + K[ i ] + w[ g ];
                a = d;
                d = c;
                c = b;
                b = b + ( ( f << S[ i ] ) | ( f >> ( 32 - S[ i ] ) ) );
                }
            a0 += a;
            b0 += b;
            c0 += c;
            d0 += d;
            }
        delete buf;
        aOut[ 0 ] = (TUint8)a0;
        aOut[ 1 ] = (TUint8)( a0 >> 8 );
        aOut[ 2 ] = (TUint8)( a0 >> 16 );
        aOut[ 3 ] = (TUint8)( a0 >> 24 );
        aOut[ 4 ] = (TUint8)b0;
        aOut[ 5 ] = (TUint8)( b0 >> 8 );
        aOut[ 6 ] = (TUint8)( b0 >> 16 );
        aOut[ 7 ] = (TUint8)( b0 >> 24 );
        aOut[ 8 ] = (TUint8)c0;
        aOut[ 9 ] = (TUint8)( c0 >> 8 );
        aOut[ 10 ] = (TUint8)( c0 >> 16 );
        aOut[ 11 ] = (TUint8)( c0 >> 24 );
        aOut[ 12 ] = (TUint8)d0;
        aOut[ 13 ] = (TUint8)( d0 >> 8 );
        aOut[ 14 ] = (TUint8)( d0 >> 16 );
        aOut[ 15 ] = (TUint8)( d0 >> 24 );
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
        TInt i = 0;
        while ( i < aIn.Length() && aOut.Length() < aOut.MaxLength() )
            {
            const TUint c = aIn[ i++ ];
            TUint cp = c;
            if ( c < 0x80 )
                {
                }
            else if ( ( c & 0xE0 ) == 0xC0 && i < aIn.Length() )
                {
                cp = ( ( c & 0x1F ) << 6 ) | ( aIn[ i++ ] & 0x3F );
                }
            else if ( ( c & 0xF0 ) == 0xE0 && i + 1 < aIn.Length() )
                {
                cp = ( ( c & 0x0F ) << 12 )
                   | ( ( aIn[ i ] & 0x3F ) << 6 )
                   | ( aIn[ i + 1 ] & 0x3F );
                i += 2;
                }
            else if ( ( c & 0xF8 ) == 0xF0 && i + 2 < aIn.Length() )
                {
                cp = ( ( c & 7 ) << 18 )
                   | ( ( aIn[ i ] & 0x3F ) << 12 )
                   | ( ( aIn[ i + 1 ] & 0x3F ) << 6 )
                   | ( aIn[ i + 2 ] & 0x3F );
                i += 3;
                }
            else
                {
                continue;
                }
            if ( cp == 0x200D || cp == 0x20E3 ||
                 ( cp >= 0xFE00 && cp <= 0xFE0F ) ||
                 ( cp >= 0x1F3FB && cp <= 0x1F3FF ) ||
                 ( cp >= 0xE0020 && cp <= 0xE007F ) )
                {
                continue;
                }
            if ( cp > 0xFFFF )
                {
                if ( aOut.Length() + 2 <= aOut.MaxLength() )
                    {
                    const TUint u = cp - 0x10000;
                    aOut.Append( (TText)( 0xD800 | ( u >> 10 ) ) );
                    aOut.Append( (TText)( 0xDC00 | ( u & 0x3FF ) ) );
                    }
                continue;
                }
            aOut.Append( (TText)cp );
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

    TInt Need( TInt aO, TInt aN, TInt aAdd )
        {
        return ( aO + aAdd <= aN ) ? KErrNone : KErrCorrupt;
        }

    TInt SkipBytes( const TUint8* aP, TInt aN, TInt& aO )
        {
        const TUint8* data = NULL;
        TInt len = 0;
        const TInt used = TlReadBytes( aP + aO, aN - aO, data, len );
        if ( used < 0 )
            {
            return KErrCorrupt;
            }
        aO += used;
        return KErrNone;
        }

    TInt ReadStr( const TUint8* aP, TInt aN, TInt& aO, TDes& aOut )
        {
        const TUint8* data = NULL;
        TInt len = 0;
        const TInt used = TlReadBytes( aP + aO, aN - aO, data, len );
        if ( used < 0 )
            {
            return KErrCorrupt;
            }
        aO += used;
        TPtrC8 raw( data, len );
        ToUnicode( raw, aOut );
        return KErrNone;
        }

    TInt SkipBool( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        return KErrNone;
        }

    TInt SkipSound( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0x830b9ae4u )
            {
            if ( SkipBytes( aP, aN, aO ) != KErrNone ||
                 SkipBytes( aP, aN, aO ) != KErrNone )
                {
                return KErrCorrupt;
                }
            }
        else if ( c == 0xff6c8049u )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 8;
            }
        return KErrNone;
        }

    TInt SkipNotify( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 8 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        if ( ( flags & 1 ) != 0 && SkipBool( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & 2 ) != 0 && SkipBool( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & 4 ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        TInt b = 3;
        for ( b = 3; b <= 5; b++ )
            {
            if ( ( flags & ( 1u << b ) ) != 0 &&
                 SkipSound( aP, aN, aO ) != KErrNone )
                {
                return KErrCorrupt;
                }
            }
        if ( ( flags & 64 ) != 0 && SkipBool( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & 128 ) != 0 && SkipBool( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        for ( b = 8; b <= 10; b++ )
            {
            if ( ( flags & ( 1u << b ) ) != 0 &&
                 SkipSound( aP, aN, aO ) != KErrNone )
                {
                return KErrCorrupt;
                }
            }
        return KErrNone;
        }

    struct TPeerName
        {
        TInt64 iId;
        TInt64 iHash;
        TInt iKind;
        TBuf<40> iName;
        TBool iSelf;
        TBool iDeleted;
        };

    TInt SkipUserPhoto( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0x4f11bae1u )
            {
            return KErrNone;
            }
        if ( Need( aO, aN, 12 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        aO += 8;
        if ( ( flags & 2 ) != 0 && SkipBytes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        return KErrNone;
        }

    TInt SkipUserStatus( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0x09d05049u )
            {
            return KErrNone;
            }
        if ( c == 0xedb93949u || c == 0x008c703fu )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            return KErrNone;
            }
        if ( c == 0xe26f42f1u || c == 0x07bf09fcu || c == 0x77ebc742u ||
             c == 0x65899777u )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            return KErrNone;
            }
        return KErrNone;
        }

    TInt SkipPeerColor( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 8 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        if ( ( flags & 1 ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        if ( ( flags & 2 ) != 0 )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 8;
            }
        return KErrNone;
        }

    TInt SkipUserTail( const TUint8* aP, TInt aN, TInt& aO,
                       TUint32 aFlags, TUint32 aFlags2 )
        {
        if ( ( aFlags & ( 1u << 5 ) ) != 0 &&
             SkipUserPhoto( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( aFlags & ( 1u << 6 ) ) != 0 &&
             SkipUserStatus( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( aFlags & ( 1u << 14 ) ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        if ( ( aFlags & ( 1u << 18 ) ) != 0 )
            {
            if ( Need( aO, aN, 8 ) != KErrNone ||
                 GetU32( aP + aO ) != 0x1cb5c415u )
                {
                return KErrCorrupt;
                }
            const TInt n = (TInt)GetU32( aP + aO + 4 );
            aO += 8;
            TInt i = 0;
            for ( i = 0; i < n; i++ )
                {
                if ( Need( aO, aN, 4 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 4;
                if ( SkipBytes( aP, aN, aO ) != KErrNone ||
                     SkipBytes( aP, aN, aO ) != KErrNone ||
                     SkipBytes( aP, aN, aO ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                }
            }
        if ( ( aFlags & ( 1u << 19 ) ) != 0 &&
             SkipBytes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( aFlags & ( 1u << 22 ) ) != 0 &&
             SkipBytes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( aFlags & ( 1u << 30 ) ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            const TUint32 c = GetU32( aP + aO );
            aO += 4;
            if ( c == 0x718d5f4cu )
                {
                if ( Need( aO, aN, 4 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                const TUint32 fl = GetU32( aP + aO );
                aO += 4;
                if ( Need( aO, aN, 8 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 8;
                if ( ( fl & 1 ) != 0 )
                    {
                    if ( Need( aO, aN, 4 ) != KErrNone )
                        {
                        return KErrCorrupt;
                        }
                    aO += 4;
                    }
                }
            else
                {
                if ( Need( aO, aN, 8 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 8;
                if ( c == 0xedfd405fu )
                    {
                    if ( Need( aO, aN, 4 ) != KErrNone )
                        {
                        return KErrCorrupt;
                        }
                    aO += 4;
                    }
                }
            }
        if ( ( aFlags2 & 1 ) != 0 )
            {
            if ( Need( aO, aN, 8 ) != KErrNone ||
                 GetU32( aP + aO ) != 0x1cb5c415u )
                {
                return KErrCorrupt;
                }
            const TInt n = (TInt)GetU32( aP + aO + 4 );
            aO += 8;
            TInt i = 0;
            for ( i = 0; i < n; i++ )
                {
                if ( Need( aO, aN, 8 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 4;
                aO += 4;
                if ( SkipBytes( aP, aN, aO ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                }
            }
        if ( ( aFlags2 & ( 1u << 5 ) ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        if ( ( aFlags2 & ( 1u << 8 ) ) != 0 &&
             SkipPeerColor( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( aFlags2 & ( 1u << 9 ) ) != 0 &&
             SkipPeerColor( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        return KErrNone;
        }

    TBool ConsumeUser( const TUint8* aP, TInt aN, TInt& aO, TPeerName& aOut )
        {
        aOut.iId = 0;
        aOut.iHash = 0;
        aOut.iKind = 1;
        aOut.iName.Zero();
        aOut.iSelf = EFalse;
        aOut.iDeleted = EFalse;
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return EFalse;
            }
        const TUint32 c = GetU32( aP + aO );
        if ( c == 0xd3bc4c2au )
            {
            if ( Need( aO, aN, 12 ) != KErrNone )
                {
                return EFalse;
                }
            aO += 4;
            aOut.iId = (TInt64)GetU64( aP + aO );
            aO += 8;
            aOut.iDeleted = ETrue;
            return ETrue;
            }
        if ( c != 0xabb5f120u && c != 0x8f97c628u )
            {
            return EFalse;
            }
        if ( Need( aO, aN, 16 ) != KErrNone )
            {
            return EFalse;
            }
        aO += 4;
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        const TUint32 flags2 = GetU32( aP + aO );
        aO += 4;
        aOut.iId = (TInt64)GetU64( aP + aO );
        aO += 8;
        aOut.iSelf = ( flags & ( 1u << 10 ) ) != 0;
        aOut.iDeleted = ( flags & ( 1u << 13 ) ) != 0;
        if ( ( flags & 1 ) != 0 )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                return EFalse;
                }
            aOut.iHash = (TInt64)GetU64( aP + aO );
            aO += 8;
            }
        TBuf<40> first;
        TBuf<40> last;
        if ( ( flags & 2 ) != 0 && ReadStr( aP, aN, aO, first ) != KErrNone )
            {
            return EFalse;
            }
        if ( ( flags & 4 ) != 0 && ReadStr( aP, aN, aO, last ) != KErrNone )
            {
            return EFalse;
            }
        aOut.iName.Copy( first );
        if ( last.Length() > 0 &&
             aOut.iName.Length() + 1 + last.Length() <= aOut.iName.MaxLength() )
            {
            aOut.iName.Append( ' ' );
            aOut.iName.Append( last );
            }
        if ( ( flags & 8 ) != 0 )
            {
            TBuf<40> user;
            if ( ReadStr( aP, aN, aO, user ) != KErrNone )
                {
                return EFalse;
                }
            if ( aOut.iName.Length() == 0 )
                {
                aOut.iName.Copy( user );
                }
            }
        if ( ( flags & 16 ) != 0 )
            {
            TBuf<40> phone;
            if ( ReadStr( aP, aN, aO, phone ) != KErrNone )
                {
                return EFalse;
                }
            if ( aOut.iName.Length() == 0 )
                {
                aOut.iName.Copy( phone );
                }
            }
        if ( SkipUserTail( aP, aN, aO, flags, flags2 ) != KErrNone )
            {
            return EFalse;
            }
        return ETrue;
        }

    TInt SkipChatPhoto( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0x37c1011cu )
            {
            return KErrNone;
            }
        if ( c != 0x1c6e1c11u )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 12 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        aO += 8;
        if ( ( flags & 2 ) != 0 && SkipBytes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        return KErrNone;
        }

    TInt SkipChatRights( const TUint8* aP, TInt aN, TInt& aO, TBool aBanned )
        {
        if ( Need( aO, aN, 8 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 8;
        if ( aBanned )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        return KErrNone;
        }

    TInt SkipInputChannel( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0xee8c1e86u )
            {
            return KErrNone;
            }
        if ( c == 0xf35aec28u )
            {
            if ( Need( aO, aN, 16 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 16;
            return KErrNone;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 p = GetU32( aP + aO );
        aO += 4;
        if ( p == 0x35a95cb9u )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 8;
            }
        else if ( p == 0xdde8a54cu || p == 0x27bcbbfcu )
            {
            if ( Need( aO, aN, 16 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 16;
            }
        if ( Need( aO, aN, 12 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 12;
        return KErrNone;
        }

    TInt SkipReasonVector( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 8 ) != KErrNone || GetU32( aP + aO ) != 0x1cb5c415u )
            {
            return KErrCorrupt;
            }
        const TInt n = (TInt)GetU32( aP + aO + 4 );
        aO += 8;
        TInt i = 0;
        for ( i = 0; i < n; i++ )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            if ( SkipBytes( aP, aN, aO ) != KErrNone ||
                 SkipBytes( aP, aN, aO ) != KErrNone ||
                 SkipBytes( aP, aN, aO ) != KErrNone )
                {
                return KErrCorrupt;
                }
            }
        return KErrNone;
        }

    TInt SkipUsernameVector( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 8 ) != KErrNone || GetU32( aP + aO ) != 0x1cb5c415u )
            {
            return KErrCorrupt;
            }
        const TInt n = (TInt)GetU32( aP + aO + 4 );
        aO += 8;
        TInt i = 0;
        for ( i = 0; i < n; i++ )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 8;
            if ( SkipBytes( aP, aN, aO ) != KErrNone )
                {
                return KErrCorrupt;
                }
            }
        return KErrNone;
        }

    TBool IsChatCtor( TUint32 aC )
        {
        return aC == 0x41cbf256u || aC == 0x83259464u || aC == 0x8261ac61u ||
               aC == 0x6592a1a7u || aC == 0x17d493d5u || aC == 0x29562865u;
        }

    TBool IsUserCtor( TUint32 aC )
        {
        return aC == 0xabb5f120u || aC == 0x8f97c628u || aC == 0xd3bc4c2au;
        }

    TBool ConsumeChat( const TUint8* aP, TInt aN, TInt& aO, TPeerName& aOut )
        {
        aOut.iId = 0;
        aOut.iHash = 0;
        aOut.iKind = 2;
        aOut.iName.Zero();
        aOut.iSelf = EFalse;
        aOut.iDeleted = EFalse;
        const TInt start = aO;
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return EFalse;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0x29562865u )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            aOut.iId = (TInt64)GetU64( aP + aO );
            aO += 8;
            return ETrue;
            }
        if ( c == 0x6592a1a7u )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            aOut.iId = (TInt64)GetU64( aP + aO );
            aO += 8;
            if ( ReadStr( aP, aN, aO, aOut.iName ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            return ETrue;
            }
        if ( c == 0x17d493d5u )
            {
            if ( Need( aO, aN, 28 ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            const TUint32 fl = GetU32( aP + aO );
            aO += 4;
            aOut.iId = (TInt64)GetU64( aP + aO );
            aO += 8;
            aOut.iHash = (TInt64)GetU64( aP + aO );
            aO += 8;
            aOut.iKind = ( ( fl & ( 1u << 8 ) ) != 0 ) ? 4 : 3;
            if ( ReadStr( aP, aN, aO, aOut.iName ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            if ( ( fl & ( 1u << 16 ) ) != 0 )
                {
                if ( Need( aO, aN, 4 ) != KErrNone )
                    {
                    aO = start;
                    return EFalse;
                    }
                aO += 4;
                }
            return ETrue;
            }
        if ( c != 0x41cbf256u && c != 0x83259464u && c != 0x8261ac61u )
            {
            aO = start;
            return EFalse;
            }
        if ( Need( aO, aN, 12 ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        TUint32 flags2 = 0;
        if ( c == 0x83259464u )
            {
            flags2 = GetU32( aP + aO );
            aO += 4;
            }
        aOut.iId = (TInt64)GetU64( aP + aO );
        aO += 8;
        if ( c == 0x41cbf256u )
            {
            aOut.iKind = 2;
            }
        else
            {
            aOut.iKind = ( ( flags & ( 1u << 8 ) ) != 0 ) ? 4 : 3;
            if ( ( flags & ( 1u << 13 ) ) != 0 )
                {
                if ( Need( aO, aN, 8 ) != KErrNone )
                    {
                    aO = start;
                    return EFalse;
                    }
                aOut.iHash = (TInt64)GetU64( aP + aO );
                aO += 8;
                }
            }
        if ( ReadStr( aP, aN, aO, aOut.iName ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        if ( c != 0x41cbf256u && ( flags & ( 1u << 6 ) ) != 0 &&
             SkipBytes( aP, aN, aO ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        if ( SkipChatPhoto( aP, aN, aO ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        if ( c == 0x41cbf256u )
            {
            if ( Need( aO, aN, 12 ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            aO += 12;
            if ( ( flags & ( 1u << 6 ) ) != 0 &&
                 SkipInputChannel( aP, aN, aO ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            if ( ( flags & ( 1u << 14 ) ) != 0 &&
                 SkipChatRights( aP, aN, aO, EFalse ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            if ( ( flags & ( 1u << 18 ) ) != 0 &&
                 SkipChatRights( aP, aN, aO, ETrue ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            return ETrue;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        aO += 4;
        if ( ( flags & ( 1u << 9 ) ) != 0 &&
             SkipReasonVector( aP, aN, aO ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        if ( ( flags & ( 1u << 14 ) ) != 0 &&
             SkipChatRights( aP, aN, aO, EFalse ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        if ( ( flags & ( 1u << 15 ) ) != 0 &&
             SkipChatRights( aP, aN, aO, ETrue ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        if ( ( flags & ( 1u << 18 ) ) != 0 &&
             SkipChatRights( aP, aN, aO, ETrue ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        if ( ( flags & ( 1u << 17 ) ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                aO = start;
                return EFalse;
                }
            aO += 4;
            }
        if ( ( flags2 & 1u ) != 0 &&
             SkipUsernameVector( aP, aN, aO ) != KErrNone )
            {
            aO = start;
            return EFalse;
            }
        return ETrue;
        }

    TBool NameFits( TInt aDlgKind, TInt aNameKind )
        {
        if ( aDlgKind <= 1 )
            {
            return aNameKind <= 1;
            }
        if ( aDlgKind == 2 )
            {
            return aNameKind == 2;
            }
        if ( aDlgKind == 3 )
            {
            return aNameKind == 3 || aNameKind == 4;
            }
        return aNameKind == aDlgKind;
        }

    TInt SkipPeer( const TUint8* aP, TInt aN, TInt& aO, TInt& aKind, TInt64& aId )
        {
        if ( Need( aO, aN, 12 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        aId = (TInt64)GetU64( aP + aO );
        aO += 8;
        if ( c == 0x59511722u )
            {
            aKind = 1;
            }
        else if ( c == 0x36c6019au )
            {
            aKind = 2;
            }
        else if ( c == 0xa2a5371eu )
            {
            aKind = 3;
            }
        else
            {
            aKind = 0;
            }
        return KErrNone;
        }

    TInt MigrateDcFromError( const TDesC8& aText )
        {
        _LIT8( KPhone, "PHONE_MIGRATE_" );
        _LIT8( KNet, "NETWORK_MIGRATE_" );
        _LIT8( KUser, "USER_MIGRATE_" );
        TInt skip = 0;
        if ( aText.Length() > KPhone().Length() &&
             aText.Left( KPhone().Length() ) == KPhone )
            {
            skip = KPhone().Length();
            }
        else if ( aText.Length() > KNet().Length() &&
                  aText.Left( KNet().Length() ) == KNet )
            {
            skip = KNet().Length();
            }
        else if ( aText.Length() > KUser().Length() &&
                  aText.Left( KUser().Length() ) == KUser )
            {
            skip = KUser().Length();
            }
        else
            {
            return 0;
            }
        TInt dc = 0;
        TLex8 lex( aText.Mid( skip ) );
        if ( lex.Val( dc ) != KErrNone || dc < 1 || dc > KDcMax )
            {
            return 0;
            }
        return dc;
        }

    void FormatHm( TInt aUnix, TDes& aOut )
        {
        aOut.Zero();
        if ( aUnix <= 0 )
            {
            return;
            }
        TTime epoch;
        _LIT( KEpoch, "19700000:000000.000000" );
        if ( epoch.Set( KEpoch ) != KErrNone )
            {
            return;
            }
        const TTime t = epoch + TTimeIntervalSeconds( aUnix );
        const TDateTime d = t.DateTime();
        if ( d.Hour() < 10 )
            {
            aOut.Append( '0' );
            }
        aOut.AppendNum( d.Hour() );
        aOut.Append( ':' );
        if ( d.Minute() < 10 )
            {
            aOut.Append( '0' );
            }
        aOut.AppendNum( d.Minute() );
        }

    TInt SkipEntities( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 8 ) != KErrNone || GetU32( aP + aO ) != 0x1cb5c415u )
            {
            return KErrCorrupt;
            }
        const TInt n = (TInt)GetU32( aP + aO + 4 );
        aO += 8;
        TInt i = 0;
        for ( i = 0; i < n; i++ )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            const TUint32 c = GetU32( aP + aO );
            aO += 4;
            if ( c == 0xf1ccaaacu )
                {
                if ( Need( aO, aN, 12 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 12;
                continue;
                }
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 8;
            if ( c == 0x73924be0u || c == 0x76a6d327u )
                {
                if ( SkipBytes( aP, aN, aO ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                }
            else if ( c == 0xdc7b1140u || c == 0xc8cf05f8u )
                {
                if ( Need( aO, aN, 8 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 8;
                }
            }
        return KErrNone;
        }

    TInt SkipFwdHeader( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 8 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        TInt kind = 0;
        TInt64 pid = 0;
        if ( ( flags & 1u ) != 0 &&
             SkipPeer( aP, aN, aO, kind, pid ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & ( 1u << 5 ) ) != 0 &&
             SkipBytes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        if ( ( flags & ( 1u << 2 ) ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        if ( ( flags & ( 1u << 3 ) ) != 0 &&
             SkipBytes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & ( 1u << 4 ) ) != 0 )
            {
            if ( SkipPeer( aP, aN, aO, kind, pid ) != KErrNone )
                {
                return KErrCorrupt;
                }
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        if ( ( flags & ( 1u << 6 ) ) != 0 &&
             SkipBytes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        return KErrNone;
        }

    TInt SkipReplyHeader( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 8 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0x9c98bfc1u || c == 0x20e82b3au )
            {
            if ( Need( aO, aN, 12 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 12;
            return KErrNone;
            }
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        TInt kind = 0;
        TInt64 pid = 0;
        if ( c == 0xa6d57763u )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            if ( ( flags & 1u ) != 0 &&
                 SkipPeer( aP, aN, aO, kind, pid ) != KErrNone )
                {
                return KErrCorrupt;
                }
            if ( ( flags & ( 1u << 1 ) ) != 0 )
                {
                if ( Need( aO, aN, 4 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 4;
                }
            return KErrNone;
            }
        if ( ( flags & ( 1u << 4 ) ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        if ( ( flags & 1u ) != 0 &&
             SkipPeer( aP, aN, aO, kind, pid ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & ( 1u << 1 ) ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        if ( ( flags & ( 1u << 6 ) ) != 0 &&
             SkipBytes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & ( 1u << 7 ) ) != 0 &&
             SkipEntities( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & ( 1u << 8 ) ) != 0 )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 4;
            }
        return KErrNone;
        }

    void FlattenPreview( TDes& aText )
        {
        TInt i = 0;
        for ( i = 0; i < aText.Length(); i++ )
            {
            const TText ch = aText[ i ];
            if ( ch == '\n' || ch == '\r' || ch == '\t' )
                {
                aText[ i ] = ' ';
                }
            }
        while ( aText.Length() > 0 && aText[ 0 ] == ' ' )
            {
            aText.Delete( 0, 1 );
            }
        }

    struct TPreview
        {
        TInt iId;
        TInt iDate;
        TBool iOut;
        TUint8 iKind;
        TInt iDuration;
        TBool iPhotoLoc;
        TInt64 iFileId;
        TInt64 iFileHash;
        TBuf8<96> iFileRef;
        TBuf<40> iFileName;
        TBuf<140> iText;
        };

    TInt SkipIntVector( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 8 ) != KErrNone || GetU32( aP + aO ) != 0x1cb5c415u )
            {
            return KErrCorrupt;
            }
        const TInt n = (TInt)GetU32( aP + aO + 4 );
        aO += 8;
        if ( Need( aO, aN, n * 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += n * 4;
        return KErrNone;
        }

    TInt SkipPhotoSize( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( SkipBytes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( c == 0xe17e23cu )
            {
            return KErrNone;
            }
        if ( c == 0x75c78e60u )
            {
            if ( Need( aO, aN, 12 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 12;
            return KErrNone;
            }
        if ( c == 0x21e1ad6u || c == 0xe0b0bc2eu || c == 0xd8214d41u )
            {
            if ( c == 0x21e1ad6u )
                {
                if ( Need( aO, aN, 8 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 8;
                }
            return SkipBytes( aP, aN, aO );
            }
        if ( c == 0xfa3efb95u )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 8;
            return SkipIntVector( aP, aN, aO );
            }
        return KErrNone;
        }

    TInt SkipPhotoSizes( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 8 ) != KErrNone || GetU32( aP + aO ) != 0x1cb5c415u )
            {
            return KErrCorrupt;
            }
        const TInt n = (TInt)GetU32( aP + aO + 4 );
        aO += 8;
        TInt i = 0;
        for ( i = 0; i < n; i++ )
            {
            if ( SkipPhotoSize( aP, aN, aO ) != KErrNone )
                {
                return KErrCorrupt;
                }
            }
        return KErrNone;
        }

    TInt ReadFileRef( const TUint8* aP, TInt aN, TInt& aO, TDes8& aRef )
        {
        const TUint8* data = NULL;
        TInt len = 0;
        const TInt used = TlReadBytes( aP + aO, aN - aO, data, len );
        if ( used < 0 )
            {
            return KErrCorrupt;
            }
        aO += used;
        aRef.Zero();
        if ( len > 0 )
            {
            const TInt n = len < aRef.MaxLength() ? len : aRef.MaxLength();
            aRef.Copy( data, n );
            }
        return KErrNone;
        }

    TInt ParsePhoto( const TUint8* aP, TInt aN, TInt& aO, TPreview& aMsg )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0x2331b22du || c == 0x9c477dd8u )
            {
            return KErrNone;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        if ( Need( aO, aN, 16 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aMsg.iFileId = (TInt64)GetU64( aP + aO );
        aO += 8;
        aMsg.iFileHash = (TInt64)GetU64( aP + aO );
        aO += 8;
        if ( ReadFileRef( aP, aN, aO, aMsg.iFileRef ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        if ( SkipPhotoSizes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & 2 ) != 0 && SkipPhotoSizes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        aMsg.iKind = (TUint8)ESgPhoto;
        aMsg.iPhotoLoc = ETrue;
        return KErrNone;
        }

    TInt SkipStickerSet( const TUint8* aP, TInt aN, TInt& aO )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0x9de7a269u )
            {
            if ( Need( aO, aN, 16 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 16;
            }
        else if ( c == 0x861cc8a0u || c == 0x43b64c29u )
            {
            return SkipBytes( aP, aN, aO );
            }
        return KErrNone;
        }

    TInt ParseDocument( const TUint8* aP, TInt aN, TInt& aO, TPreview& aMsg )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        if ( Need( aO, aN, 16 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aMsg.iFileId = (TInt64)GetU64( aP + aO );
        aO += 8;
        aMsg.iFileHash = (TInt64)GetU64( aP + aO );
        aO += 8;
        if ( ReadFileRef( aP, aN, aO, aMsg.iFileRef ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        TBuf<40> mime;
        if ( ReadStr( aP, aN, aO, mime ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        if ( ( flags & 1 ) != 0 && SkipPhotoSizes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( ( flags & 2 ) != 0 && SkipPhotoSizes( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aO += 4;
        if ( Need( aO, aN, 8 ) != KErrNone || GetU32( aP + aO ) != 0x1cb5c415u )
            {
            return KErrCorrupt;
            }
        const TInt n = (TInt)GetU32( aP + aO + 4 );
        aO += 8;
        TInt i = 0;
        aMsg.iKind = (TUint8)ESgFile;
        aMsg.iPhotoLoc = EFalse;
        _LIT( KOgg, "audio/ogg" );
        _LIT( KAac, "audio/" );
        _LIT( KVid, "video/" );
        _LIT( KWebp, "image/webp" );
        if ( mime.Length() >= 9 )
            {
            TBuf<9> head;
            head.Copy( mime.Left( 9 ) );
            if ( head.CompareF( KOgg ) == 0 )
                {
                aMsg.iKind = (TUint8)ESgVoice;
                }
            }
        if ( mime.Length() >= 6 && aMsg.iKind == (TUint8)ESgFile )
            {
            TBuf<6> head;
            head.Copy( mime.Left( 6 ) );
            if ( head.CompareF( KAac ) == 0 )
                {
                aMsg.iKind = (TUint8)ESgAudio;
                }
            else if ( head.CompareF( KVid ) == 0 )
                {
                aMsg.iKind = (TUint8)ESgVideo;
                }
            }
        if ( mime.CompareF( KWebp ) == 0 )
            {
            aMsg.iKind = (TUint8)ESgSticker;
            }
        for ( i = 0; i < n; i++ )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            const TUint32 ac = GetU32( aP + aO );
            aO += 4;
            if ( ac == 0x9852f9c6u )
                {
                if ( Need( aO, aN, 8 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                const TUint32 af = GetU32( aP + aO );
                aO += 4;
                aMsg.iDuration = GetI32( aP + aO );
                aO += 4;
                if ( ( af & ( 1u << 10 ) ) != 0 )
                    {
                    aMsg.iKind = (TUint8)ESgVoice;
                    }
                else if ( aMsg.iKind != (TUint8)ESgVoice )
                    {
                    aMsg.iKind = (TUint8)ESgAudio;
                    }
                if ( ( af & 1 ) != 0 && SkipBytes( aP, aN, aO ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                if ( ( af & 2 ) != 0 && SkipBytes( aP, aN, aO ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                if ( ( af & 4 ) != 0 && SkipBytes( aP, aN, aO ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                }
            else if ( ac == 0x6319d612u )
                {
                if ( Need( aO, aN, 4 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                const TUint32 sf = GetU32( aP + aO );
                aO += 4;
                TBuf<40> alt;
                if ( ReadStr( aP, aN, aO, alt ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aMsg.iKind = (TUint8)ESgSticker;
                if ( aMsg.iText.Length() == 0 )
                    {
                    aMsg.iText.Copy( alt );
                    }
                if ( SkipStickerSet( aP, aN, aO ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                if ( ( sf & 1 ) != 0 )
                    {
                    if ( Need( aO, aN, 16 ) != KErrNone )
                        {
                        return KErrCorrupt;
                        }
                    aO += 16;
                    }
                }
            else if ( ac == 0x0ef02ce6u || ac == 0x6319d612u )
                {
                }
            else if ( ac == 0xef02ce6u || ac == 0x5910cccbu )
                {
                if ( Need( aO, aN, 4 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                const TUint32 vf = GetU32( aP + aO );
                aO += 4;
                if ( Need( aO, aN, 12 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aMsg.iDuration = GetI32( aP + aO );
                aO += 12;
                aMsg.iKind = (TUint8)ESgVideo;
                if ( ( vf & 4 ) != 0 )
                    {
                    aO += 8;
                    }
                }
            else if ( ac == 0x15590068u )
                {
                TBuf<40> fn;
                if ( ReadStr( aP, aN, aO, fn ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aMsg.iFileName.Copy( fn.Left( aMsg.iFileName.MaxLength() ) );
                if ( aMsg.iText.Length() == 0 )
                    {
                    aMsg.iText.Copy( fn );
                    }
                }
            else if ( ac == 0x6319d612u )
                {
                }
            else
                {
                if ( ac == 0x11b58939u || ac == 0x9801d8f9u ||
                     ac == 0xfd80bd84u || ac == 0x3a36e31bu )
                    {
                    }
                else if ( ac == 0x51448e5u )
                    {
                    if ( Need( aO, aN, 4 ) != KErrNone )
                        {
                        return KErrCorrupt;
                        }
                    aMsg.iDuration = GetI32( aP + aO );
                    aO += 4;
                    if ( SkipBytes( aP, aN, aO ) != KErrNone ||
                         SkipBytes( aP, aN, aO ) != KErrNone )
                        {
                        return KErrCorrupt;
                        }
                    aMsg.iKind = (TUint8)ESgAudio;
                    }
                }
            }
        return KErrNone;
        }

    void LabelKind( TPreview& aMsg )
        {
        if ( aMsg.iText.Length() > 0 )
            {
            return;
            }
        _LIT( KPhoto, "фото" );
        _LIT( KVoice, "голосовое" );
        _LIT( KAudio, "аудио" );
        _LIT( KVideo, "видео" );
        _LIT( KSticker, "стикер" );
        _LIT( KFile, "файл" );
        _LIT( KSvc, "сервис" );
        switch ( aMsg.iKind )
            {
            case ESgPhoto:   aMsg.iText.Copy( KPhoto ); break;
            case ESgVoice:   aMsg.iText.Copy( KVoice ); break;
            case ESgAudio:   aMsg.iText.Copy( KAudio ); break;
            case ESgVideo:   aMsg.iText.Copy( KVideo ); break;
            case ESgSticker: aMsg.iText.Copy( KSticker ); break;
            case ESgFile:    aMsg.iText.Copy( KFile ); break;
            case ESgService: aMsg.iText.Copy( KSvc ); break;
            default: break;
            }
        if ( aMsg.iDuration > 0 &&
             ( aMsg.iKind == ESgVoice || aMsg.iKind == ESgAudio ||
               aMsg.iKind == ESgVideo ) )
            {
            aMsg.iText.Append( ' ' );
            aMsg.iText.AppendNum( aMsg.iDuration / 60 );
            aMsg.iText.Append( ':' );
            const TInt s = aMsg.iDuration % 60;
            if ( s < 10 )
                {
                aMsg.iText.Append( '0' );
                }
            aMsg.iText.AppendNum( s );
            }
        }

    void FillKindFromCtor( TUint32 aC, TPreview& aMsg );

    TInt ParseMedia( const TUint8* aP, TInt aN, TInt& aO, TPreview& aMsg )
        {
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        if ( c == 0x695150d7u || c == 0x695150d3u || c == 0x86fa6a3cu )
            {
            aO += 4;
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            const TUint32 flags = GetU32( aP + aO );
            aO += 4;
            if ( ( flags & 1 ) != 0 &&
                 ParsePhoto( aP, aN, aO, aMsg ) != KErrNone )
                {
                return KErrCorrupt;
                }
            if ( ( flags & 4 ) != 0 )
                {
                if ( Need( aO, aN, 4 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 4;
                }
            aMsg.iKind = (TUint8)ESgPhoto;
            return KErrNone;
            }
        if ( c == 0x9cb070d7u || c == 0x4cf4dfdfu || c == 0x997739cfu )
            {
            aO += 4;
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            const TUint32 flags = GetU32( aP + aO );
            aO += 4;
            if ( ( flags & 1 ) != 0 &&
                 ParseDocument( aP, aN, aO, aMsg ) != KErrNone )
                {
                return KErrCorrupt;
                }
            if ( ( flags & 4 ) != 0 )
                {
                if ( Need( aO, aN, 4 ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                aO += 4;
                }
            return KErrNone;
            }
        if ( c == 0x56e0d474u || c == 0xb940c666u || c == 0x2ec0533fu )
            {
            if ( aMsg.iText.Length() == 0 )
                {
                _LIT( KGeo, "геолокация" );
                aMsg.iText.Copy( KGeo );
                }
            return KErrNotSupported;
            }
        if ( c == 0x3f7ee58bu )
            {
            if ( aMsg.iText.Length() == 0 )
                {
                _LIT( KDice, "стикер" );
                aMsg.iText.Copy( KDice );
                }
            aMsg.iKind = (TUint8)ESgSticker;
            return KErrNotSupported;
            }
        FillKindFromCtor( c, aMsg );
        return KErrNotSupported;
        }

    void FillKindFromCtor( TUint32 aC, TPreview& aMsg )
        {
        if ( aMsg.iText.Length() > 0 )
            {
            return;
            }
        _LIT( KPhoto, "фото" );
        _LIT( KFile, "файл" );
        _LIT( KMedia, "медиа" );
        _LIT( KGeo, "геолокация" );
        _LIT( KContact, "контакт" );
        _LIT( KPoll, "опрос" );
        _LIT( KLink, "ссылка" );
        if ( aC == 0x695150d7u || aC == 0x695150d3u || aC == 0x86fa6a3cu )
            {
            aMsg.iText.Copy( KPhoto );
            aMsg.iKind = (TUint8)ESgPhoto;
            }
        else if ( aC == 0x9cb070d7u || aC == 0x4cf4dfdfu )
            {
            aMsg.iText.Copy( KFile );
            aMsg.iKind = (TUint8)ESgFile;
            }
        else if ( aC == 0x56e0d474u || aC == 0xb940c666u )
            {
            aMsg.iText.Copy( KGeo );
            }
        else if ( aC == 0x70322949u )
            {
            aMsg.iText.Copy( KContact );
            }
        else if ( aC == 0x4bd6e798u )
            {
            aMsg.iText.Copy( KPoll );
            }
        else if ( aC == 0xa32dd600u || aC == 0xddf10c3bu )
            {
            aMsg.iText.Copy( KLink );
            }
        else if ( aC != 0x3ded6320u && aC != 0x9f84f49eu )
            {
            aMsg.iText.Copy( KMedia );
            }
        }

    TInt ReadMessage( const TUint8* aP, TInt aN, TInt& aO, TPreview& aMsg )
        {
        aMsg.iId = 0;
        aMsg.iDate = 0;
        aMsg.iOut = EFalse;
        aMsg.iKind = (TUint8)ESgText;
        aMsg.iDuration = 0;
        aMsg.iPhotoLoc = EFalse;
        aMsg.iFileId = 0;
        aMsg.iFileHash = 0;
        aMsg.iFileRef.Zero();
        aMsg.iFileName.Zero();
        aMsg.iText.Zero();
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 c = GetU32( aP + aO );
        aO += 4;
        if ( c == 0x83e5de54u )
            {
            if ( Need( aO, aN, 4 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aMsg.iId = GetI32( aP + aO );
            aO += 4;
            return KErrNone;
            }
        if ( c == 0x90a6ca84u )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            const TUint32 flags = GetU32( aP + aO );
            aO += 4;
            aMsg.iId = GetI32( aP + aO );
            aO += 4;
            if ( ( flags & 1u ) != 0 )
                {
                TInt kind = 0;
                TInt64 pid = 0;
                if ( SkipPeer( aP, aN, aO, kind, pid ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                }
            return KErrNone;
            }
        if ( c != 0x38116ee0u && c != 0x2b085862u )
            {
            return KErrNotSupported;
            }
        if ( Need( aO, aN, 8 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TUint32 flags = GetU32( aP + aO );
        aO += 4;
        aMsg.iId = GetI32( aP + aO );
        aO += 4;
        aMsg.iOut = ( flags & 2 ) != 0;
        TInt kind = 0;
        TInt64 pid = 0;
        if ( ( flags & ( 1u << 8 ) ) != 0 &&
             SkipPeer( aP, aN, aO, kind, pid ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( SkipPeer( aP, aN, aO, kind, pid ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( c == 0x38116ee0u && ( flags & ( 1u << 2 ) ) != 0 &&
             SkipFwdHeader( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( c == 0x38116ee0u && ( flags & ( 1u << 11 ) ) != 0 )
            {
            if ( Need( aO, aN, 8 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            aO += 8;
            }
        if ( ( flags & ( 1u << 3 ) ) != 0 &&
             SkipReplyHeader( aP, aN, aO ) != KErrNone )
            {
            return KErrCorrupt;
            }
        if ( Need( aO, aN, 4 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        aMsg.iDate = GetI32( aP + aO );
        aO += 4;
        if ( c == 0x2b085862u )
            {
            aMsg.iKind = (TUint8)ESgService;
            _LIT( KSvc, "сервис" );
            aMsg.iText.Copy( KSvc );
            return KErrNone;
            }
        if ( ReadStr( aP, aN, aO, aMsg.iText ) != KErrNone )
            {
            return KErrCorrupt;
            }
        FlattenPreview( aMsg.iText );
        if ( ( flags & ( 1u << 9 ) ) != 0 )
            {
            const TInt saved = aO;
            if ( ParseMedia( aP, aN, aO, aMsg ) != KErrNone )
                {
                aO = saved;
                if ( Need( aO, aN, 4 ) == KErrNone )
                    {
                    FillKindFromCtor( GetU32( aP + aO ), aMsg );
                    }
                }
            LabelKind( aMsg );
            }
        return KErrNone;
        }

    TInt ReadMessagePreview( const TUint8* aP, TInt aN, TInt& aO,
                             TInt& aId, TInt& aDate, TDes& aText )
        {
        TPreview msg;
        const TInt err = ReadMessage( aP, aN, aO, msg );
        aId = msg.iId;
        aDate = msg.iDate;
        aText.Zero();
        aText.Copy( msg.iText.Left( aText.MaxLength() ) );
        return err;
        }

    void PutInputPeer( TDes8& aOut, TInt aKind, TInt64 aId, TInt64 aHash )
        {
        if ( aKind == 2 )
            {
            PutU32( aOut, KInputPeerChat );
            PutU64( aOut, (TUint64)aId );
            }
        else if ( aKind == 3 || aKind == 4 )
            {
            PutU32( aOut, KInputPeerChan );
            PutU64( aOut, (TUint64)aId );
            PutU64( aOut, (TUint64)aHash );
            }
        else
            {
            PutU32( aOut, KInputPeerUser );
            PutU64( aOut, (TUint64)aId );
            PutU64( aOut, (TUint64)aHash );
            }
        }

    _LIT8( KDevice, "Nokia N95 8GB" );
    _LIT8( KSystem, "Symbian OS 9.2" );
    _LIT8( KAppVer, "0.2.10" );
    _LIT8( KLang, "ru" );
    _LIT8( KLangPack, "" );

    _LIT( KStatusIap, "Точка доступа..." );
    _LIT( KStatusConn, "Соединение..." );
    _LIT( KStatusSend, "Запрос к Telegram..." );
    _LIT( KStatusWait, "Ответ Telegram..." );
    _LIT( KStatusKey, "Ключ сессии..." );
    _LIT( KStatusCode, "Запрос кода..." );
    _LIT( KStatusPwd, "Проверка пароля..." );
    _LIT( KStatusResume, "Переподключение..." );
    _LIT( KStatusDc, "Датацентр " );
    _LIT( KStatusChats, "Загрузка чатов..." );
    _LIT( KStatusContacts, "Контакты..." );
    _LIT( KStatusHist, "Сообщения..." );
    _LIT( KStatusOut, "Отправка..." );
    _LIT( KStatusFile, "Медиа..." );
    _LIT( KStatusUp, "Загрузка..." );
    _LIT( KStatusSaved, "Вход..." );
    _LIT( KNeedApi, "Впишите api_id и api_hash в SymgramApi.h" );
    _LIT8( KNeedPwdErr, "SESSION_PASSWORD_NEEDED" );
    _LIT8( KBadPwdErr, "PASSWORD_HASH_INVALID" );
    _LIT8( KSrpIdErr, "SRP_ID_INVALID" );
    _LIT8( KAuthUnreg, "AUTH_KEY_UNREGISTERED" );
    _LIT8( KAuthInv, "AUTH_KEY_INVALID" );
    _LIT8( KSessRev, "SESSION_REVOKED" );
    _LIT8( KUserDead, "USER_DEACTIVATED" );
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
      iAuthed( EFalse ),
      iRpc( ERpcNone ),
      iRsaN( KTelegramRsaN ),
      iAuthKeyId( 0 ),
      iSalt( 0 ),
      iSessionId( 0 ),
      iLastMsgId( 0 ),
      iSeq( 0 ),
      iTimeOffset( 0 ),
      iPhase( 0 ),
      iDcId( 2 ),
      iHaveSrp( EFalse ),
      iSrpTries( 0 ),
      iSrpG( 0 ),
      iSrpId( 0 ),
      iRead( NULL, 0, 0 ),
      iHave( 0 ),
      iResume( EFalse ),
      iResumeTries( 0 ),
      iSwapDcAddr( EFalse ),
      iTriedAddrSwap( EFalse ),
      iPbkdfIdle( NULL ),
      iPbkdfN( 0 ),
      iPbkdfLastPct( -1 ),
      iSrp( NULL ),
      iPeerId( 0 ),
      iPeerKind( 0 ),
      iPeerHash( 0 ),
      iFileMsgId( 0 ),
      iFileId( 0 ),
      iFileHash( 0 ),
      iFilePhoto( EFalse ),
      iPendRpc( ERpcNone ),
      iSaveFull( EFalse ),
      iSaveOpen( EFalse ),
      iSaveOff( 0 ),
      iUpload( NULL ),
      iUploadId( 0 ),
      iUploadPart( 0 ),
      iUploadParts( 0 ),
      iUploadPhoto( EFalse ),
      iIn( NULL )
    {
    CActiveScheduler::Add( this );
    }

void CSymgramSession::ConstructL()
    {
    User::LeaveIfError( iFs.Connect() );
    User::LeaveIfError( iServ.Connect() );
    iIn = HBufC8::NewL( 8192 );
    iPbkdfIdle = CIdle::NewL( CActive::EPriorityLow );
    // EKA2L1 copies TInetAddr as four LE bytes into sockaddr_in with no
    // htonl, so a host-order INET_ADDR goes to the wrong host. Real S60
    // phones stay well under 800 MHz; the emulator often reports the PC.
    TInt khz = 0;
    TInt ram = 0;
    if ( ( HAL::Get( HALData::ECPUSpeed, khz ) == KErrNone &&
           khz >= 800000 ) ||
         ( HAL::Get( HALData::EMemoryRAM, ram ) == KErrNone &&
           ram > 256 * 1024 * 1024 ) )
        {
        iSwapDcAddr = ETrue;
        }
    LoadSession();
    }

CSymgramSession::~CSymgramSession()
    {
    Cancel();
    StopPbkdf();
    delete iPbkdfIdle;
    delete iIn;
    delete iUpload;
    ClearSave();
    CloseSocket();
    iConn.Close();
    iServ.Close();
    iFs.Close();
    }

TBool CSymgramSession::IsBusy() const
    {
    return iBusy;
    }

TBool CSymgramSession::HasSavedSession() const
    {
    return iAuthKey.Length() == 256 && iAuthKeyId != 0 && iPhase >= 3;
    }

const TDesC8& CSymgramSession::Phone() const
    {
    return iPhone;
    }

void CSymgramSession::SendRpcL( const TDesC8& aWrapped )
    {
    if ( IsActive() )
        {
        Cancel();
        }
    if ( aWrapped.Length() <= iLastRpc.MaxLength() )
        {
        iLastRpc.Copy( aWrapped );
        }
    else
        {
        iLastRpc.Zero();
        }
    iBusy = ETrue;
    SendEncryptedL( aWrapped );
    }

void CSymgramSession::LogoutL()
    {
    Cancel();
    CloseSocket();
    DropSavedSession();
    iPwdUtf.Zero();
    iBusy = EFalse;
    iAuthed = EFalse;
    iRpc = ERpcNone;
    iPendRpc = ERpcNone;
    iState = EIdle;
    iObserver.SessionLoggedOutL();
    }

void CSymgramSession::RefreshDialogsL()
    {
    if ( iPhase < 3 || iAuthKeyId == 0 )
        {
        return;
        }
    if ( iBusy )
        {
        iPendRpc = ERpcDialogs;
        return;
        }
    SendGetDialogsL();
    }

void CSymgramSession::GetHistoryL( TInt64 aId, TInt aKind, TInt64 aHash )
    {
    iPeerId = aId;
    iPeerKind = aKind;
    iPeerHash = aHash;
    if ( iBusy || iPhase < 3 )
        {
        iPendRpc = ERpcHistory;
        return;
        }
    SendGetHistoryNowL();
    }

void CSymgramSession::SendTextL( TInt64 aId, TInt aKind, TInt64 aHash,
                                 const TDesC8& aText )
    {
    iPeerId = aId;
    iPeerKind = aKind;
    iPeerHash = aHash;
    iSendUtf.Copy( aText.Left( iSendUtf.MaxLength() ) );
    if ( iBusy || iPhase < 3 )
        {
        iPendRpc = ERpcSend;
        return;
        }
    SendTextNowL();
    }

void CSymgramSession::GetFileL( TInt aMsgId, TInt64 aFileId, TInt64 aHash,
                                const TDesC8& aRef, TBool aPhoto )
    {
    if ( iSaveFull )
        {
        return;
        }
    iFileMsgId = aMsgId;
    iFileId = aFileId;
    iFileHash = aHash;
    iFileRef.Copy( aRef.Left( iFileRef.MaxLength() ) );
    iFilePhoto = aPhoto;
    iSaveFull = EFalse;
    if ( iBusy || iPhase < 3 )
        {
        iPendRpc = ERpcFile;
        return;
        }
    SendGetFileNowL();
    }

void CSymgramSession::ClearSave()
    {
    iSaveFile.Close();
    iSaveFull = EFalse;
    iSaveOpen = EFalse;
    iSaveOff = 0;
    iSavePath.Zero();
    }

void CSymgramSession::GetFileSaveL( TInt aMsgId, TInt64 aFileId, TInt64 aHash,
                                    const TDesC8& aRef, TBool aPhoto,
                                    const TDesC& aPath, TBool aOpen )
    {
    if ( iPhase < 3 || iAuthKeyId == 0 )
        {
        return;
        }
    if ( iBusy )
        {
        _LIT( KWait, "Подождите..." );
        iObserver.SessionStatusL( KWait );
        return;
        }
    ClearSave();
    iFileMsgId = aMsgId;
    iFileId = aFileId;
    iFileHash = aHash;
    iFileRef.Copy( aRef.Left( iFileRef.MaxLength() ) );
    iFilePhoto = aPhoto;
    iSavePath.Copy( aPath.Left( iSavePath.MaxLength() ) );
    iSaveOpen = aOpen;
    iSaveOff = 0;
    iSaveFull = ETrue;
    User::LeaveIfError( iSaveFile.Replace( iFs, iSavePath,
                                           EFileWrite | EFileStream ) );
    SendGetFileNowL();
    }

void CSymgramSession::ClearUpload()
    {
    delete iUpload;
    iUpload = NULL;
    iUploadId = 0;
    iUploadPart = 0;
    iUploadParts = 0;
    iUploadPhoto = EFalse;
    iUploadName.Zero();
    iUploadMd5.Zero();
    }

void CSymgramSession::SendFileL( TInt64 aId, TInt aKind, TInt64 aHash,
                                 const TDesC& aPath )
    {
    if ( iPhase < 3 || iAuthKeyId == 0 )
        {
        return;
        }
    if ( iBusy )
        {
        _LIT( KWait, "Подождите..." );
        iObserver.SessionStatusL( KWait );
        return;
        }
    RFile file;
    User::LeaveIfError( file.Open( iFs, aPath, EFileRead | EFileShareReadersOnly ) );
    CleanupClosePushL( file );
    TInt size = 0;
    User::LeaveIfError( file.Size( size ) );
    if ( size < 32 || size > KUploadMax )
        {
        CleanupStack::PopAndDestroy();
        _LIT( KSz, "Файл слишком большой" );
        iObserver.SessionErrorL( KSz );
        return;
        }
    ClearUpload();
    iUpload = HBufC8::NewL( size );
    TPtr8 body = iUpload->Des();
    User::LeaveIfError( file.Read( body ) );
    CleanupStack::PopAndDestroy();
    TUint8 dig[ 16 ];
    Md5( iUpload->Ptr(), iUpload->Length(), dig );
    _LIT8( KHex, "0123456789abcdef" );
    TInt h = 0;
    for ( h = 0; h < 16; h++ )
        {
        iUploadMd5.Append( KHex()[ ( dig[ h ] >> 4 ) & 0xF ] );
        iUploadMd5.Append( KHex()[ dig[ h ] & 0xF ] );
        }
    TParse parse;
    parse.Set( aPath, NULL, NULL );
    const TPtrC nm = parse.NameAndExt();
    TInt c = 0;
    for ( c = 0; c < nm.Length() && iUploadName.Length() < iUploadName.MaxLength(); c++ )
        {
        const TText ch = nm[ c ];
        if ( ch >= 32 && ch < 127 )
            {
            iUploadName.Append( (TUint8)ch );
            }
        }
    if ( iUploadName.Length() == 0 )
        {
        _LIT8( KDef, "photo.jpg" );
        iUploadName.Copy( KDef );
        }
    iUploadPhoto = ( iUpload->Length() >= 3 &&
                     (*iUpload)[ 0 ] == 0xFF &&
                     (*iUpload)[ 1 ] == 0xD8 &&
                     (*iUpload)[ 2 ] == 0xFF );
    iPeerId = aId;
    iPeerKind = aKind;
    iPeerHash = aHash;
    iUploadId = (TInt64)MessageId();
    iUploadPart = 0;
    iUploadParts = ( iUpload->Length() + KUploadChunk - 1 ) / KUploadChunk;
    SendSavePartNowL();
    }

void CSymgramSession::SendSavePartNowL()
    {
    if ( !iUpload || iUploadPart >= iUploadParts )
        {
        SendMediaNowL();
        return;
        }
    const TInt off = iUploadPart * KUploadChunk;
    TInt n = iUpload->Length() - off;
    if ( n > KUploadChunk )
        {
        n = KUploadChunk;
        }
    HBufC8* query = HBufC8::NewLC( n + 48 );
    TPtr8 q = query->Des();
    PutU32( q, KSaveFilePart );
    PutU64( q, (TUint64)iUploadId );
    PutU32( q, iUploadPart );
    TlAppendBytes( q, iUpload->Ptr() + off, n );
    HBufC8* wrapped = HBufC8::NewLC( q.Length() + 256 );
    TPtr8 w = wrapped->Des();
    WrapInitL( q, w );
    iRpc = ERpcSavePart;
    iObserver.SessionStatusL( KStatusUp );
    SendRpcL( w );
    CleanupStack::PopAndDestroy( 2, query );
    }

void CSymgramSession::SendMediaNowL()
    {
    TBuf8<512> query;
    PutU32( query, KSendMedia );
    PutU32( query, 0 );
    PutInputPeer( query, iPeerKind, iPeerId, iPeerHash );
    if ( iUploadPhoto )
        {
        PutU32( query, KInputMediaPhoto );
        PutU32( query, 0 );
        PutU32( query, KInputFile );
        PutU64( query, (TUint64)iUploadId );
        PutU32( query, iUploadParts );
        TlAppendBytes( query, iUploadName.Ptr(), iUploadName.Length() );
        TlAppendBytes( query, iUploadMd5.Ptr(), iUploadMd5.Length() );
        }
    else
        {
        PutU32( query, KInputMediaDoc );
        PutU32( query, 0 );
        PutU32( query, KInputFile );
        PutU64( query, (TUint64)iUploadId );
        PutU32( query, iUploadParts );
        TlAppendBytes( query, iUploadName.Ptr(), iUploadName.Length() );
        TlAppendBytes( query, iUploadMd5.Ptr(), iUploadMd5.Length() );
        _LIT8( KMime, "application/octet-stream" );
        _LIT8( KJpeg, "image/jpeg" );
        _LIT8( KPng, "image/png" );
        const TDesC8* mime = &KMime;
        if ( iUploadName.Length() >= 4 )
            {
            const TInt n = iUploadName.Length();
            if ( iUploadName[ n - 4 ] == '.' &&
                 ( iUploadName[ n - 3 ] == 'j' || iUploadName[ n - 3 ] == 'J' ) )
                {
                mime = &KJpeg;
                }
            else if ( iUploadName[ n - 4 ] == '.' &&
                      ( iUploadName[ n - 3 ] == 'p' || iUploadName[ n - 3 ] == 'P' ) )
                {
                mime = &KPng;
                }
            }
        TlAppendBytes( query, mime->Ptr(), mime->Length() );
        PutU32( query, KTlVector );
        PutU32( query, 1 );
        PutU32( query, KDocAttrName );
        TlAppendBytes( query, iUploadName.Ptr(), iUploadName.Length() );
        }
    PutU32( query, 0 );
    PutU64( query, MessageId() );
    TBuf8<768> wrapped;
    WrapInitL( query, wrapped );
    iRpc = ERpcSend;
    iObserver.SessionStatusL( KStatusOut );
    SendRpcL( wrapped );
    }

void CSymgramSession::SendGetHistoryNowL()
    {
    TBuf8<80> query;
    PutU32( query, KGetHistory );
    PutInputPeer( query, iPeerKind, iPeerId, iPeerHash );
    PutU32( query, 0 );
    PutU32( query, 0 );
    PutU32( query, 0 );
    PutU32( query, 20 );
    PutU32( query, 0 );
    PutU32( query, 0 );
    PutU64( query, 0 );
    TBuf8<448> wrapped;
    WrapInitL( query, wrapped );
    iRpc = ERpcHistory;
    iObserver.SessionStatusL( KStatusHist );
    SendRpcL( wrapped );
    }

void CSymgramSession::SendTextNowL()
    {
    TBuf8<512> query;
    PutU32( query, KSendMessage );
    PutU32( query, 0 );
    PutInputPeer( query, iPeerKind, iPeerId, iPeerHash );
    TlAppendBytes( query, iSendUtf.Ptr(), iSendUtf.Length() );
    PutU64( query, MessageId() );
    TBuf8<700> wrapped;
    WrapInitL( query, wrapped );
    iRpc = ERpcSend;
    iObserver.SessionStatusL( KStatusOut );
    SendRpcL( wrapped );
    }

void CSymgramSession::SendGetFileNowL()
    {
    TBuf8<256> query;
    PutU32( query, KGetFile );
    PutU32( query, 0 );
    PutU32( query, iFilePhoto ? KInputPhotoLoc : KInputDocLoc );
    PutU64( query, (TUint64)iFileId );
    PutU64( query, (TUint64)iFileHash );
    TlAppendBytes( query, iFileRef.Ptr(), iFileRef.Length() );
    if ( iSaveFull )
        {
        PutU32( query, 0 );
        PutU64( query, (TUint64)iSaveOff );
        }
    else
        {
        _LIT8( KSz, "s" );
        TlAppendBytes( query, KSz().Ptr(), KSz().Length() );
        PutU64( query, 0 );
        }
    PutU32( query, KDownloadChunk );
    TBuf8<512> wrapped;
    WrapInitL( query, wrapped );
    iRpc = iSaveFull ? ERpcSaveFile : ERpcFile;
    iObserver.SessionStatusL( KStatusFile );
    SendRpcL( wrapped );
    }

void CSymgramSession::FlushPendingL()
    {
    const TRpc next = iPendRpc;
    iPendRpc = ERpcNone;
    if ( next == ERpcHistory )
        {
        SendGetHistoryNowL();
        }
    else if ( next == ERpcSend )
        {
        SendTextNowL();
        }
    else if ( next == ERpcFile || next == ERpcSaveFile )
        {
        SendGetFileNowL();
        }
    else if ( next == ERpcDialogs )
        {
        SendGetDialogsL();
        }
    }

TBool CSymgramSession::IsFileMigrate( const TDesC8& aMsg ) const
    {
    _LIT8( KMig, "FILE_MIGRATE_" );
    return aMsg.Length() > KMig().Length() &&
           aMsg.Left( KMig().Length() ) == KMig;
    }

TInt CSymgramSession::SessionFileName( TFileName& aOut )
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
    _LIT( KFn, "session.bin" );
    aOut.Append( KFn );
    return KErrNone;
    }

TInt CSymgramSession::LoadSession()
    {
    TFileName fn;
    if ( SessionFileName( fn ) != KErrNone )
        {
        return KErrNotFound;
        }
    RFile file;
    if ( file.Open( iFs, fn, EFileRead | EFileStream | EFileShareReadersOnly )
         != KErrNone )
        {
        return KErrNotFound;
        }
    TBuf8<320> raw;
    TInt err = file.Read( raw );
    file.Close();
    if ( err != KErrNone || raw.Length() < 309 )
        {
        return KErrCorrupt;
        }
    const TUint8* p = raw.Ptr();
    if ( GetU32( p ) != KSessionMagic )
        {
        return KErrCorrupt;
        }
    const TInt dc = GetI32( p + 4 );
    if ( dc < 1 || dc > KDcMax )
        {
        return KErrCorrupt;
        }
    iDcId = dc;
    iAuthKey.Copy( p + 8, 256 );
    iAuthKeyId = GetU64( p + 264 );
    iSalt = GetU64( p + 272 );
    iTimeOffset = GetI32( p + 280 );
    const TInt n = p[ 284 ];
    if ( n > 0 && n <= iPhone.MaxLength() )
        {
        iPhone.Copy( p + 285, n );
        }
    if ( iAuthKeyId == 0 )
        {
        iAuthKey.Zero();
        return KErrNotFound;
        }
    iPhase = 3;
    return KErrNone;
    }

void CSymgramSession::SaveSession()
    {
    if ( iAuthKey.Length() != 256 || iAuthKeyId == 0 )
        {
        return;
        }
    TFileName fn;
    if ( SessionFileName( fn ) != KErrNone )
        {
        return;
        }
    TBuf8<320> raw;
    PutU32( raw, KSessionMagic );
    PutU32( raw, (TUint32)iDcId );
    raw.Append( iAuthKey );
    PutU64( raw, iAuthKeyId );
    PutU64( raw, iSalt );
    PutU32( raw, (TUint32)iTimeOffset );
    raw.Append( (TUint8)iPhone.Length() );
    raw.Append( iPhone );
    while ( raw.Length() < 309 )
        {
        raw.Append( 0 );
        }
    RFile file;
    TInt err = file.Replace( iFs, fn, EFileWrite | EFileStream );
    if ( err != KErrNone )
        {
        return;
        }
    file.Write( raw );
    file.Close();
    }

void CSymgramSession::DropSavedSession()
    {
    TFileName fn;
    if ( SessionFileName( fn ) == KErrNone )
        {
        iFs.Delete( fn );
        }
    iAuthKey.Zero();
    iAuthKeyId = 0;
    iSalt = 0;
    iPhase = 0;
    iPwdUtf.Zero();
    }

TBool CSymgramSession::IsDeadAuth( const TDesC8& aMsg ) const
    {
    return aMsg == KAuthUnreg
        || aMsg == KAuthInv
        || aMsg == KSessRev
        || aMsg == KUserDead;
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
        case EResuming:
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
    StopPbkdf();
    iPwdUtf.Zero();
    ClearUpload();
    ClearSave();
    CloseSocket();
    iState = EIdle;
    iBusy = EFalse;
    iObserver.SessionFailedL( aError );
    }

void CSymgramSession::FailTextL( const TDesC& aText )
    {
    StopPbkdf();
    iPwdUtf.Zero();
    ClearUpload();
    ClearSave();
    CloseSocket();
    iState = EIdle;
    iBusy = EFalse;
    iObserver.SessionErrorL( aText );
    }

TBool CSymgramSession::CanResume( TInt aError ) const
    {
    if ( iPhase < 3 || iResumeTries >= 3 )
        {
        return EFalse;
        }
    if ( iLastRpc.Length() == 0 && iAuthKeyId == 0 )
        {
        return EFalse;
        }
    return aError == KErrDisconnected
        || aError == KErrCouldNotDisconnect
        || aError == KErrCouldNotConnect
        || aError == KErrTimedOut
        || aError == KErrCommsLineFail
        || aError == KErrNotReady
        || aError == KErrEof
        || aError == KErrSessionClosed;
    }

TInt CSymgramSession::OpenTcpSocket()
    {
    CloseSocket();
    TInt err = KErrNotReady;
    if ( iConn.SubSessionHandle() )
        {
        err = iSocket.Open( iServ, KAfInet, KSockStream,
                            KProtocolInetTcp, iConn );
        }
    if ( err != KErrNone )
        {
        CloseSocket();
        err = iSocket.Open( iServ, KAfInet, KSockStream, KProtocolInetTcp );
        }
    return err;
    }

void CSymgramSession::BeginTcpConnectL()
    {
    User::LeaveIfError( OpenTcpSocket() );
    iAddr = TInetAddr();
    iAddr.SetAddress( DcAddr() );
    iAddr.SetPort( KDcPort );
    if ( iState != EResuming )
        {
        iState = EConnecting;
        }
    if ( iResume )
        {
        iObserver.SessionStatusL( KStatusResume );
        }
    else
        {
        iObserver.SessionStatusL( KStatusConn );
        }
    iSocket.Connect( iAddr, iStatus );
    SetActive();
    }

TBool CSymgramSession::ShouldRetryAddrSwap( TInt aError ) const
    {
    if ( iTriedAddrSwap )
        {
        return EFalse;
        }
    return aError == KErrTimedOut
        || aError == KErrCouldNotConnect
        || aError == KErrGeneral
        || aError == KErrCouldNotDisconnect
        || aError == KErrNotReady;
    }

void CSymgramSession::ResumeConnectL()
    {
    iResumeTries++;
    Cancel();
    CloseSocket();
    iSentAbridged = EFalse;
    iHave = 0;
    iIn->Des().Zero();
    iSeq = 0;
    iLastMsgId = 0;
    TBuf8<8> sid;
    sid.SetLength( 8 );
    GenerateRandomBytesL( sid );
    iSessionId = GetU64( sid.Ptr() );
    iBusy = ETrue;
    iResume = ETrue;
    iState = EResuming;
    BeginTcpConnectL();
    }

void CSymgramSession::RpcFailL( const TDesC8& aMsg )
    {
    if ( ( iRpc == ERpcFile || iRpc == ERpcSaveFile ) && IsFileMigrate( aMsg ) )
        {
        ClearSave();
        iBusy = EFalse;
        iRpc = ERpcNone;
        iState = EIdle;
        _LIT( KOn, "Файл на другом сервере" );
        iObserver.SessionStatusL( KOn );
        FlushPendingL();
        return;
        }
    if ( IsDeadAuth( aMsg ) )
        {
        DropSavedSession();
        iBusy = EFalse;
        iState = EIdle;
        iAuthed = EFalse;
        iObserver.SessionLoggedOutL();
        }
    TBuf<64> text;
    ToUnicode( aMsg, text );
    iBusy = EFalse;
    iState = EIdle;
    iRpc = ERpcNone;
    ClearUpload();
    ClearSave();
    iObserver.SessionErrorL( text );
    }

TUint32 CSymgramSession::DcAddr() const
    {
    TUint32 addr = KDcAddr[ 2 ];
    if ( iDcId >= 1 && iDcId <= KDcMax )
        {
        addr = KDcAddr[ iDcId ];
        }
    if ( iSwapDcAddr )
        {
        addr = ByteOrder::Swap32( addr );
        }
    return addr;
    }

void CSymgramSession::MigrateToDcL( TInt aDc )
    {
    if ( aDc < 1 || aDc > KDcMax )
        {
        FailL( KErrNotFound );
        return;
        }
    iDcId = aDc;
    iPhase = 0;
    iAuthKey.Zero();
    iAuthKeyId = 0;
    iSalt = 0;
    iSentAbridged = EFalse;
    iResumeTries = 0;
    TBuf<24> text;
    text.Copy( KStatusDc );
    text.AppendNum( aDc );
    iObserver.SessionStatusL( text );
    ResumeConnectL();
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

    iHave = 0;
    iPhase = 0;
    iSentAbridged = EFalse;
    iResume = EFalse;
    iResumeTries = 0;
    iTriedAddrSwap = EFalse;
    iSeq = 0;
    iLastMsgId = 0;
    iTimeOffset = 0;
    iIn->Des().Zero();
    iOut.Zero();
    iPhone.Copy( aPhone );
    iPhoneCodeHash.Zero();
    iHaveSrp = EFalse;
    iDcId = 2;
    iAuthKey.Zero();
    iAuthKeyId = 0;
    iSalt = 0;
    BeginIapL();
    }

void CSymgramSession::StartSavedL()
    {
    if ( iBusy || !HasSavedSession() )
        {
        return;
        }
    if ( KSymgramApiId <= 0 || KSymgramApiHash().Length() == 0 )
        {
        iObserver.SessionErrorL( KNeedApi );
        return;
        }
    TBuf8<8> sid;
    sid.SetLength( 8 );
    GenerateRandomBytesL( sid );
    iSessionId = GetU64( sid.Ptr() );
    iLastRpc.Zero();
    iHaveSrp = EFalse;
    iObserver.SessionStatusL( KStatusSaved );
    iAuthed = ETrue;
    BeginIapL();
    }

void CSymgramSession::BeginIapL()
    {
    Cancel();
    CloseSocket();
    iConn.Close();
    iHave = 0;
    iBusy = ETrue;
    iSentAbridged = EFalse;
    iResume = EFalse;
    iResumeTries = 0;
    iTriedAddrSwap = EFalse;
    iSeq = 0;
    iLastMsgId = 0;
    iIn->Des().Zero();
    iOut.Zero();

    User::LeaveIfError( iConn.Open( iServ ) );

    iPref = TCommDbConnPref();
    iPref.SetDialogPreference( ECommDbDialogPrefDoNotPrompt );
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

TInt CSymgramSession::SubmitPasswordL( const TDesC8& aPassword )
    {
    if ( iPhase != 3 || !iHaveSrp )
        {
        return KErrNotReady;
        }
    if ( iBusy )
        {
        return KErrInUse;
        }
    iPwdUtf.Copy( aPassword.Left( iPwdUtf.MaxLength() ) );
    if ( IsActive() )
        {
        Cancel();
        }
    iBusy = ETrue;
    TInt err = StartPbkdfL( aPassword );
    if ( err != KErrNone )
        {
        FailL( err );
        return err;
        }
    return KErrNone;
    }

void CSymgramSession::ClearSrp()
    {
    if ( iSrp )
        {
        Mem::FillZ( iSrp, sizeof( TSrpWork ) );
        delete iSrp;
        iSrp = NULL;
        }
    }

void CSymgramSession::StopPbkdf()
    {
    if ( iPbkdfIdle && iPbkdfIdle->IsActive() )
        {
        iPbkdfIdle->Cancel();
        }
    iPbkdfN = 0;
    iPbkdfLastPct = -1;
    Mem::FillZ( iPbkdfPass, 32 );
    Mem::FillZ( iPbkdfU, 64 );
    Mem::FillZ( iPbkdfT, 64 );
    ClearSrp();
    }

void CSymgramSession::ReportPbkdfProgressL()
    {
    TInt pct = ( iPbkdfN * 100 ) / KPbkdfIters;
    if ( pct > 100 )
        {
        pct = 100;
        }
    if ( pct == iPbkdfLastPct )
        {
        return;
        }
    iPbkdfLastPct = pct;
    TBuf<24> text;
    _LIT( KPct, "Пароль " );
    text.Copy( KPct );
    text.AppendNum( pct );
    text.Append( '%' );
    iObserver.SessionStatusL( text );
    }

TInt CSymgramSession::PbkdfCb( TAny* aPtr )
    {
    CSymgramSession* self = static_cast<CSymgramSession*>( aPtr );
    TInt cont = 0;
    TRAPD( err, cont = self->iSrp ? self->SrpStepL() : self->PbkdfStep() );
    if ( err != KErrNone )
        {
        TRAP_IGNORE( self->FailL( err ) );
        return 0;
        }
    return cont;
    }

TInt CSymgramSession::PbkdfStep()
    {
    const TInt left = KPbkdfIters - iPbkdfN;
    if ( left <= 0 )
        {
        StartSrpL();
        return 1;
        }
    TInt chunk = KPbkdfChunk;
    if ( chunk > left )
        {
        chunk = left;
        }
    Pbkdf2HmacSha512Rounds( iPbkdfPass, 32, iPbkdfU, iPbkdfT, chunk );
    iPbkdfN += chunk;
    ReportPbkdfProgressL();
    if ( iPbkdfN >= KPbkdfIters )
        {
        StartSrpL();
        return 1;
        }
    return 1;
    }

TInt CSymgramSession::StartPbkdfL( const TDesC8& aPassword )
    {
    const TInt pLen = iSrpP.Length();
    if ( pLen < 64 || pLen > 256 || iSrpB.Length() < 1 )
        {
        return KErrCorrupt;
        }

    TUint8* cat = (TUint8*)User::AllocL( 900 );
    CleanupStack::PushL( cat );
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
    CleanupStack::PopAndDestroy( cat );
    Mem::Copy( iPbkdfPass, hash2, 32 );

    Pbkdf2HmacSha512Begin( iPbkdfPass, 32, iSalt1.Ptr(), iSalt1.Length(),
                           iPbkdfU, iPbkdfT );
    iPbkdfN = 1;
    iPbkdfLastPct = -1;
    ReportPbkdfProgressL();
    iPbkdfIdle->Start( TCallBack( PbkdfCb, this ) );
    return KErrNone;
    }

void CSymgramSession::BeginSrpExp( const TBn& aBase, const TUint32* aExp,
                                   TInt aLimbs )
    {
    TInt n = aLimbs;
    if ( n > 80 )
        {
        n = 80;
        }
    Mem::FillZ( iSrp->iExp, sizeof( iSrp->iExp ) );
    Mem::Copy( iSrp->iExp, aExp, n * sizeof( TUint32 ) );
    iSrp->iExpLimbs = n;
    BnExpBegin( iSrp->iAcc, iSrp->iBaseM, iSrp->iN0inv, aBase, iSrp->iP );
    BnExpMsb( iSrp->iExp, n, iSrp->iLimb, iSrp->iBit );
    iSrp->iBitsTotal += iSrp->iLimb * 32 + iSrp->iBit + 1;
    }

void CSymgramSession::ReportSrpProgressL()
    {
    TInt pct = 0;
    if ( iSrp && iSrp->iBitsTotal > 0 )
        {
        pct = ( iSrp->iBitsDone * 100 ) / iSrp->iBitsTotal;
        if ( pct > 100 )
            {
            pct = 100;
            }
        }
    TBuf<24> text;
    _LIT( KKey, "Ключ " );
    text.Copy( KKey );
    text.AppendNum( pct );
    text.Append( '%' );
    iObserver.SessionStatusL( text );
    }

void CSymgramSession::StartSrpL()
    {
    const TInt pLen = iSrpP.Length();
    if ( pLen < 64 || pLen > 256 || iSrpB.Length() < 1 )
        {
        FailL( KErrCorrupt );
        return;
        }

    ClearSrp();
    iSrp = new ( ELeave ) TSrpWork;
    Mem::FillZ( iSrp, sizeof( TSrpWork ) );
    iSrp->iStage = 1;

    TUint8 cat[ 576 ];
    TInt n = 0;
    Mem::Copy( cat + n, iSalt2.Ptr(), iSalt2.Length() );
    n += iSalt2.Length();
    Mem::Copy( cat + n, iPbkdfT, 64 );
    n += 64;
    Mem::Copy( cat + n, iSalt2.Ptr(), iSalt2.Length() );
    n += iSalt2.Length();
    TUint8 xbytes[ 32 ];
    Sha256( cat, n, xbytes );
    Mem::FillZ( iPbkdfPass, 32 );
    Mem::FillZ( iPbkdfU, 64 );
    Mem::FillZ( iPbkdfT, 64 );

    iSrp->iP.FromBe( iSrpP.Ptr(), pLen );
    iSrp->iG.Zero();
    iSrp->iG.iD[ 0 ] = iSrpG;
    iSrp->iX.FromBe( xbytes, 32 );
    Mem::FillZ( xbytes, 32 );
    BeginSrpExp( iSrp->iG, iSrp->iX.iD, KBnLimbs );
    ReportSrpProgressL();
    }

TInt CSymgramSession::SrpStepL()
    {
    if ( !iSrp )
        {
        return 0;
        }

    const TInt remain = iSrp->iLimb * 32 + iSrp->iBit + 1;
    const TBool done = BnExpStep( iSrp->iAcc, iSrp->iBaseM, iSrp->iP,
                                  iSrp->iN0inv, iSrp->iExp,
                                  iSrp->iLimb, iSrp->iBit, KSrpBitBudget );
    if ( !done )
        {
        iSrp->iBitsDone += KSrpBitBudget;
        ReportSrpProgressL();
        return 1;
        }
    iSrp->iBitsDone += remain;
    return SrpOnExpDoneL();
    }

TInt CSymgramSession::SrpOnExpDoneL()
    {
    if ( iSrp->iStage == 1 )
        {
        BnExpFinish( iSrp->iGx, iSrp->iAcc, iSrp->iP, iSrp->iN0inv );
        TUint8 abytes[ 256 ];
        TPtr8 abuf( abytes, 256, 256 );
        abuf.SetLength( 256 );
        GenerateRandomBytesL( abuf );
        iSrp->iAexp.FromBe( abytes, 256 );
        Mem::FillZ( abytes, 256 );
        iSrp->iStage = 2;
        BeginSrpExp( iSrp->iG, iSrp->iAexp.iD, KBnLimbs );
        ReportSrpProgressL();
        return 1;
        }

    if ( iSrp->iStage == 2 )
        {
        return SrpBeginSharedL();
        }

    BnExpFinish( iSrp->iS, iSrp->iAcc, iSrp->iP, iSrp->iN0inv );
    FinishSrpSendL();
    return 0;
    }

TInt CSymgramSession::SrpBeginSharedL()
    {
    BnExpFinish( iSrp->iA, iSrp->iAcc, iSrp->iP, iSrp->iN0inv );
    TBn one;
    one.Zero();
    one.iD[ 0 ] = 1;
    if ( iSrp->iA.IsZero() || iSrp->iA.Cmp( one ) <= 0 ||
         iSrp->iA.Cmp( iSrp->iP ) >= 0 )
        {
        _LIT( KSrpA, "SRP A" );
        FailTextL( KSrpA );
        return 0;
        }

    const TInt pLen = iSrpP.Length();
    iSrp->iA.ToBe( iSrp->iAp, pLen );

    TUint8 gp[ 256 ];
    iSrp->iG.ToBe( gp, pLen );
    TUint8 pg[ 512 ];
    Mem::Copy( pg, iSrpP.Ptr(), pLen );
    Mem::Copy( pg + pLen, gp, pLen );
    TUint8 kbytes[ 32 ];
    Sha256( pg, pLen * 2, kbytes );
    iSrp->iK.FromBe( kbytes, 32 );

    TUint8 Bp[ 256 ];
    PadBe( Bp, pLen, iSrpB.Ptr(), iSrpB.Length() );
    TUint8 au[ 512 ];
    Mem::Copy( au, iSrp->iAp, pLen );
    Mem::Copy( au + pLen, Bp, pLen );
    TUint8 ubytes[ 32 ];
    Sha256( au, pLen * 2, ubytes );
    iSrp->iU.FromBe( ubytes, 32 );
    if ( iSrp->iU.IsZero() )
        {
        _LIT( KSrpU, "SRP U" );
        FailTextL( KSrpU );
        return 0;
        }

    iSrp->iB.FromBe( iSrpB.Ptr(), iSrpB.Length() );
    if ( iSrp->iB.IsZero() || iSrp->iB.Cmp( one ) <= 0 ||
         iSrp->iB.Cmp( iSrp->iP ) >= 0 )
        {
        _LIT( KSrpB, "SRP B" );
        FailTextL( KSrpB );
        return 0;
        }

    TBn kgx;
    TBn base;
    BnMulMod( kgx, iSrp->iK, iSrp->iGx, iSrp->iP );
    BnSubMod( base, iSrp->iB, kgx, iSrp->iP );

    TUint32 ux[ 16 ];
    Mem::FillZ( ux, sizeof( ux ) );
    TInt ii = 0;
    for ( ii = 0; ii < 8; ii++ )
        {
        TUint64 c = 0;
        TInt jj = 0;
        for ( jj = 0; jj < 8; jj++ )
            {
            c += (TUint64)ux[ ii + jj ]
               + (TUint64)iSrp->iU.iD[ ii ] * iSrp->iX.iD[ jj ];
            ux[ ii + jj ] = (TUint32)c;
            c >>= 32;
            }
        ux[ ii + 8 ] += (TUint32)c;
        }

    TUint32 exp[ 80 ];
    TInt i = 0;
    for ( i = 0; i < 80; i++ )
        {
        exp[ i ] = ( i < KBnLimbs ) ? iSrp->iAexp.iD[ i ] : 0;
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

    iSrp->iStage = 3;
    BeginSrpExp( base, exp, 80 );
    ReportSrpProgressL();
    return 1;
    }

void CSymgramSession::FinishSrpSendL()
    {
    const TInt pLen = iSrpP.Length();
    TUint8 Sp[ 256 ];
    iSrp->iS.ToBe( Sp, pLen );
    TUint8 Khash[ 32 ];
    Sha256( Sp, pLen, Khash );

    TUint8 gp[ 256 ];
    iSrp->iG.ToBe( gp, pLen );
    TUint8 hp[ 32 ];
    TUint8 hg[ 32 ];
    Sha256( iSrpP.Ptr(), pLen, hp );
    Sha256( gp, pLen, hg );
    TUint8 xh[ 32 ];
    TInt i = 0;
    for ( i = 0; i < 32; i++ )
        {
        xh[ i ] = (TUint8)( hp[ i ] ^ hg[ i ] );
        }
    TUint8 hs1[ 32 ];
    TUint8 hs2[ 32 ];
    Sha256( iSalt1.Ptr(), iSalt1.Length(), hs1 );
    Sha256( iSalt2.Ptr(), iSalt2.Length(), hs2 );

    TUint8 m1in[ 640 ];
    TInt n = 0;
    Mem::Copy( m1in + n, xh, 32 );
    n += 32;
    Mem::Copy( m1in + n, hs1, 32 );
    n += 32;
    Mem::Copy( m1in + n, hs2, 32 );
    n += 32;
    Mem::Copy( m1in + n, iSrp->iAp, pLen );
    n += pLen;
    TUint8 Bp[ 256 ];
    PadBe( Bp, pLen, iSrpB.Ptr(), iSrpB.Length() );
    Mem::Copy( m1in + n, Bp, pLen );
    n += pLen;
    Mem::Copy( m1in + n, Khash, 32 );
    n += 32;
    TUint8 m1[ 32 ];
    Sha256( m1in, n, m1 );

    TBuf8<256> A;
    TInt off = 0;
    while ( off < pLen - 1 && iSrp->iAp[ off ] == 0 )
        {
        off++;
        }
    A.Append( iSrp->iAp + off, pLen - off );
    TBuf8<32> M1;
    M1.Append( m1, 32 );
    ClearSrp();

    TBuf8<384> query;
    PutU32( query, KCheckPassword );
    PutU32( query, KInputSrp );
    PutU64( query, (TUint64)iSrpId );
    TlAppendBytes( query, A.Ptr(), A.Length() );
    TlAppendBytes( query, M1.Ptr(), M1.Length() );

    TBuf8<768> wrapped;
    WrapInitL( query, wrapped );
    iLastRpc.Copy( wrapped );
    iResumeTries = 0;
    if ( iSocket.SubSessionHandle() )
        {
        SendEncryptedL( wrapped );
        }
    else
        {
        ResumeConnectL();
        }
    }

void CSymgramSession::RunL()
    {
    if ( iStatus.Int() != KErrNone )
        {
        if ( iState == EStarting &&
             iPref.DialogPreference() == ECommDbDialogPrefDoNotPrompt &&
             iStatus.Int() != KErrCancel )
            {
            iPref.SetDialogPreference( ECommDbDialogPrefPrompt );
            iConn.Close();
            User::LeaveIfError( iConn.Open( iServ ) );
            iObserver.SessionStatusL( KStatusIap );
            iConn.Start( iPref, iStatus );
            SetActive();
            return;
            }
        if ( iState == EStarting && iStatus.Int() != KErrCancel )
            {
            // EKA2L1 stubs RConnection; TCP still works without an IAP.
            iConn.Close();
            BeginTcpConnectL();
            return;
            }
        if ( ( iState == EConnecting || iState == EResuming ) &&
             ShouldRetryAddrSwap( iStatus.Int() ) )
            {
            iTriedAddrSwap = ETrue;
            iSwapDcAddr = !iSwapDcAddr;
            BeginTcpConnectL();
            return;
            }
        if ( CanResume( iStatus.Int() ) )
            {
            ResumeConnectL();
            return;
            }
        FailL( iStatus.Int() );
        return;
        }

    switch ( iState )
        {
        case EStarting:
            {
            BeginTcpConnectL();
            break;
            }
        case EConnecting:
        case EResuming:
            {
            TInt keep = 1;
            iSocket.SetOpt( KSoTcpKeepAlive, KSolInetTcp, keep );
            if ( iResume && iPhase >= 3 && iLastRpc.Length() > 0 )
                {
                iResume = EFalse;
                SendEncryptedL( iLastRpc );
                }
            else if ( iPhase >= 3 && iAuthKeyId != 0 )
                {
                iResume = EFalse;
                iObserver.SessionSignedInL();
                SendGetDialogsL();
                }
            else
                {
                iResume = EFalse;
                SendPqL();
                }
            break;
            }
        case EWriting:
            iHave = 0;
            iIn->Des().Zero();
            if ( !iAuthed )
                {
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
                }
            ReadMoreL();
            break;
        case EReading:
            iHave += iRead.Length();
            iIn->Des().SetLength( iHave );
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
    if ( 8 + aPayload.Length() > iOut.MaxLength() )
        {
        _LIT( KBig, "Пакет слишком большой" );
        FailTextL( KBig );
        return;
        }
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

void CSymgramSession::EnsureInL( TInt aNeed )
    {
    if ( aNeed <= iIn->Des().MaxLength() )
        {
        return;
        }
    if ( aNeed > KMaxIn )
        {
        _LIT( KBig, "Пакет слишком большой" );
        FailTextL( KBig );
        return;
        }
    TInt cap = iIn->Des().MaxLength();
    while ( cap < aNeed )
        {
        cap *= 2;
        }
    if ( cap > KMaxIn )
        {
        cap = KMaxIn;
        }
    iIn = iIn->ReAllocL( cap );
    }

void CSymgramSession::ReadMoreL()
    {
    if ( iHave >= iIn->Des().MaxLength() )
        {
        EnsureInL( iHave + 2048 );
        if ( iHave >= iIn->Des().MaxLength() )
            {
            return;
            }
        }

    iRead.Set( const_cast<TUint8*>( iIn->Ptr() ) + iHave,
               0,
               iIn->Des().MaxLength() - iHave );
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
        const TUint8 first = ( *iIn )[ 0 ];
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
            words = ( *iIn )[ 1 ]
                  | ( ( *iIn )[ 2 ] << 8 )
                  | ( ( *iIn )[ 3 ] << 16 );
            header = 4;
            }
        else
            {
            words = first;
            }

        const TInt need = header + words * 4;
        if ( need <= header )
            {
            FailL( KErrCorrupt );
            return;
            }
        if ( need > iIn->Des().MaxLength() )
            {
            EnsureInL( need );
            if ( need > iIn->Des().MaxLength() )
                {
                return;
                }
            }
        if ( iHave < need )
            {
            ReadMoreL();
            return;
            }

        const TUint8* p = iIn->Ptr() + header;
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
            Mem::Move( const_cast<TUint8*>( iIn->Ptr() ),
                       iIn->Ptr() + need, left );
            }
        iHave = left;
        iIn->Des().SetLength( iHave );

        if ( iState == EWriting )
            {
            return;
            }
        if ( iState == EIdle && iPhase < 3 )
            {
            return;
            }
        if ( iState == EIdle )
            {
            ReadMoreL();
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
    PutU32( inner, iDcId );

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

    TDhWork* dh = new ( ELeave ) TDhWork;
    CleanupStack::PushL( dh );
    dh->iP.FromBe( prime, primeLen );
    dh->iG.Zero();
    dh->iG.iD[ 0 ] = g;
    dh->iGa.FromBe( ga, gaLen );
    if ( dh->iGa.IsZero() || dh->iGa.Cmp( dh->iP ) >= 0 )
        {
        CleanupStack::PopAndDestroy( 2, dec );
        _LIT( KDhGa, "DH Ga" );
        FailTextL( KDhGa );
        return;
        }

    TPtr8 bb( dh->iBbytes, 256, 256 );
    bb.SetLength( 256 );
    GenerateRandomBytesL( bb );
    dh->iB.FromBe( dh->iBbytes, 256 );

    BnModExp( dh->iGb, dh->iG, dh->iB, dh->iP );
    BnModExp( dh->iAk, dh->iGa, dh->iB, dh->iP );

    dh->iAk.ToBe( dh->iAkb, 256 );
    iAuthKey.Copy( dh->iAkb, 256 );

    dh->iGb.ToBe( dh->iGbb, 256 );
    TInt goff = 0;
    while ( goff < 255 && dh->iGbb[ goff ] == 0 )
        {
        goff++;
        }

    TBuf8<400> inner;
    PutU32( inner, KClientDhInner );
    inner.Append( iNonce );
    inner.Append( iServerNonce );
    PutU64( inner, 0 );
    TlAppendBytes( inner, dh->iGbb + goff, 256 - goff );

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
    CleanupStack::PopAndDestroy( 2, dec );
    iPhase = 2;
    WritePacketL( payload );
    }

void CSymgramSession::HandleDhGenL( const TUint8* aBody, TInt aLen )
    {
    if ( aLen < 52 || GetU32( aBody ) != KDhGenOk )
        {
        _LIT( KDhOk, "DH ответ" );
        FailTextL( KDhOk );
        return;
        }
    if ( Mem::Compare( aBody + 4, 16, iNonce.Ptr(), 16 ) != 0 ||
         Mem::Compare( aBody + 20, 16, iServerNonce.Ptr(), 16 ) != 0 )
        {
        _LIT( KDhNonce, "DH nonce" );
        FailTextL( KDhNonce );
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
        _LIT( KDhHash, "Ключ DH" );
        FailTextL( KDhHash );
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
    const TInt inner = 32 + aMsg.Length();
    TInt pad = 16 - ( inner % 16 );
    if ( pad < 12 )
        {
        pad += 16;
        }
    const TInt plainLen = inner + pad;
    HBufC8* plainBuf = HBufC8::NewLC( plainLen );
    TPtr8 plain = plainBuf->Des();
    PutU64( plain, iSalt );
    PutU64( plain, iSessionId );
    PutU64( plain, MessageId() );
    PutU32( plain, SeqNo() );
    PutU32( plain, aMsg.Length() );
    plain.Append( aMsg );
    TBuf8<32> padding;
    padding.SetLength( pad );
    GenerateRandomBytesL( padding );
    plain.Append( padding );

    HBufC8* mkBuf = HBufC8::NewLC( 32 + plain.Length() );
    TPtr8 mk = mkBuf->Des();
    mk.Append( iAuthKey.Ptr() + 88, 32 );
    mk.Append( plain );
    TUint8 sha[ 32 ];
    Sha256( mk.Ptr(), mk.Length(), sha );
    TUint8 msgKey[ 16 ];
    Mem::Copy( msgKey, sha + 8, 16 );

    TUint8 aesKey[ 32 ];
    TUint8 aesIv[ 32 ];
    DeriveAes( 0, msgKey, aesKey, aesIv );

    HBufC8* encBuf = HBufC8::NewLC( plain.Length() );
    TPtr8 enc = encBuf->Des();
    enc.SetLength( plain.Length() );
    Aes256IgeEncrypt( plain.Ptr(), const_cast<TUint8*>( enc.Ptr() ),
                      plain.Length(), aesKey, aesIv );

    HBufC8* packetBuf = HBufC8::NewLC( 24 + enc.Length() );
    TPtr8 packet = packetBuf->Des();
    PutU64( packet, iAuthKeyId );
    packet.Append( msgKey, 16 );
    packet.Append( enc );
    WritePacketL( packet );
    CleanupStack::PopAndDestroy( 4, plainBuf );
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

void CSymgramSession::SendGetDialogsL()
    {
    TBuf8<48> query;
    PutU32( query, KGetDialogs );
    PutU32( query, 0 );
    PutU32( query, 0 );
    PutU32( query, 0 );
    PutU32( query, KInputPeerEmpty );
    PutU32( query, 20 );
    PutU32( query, 0 );
    PutU32( query, 0 );
    TBuf8<384> wrapped;
    WrapInitL( query, wrapped );
    iRpc = ERpcDialogs;
    iObserver.SessionStatusL( KStatusChats );
    SendRpcL( wrapped );
    }

void CSymgramSession::HandleDialogsL( const TUint8* aP, TInt aLen )
    {
    TInt o = 0;
    if ( aLen < 8 )
        {
        SendGetContactsL();
        return;
        }
    const TUint32 root = GetU32( aP );
    o = 4;
    if ( root == KDialogsSlice )
        {
        if ( Need( o, aLen, 4 ) != KErrNone )
            {
            SendGetContactsL();
            return;
            }
        o += 4;
        }
    else if ( root != KDialogs )
        {
        SendGetContactsL();
        return;
        }

    if ( Need( o, aLen, 8 ) != KErrNone || GetU32( aP + o ) != KTlVector )
        {
        SendGetContactsL();
        return;
        }
    const TInt dcount = (TInt)GetU32( aP + o + 4 );
    o += 8;

    struct TDlg
        {
        TInt iKind;
        TInt64 iId;
        TInt64 iHash;
        TInt iUnread;
        TInt iTopMsg;
        };
    TDlg dlg[ 30 ];
    TInt got = 0;
    TInt i = 0;
    for ( i = 0; i < dcount && got < 30; i++ )
        {
        if ( Need( o, aLen, 8 ) != KErrNone )
            {
            break;
            }
        const TUint32 c = GetU32( aP + o );
        o += 4;
        if ( c == 0x71bd134cu )
            {
            break;
            }
        const TUint32 flags = GetU32( aP + o );
        o += 4;
        TInt kind = 0;
        TInt64 id = 0;
        if ( SkipPeer( aP, aLen, o, kind, id ) != KErrNone )
            {
            break;
            }
        if ( Need( o, aLen, 24 ) != KErrNone )
            {
            break;
            }
        const TInt topMsg = GetI32( aP + o );
        o += 12;
        const TInt unread = (TInt)GetU32( aP + o );
        o += 12;
        if ( SkipNotify( aP, aLen, o ) != KErrNone )
            {
            break;
            }
        if ( ( flags & 1 ) != 0 )
            {
            if ( Need( o, aLen, 4 ) != KErrNone )
                {
                break;
                }
            o += 4;
            }
        if ( ( flags & 2 ) != 0 )
            {
            if ( Need( o, aLen, 4 ) != KErrNone )
                {
                break;
                }
            o += 4;
            if ( SkipBytes( aP, aLen, o ) != KErrNone )
                {
                break;
                }
            }
        if ( ( flags & 16 ) != 0 )
            {
            if ( Need( o, aLen, 4 ) != KErrNone )
                {
                break;
                }
            o += 4;
            }
        if ( ( flags & 32 ) != 0 )
            {
            if ( Need( o, aLen, 4 ) != KErrNone )
                {
                break;
                }
            o += 4;
            }
        dlg[ got ].iKind = kind;
        dlg[ got ].iId = id;
        dlg[ got ].iHash = 0;
        dlg[ got ].iUnread = unread;
        dlg[ got ].iTopMsg = topMsg;
        got++;
        }

    struct TMsg
        {
        TInt iId;
        TInt iDate;
        TBuf<80> iText;
        };
    TMsg msgs[ 30 ];
    TInt nmsgs = 0;
    const TInt afterDialogs = o;
    if ( Need( o, aLen, 8 ) == KErrNone && GetU32( aP + o ) == KTlVector )
        {
        const TInt mcount = (TInt)GetU32( aP + o + 4 );
        o += 8;
        TInt scan = o;
        while ( scan + 16 <= aLen && nmsgs < 30 && nmsgs < mcount )
            {
            const TUint32 c = GetU32( aP + scan );
            if ( c == 0x38116ee0u || c == 0x2b085862u ||
                 c == 0x90a6ca84u || c == 0x83e5de54u )
                {
                TInt t = scan;
                TInt id = 0;
                TInt date = 0;
                TBuf<80> text;
                if ( ReadMessagePreview( aP, aLen, t, id, date, text ) ==
                     KErrNone )
                    {
                    if ( id != 0 &&
                         date > 1000000000 && date < 2000000000 )
                        {
                        msgs[ nmsgs ].iId = id;
                        msgs[ nmsgs ].iDate = date;
                        msgs[ nmsgs ].iText.Copy(
                            text.Left( msgs[ nmsgs ].iText.MaxLength() ) );
                        nmsgs++;
                        }
                    scan = t;
                    continue;
                    }
                }
            scan += 4;
            }
        o = afterDialogs;
        }

    TPeerName names[ 40 ];
    TInt nnames = 0;
    TInt hunt = afterDialogs;
    while ( hunt + 16 <= aLen )
        {
        if ( GetU32( aP + hunt ) == KTlVector )
            {
            const TUint32 next = GetU32( aP + hunt + 8 );
            if ( IsChatCtor( next ) || IsUserCtor( next ) )
                {
                break;
                }
            }
        hunt += 4;
        }
    o = hunt;
    if ( hunt + 16 <= aLen && GetU32( aP + hunt ) == KTlVector &&
         IsChatCtor( GetU32( aP + hunt + 8 ) ) )
        {
        const TInt ccount = (TInt)GetU32( aP + hunt + 4 );
        o = hunt + 8;
        TInt ci = 0;
        for ( ci = 0; ci < ccount && nnames < 40; ci++ )
            {
            TPeerName one;
            if ( !ConsumeChat( aP, aLen, o, one ) )
                {
                break;
                }
            names[ nnames ].iId = one.iId;
            names[ nnames ].iHash = one.iHash;
            names[ nnames ].iKind = one.iKind;
            names[ nnames ].iName.Copy( one.iName );
            names[ nnames ].iSelf = EFalse;
            names[ nnames ].iDeleted = EFalse;
            nnames++;
            }
        }
    if ( !( Need( o, aLen, 8 ) == KErrNone && GetU32( aP + o ) == KTlVector &&
            IsUserCtor( GetU32( aP + o + 8 ) ) ) )
        {
        hunt = o;
        while ( hunt + 16 <= aLen )
            {
            if ( GetU32( aP + hunt ) == KTlVector &&
                 IsUserCtor( GetU32( aP + hunt + 8 ) ) )
                {
                o = hunt;
                break;
                }
            hunt += 4;
            }
        }
    if ( Need( o, aLen, 8 ) == KErrNone && GetU32( aP + o ) == KTlVector &&
         IsUserCtor( GetU32( aP + o + 8 ) ) )
        {
        const TInt ucount = (TInt)GetU32( aP + o + 4 );
        o += 8;
        TInt ui = 0;
        for ( ui = 0; ui < ucount && nnames < 40; ui++ )
            {
            TPeerName one;
            if ( !ConsumeUser( aP, aLen, o, one ) )
                {
                break;
                }
            names[ nnames ].iId = one.iId;
            names[ nnames ].iHash = one.iHash;
            names[ nnames ].iKind = one.iKind;
            names[ nnames ].iName.Copy( one.iName );
            names[ nnames ].iSelf = one.iSelf;
            names[ nnames ].iDeleted = one.iDeleted;
            nnames++;
            }
        }

    for ( i = 0; i < got; i++ )
        {
        TSymgramChat chat;
        chat.iId = dlg[ i ].iId;
        chat.iHash = dlg[ i ].iHash;
        chat.iPeerKind = dlg[ i ].iKind;
        chat.iUnread = dlg[ i ].iUnread;
        chat.iDate = 0;
        _LIT( KDlg, "Диалог" );
        chat.iName.Copy( KDlg );
        TInt k = 0;
        for ( k = 0; k < nnames; k++ )
            {
            if ( names[ k ].iId == dlg[ i ].iId &&
                 NameFits( dlg[ i ].iKind, names[ k ].iKind ) )
                {
                if ( names[ k ].iName.Length() > 0 )
                    {
                    chat.iName.Copy( names[ k ].iName );
                    }
                if ( names[ k ].iHash != 0 )
                    {
                    chat.iHash = names[ k ].iHash;
                    }
                if ( names[ k ].iKind != 0 )
                    {
                    chat.iPeerKind = names[ k ].iKind;
                    }
                break;
                }
            }
        TInt m = 0;
        for ( m = 0; m < nmsgs; m++ )
            {
            if ( msgs[ m ].iId == dlg[ i ].iTopMsg )
                {
                FormatHm( msgs[ m ].iDate, chat.iTime );
                chat.iDate = msgs[ m ].iDate;
                if ( msgs[ m ].iText.Length() > 0 )
                    {
                    chat.iPreview.Copy( msgs[ m ].iText );
                    }
                break;
                }
            }
        iObserver.SessionAddChatL( chat );
        }
    SaveSession();
    iObserver.SessionChatsReadyL();
    SendGetContactsL();
    }

void CSymgramSession::SendGetContactsL()
    {
    TBuf8<16> query;
    PutU32( query, KGetContacts );
    PutU64( query, 0 );
    TBuf8<384> wrapped;
    WrapInitL( query, wrapped );
    iRpc = ERpcContacts;
    iObserver.SessionStatusL( KStatusContacts );
    SendRpcL( wrapped );
    }

void CSymgramSession::HandleContactsL( const TUint8* aP, TInt aLen )
    {
    TInt o = 0;
    _LIT( KOn, "В сети" );
    if ( aLen < 4 )
        {
        iBusy = EFalse;
        iState = EIdle;
        iObserver.SessionStatusL( KOn );
        return;
        }
    const TUint32 root = GetU32( aP );
    o = 4;
    TBool hasSaved = EFalse;
    if ( root == KContacts )
        {
        hasSaved = ETrue;
        }
    else if ( root != KContactsOld )
        {
        iBusy = EFalse;
        iState = EIdle;
        iObserver.SessionStatusL( KOn );
        return;
        }

    if ( Need( o, aLen, 8 ) != KErrNone || GetU32( aP + o ) != KTlVector )
        {
        iBusy = EFalse;
        iState = EIdle;
        iObserver.SessionStatusL( KOn );
        return;
        }
    const TInt ncont = (TInt)GetU32( aP + o + 4 );
    o += 8;
    TInt i = 0;
    for ( i = 0; i < ncont; i++ )
        {
        if ( Need( o, aLen, 4 ) != KErrNone )
            {
            break;
            }
        const TUint32 c = GetU32( aP + o );
        o += 4;
        if ( c == KContact )
            {
            if ( Need( o, aLen, 12 ) != KErrNone )
                {
                break;
                }
            o += 12;
            }
        else if ( c == KContactOld )
            {
            if ( Need( o, aLen, 8 ) != KErrNone )
                {
                break;
                }
            o += 8;
            }
        else
            {
            break;
            }
        }
    if ( hasSaved )
        {
        if ( Need( o, aLen, 4 ) != KErrNone )
            {
            iBusy = EFalse;
            iState = EIdle;
            iObserver.SessionStatusL( KOn );
            return;
            }
        o += 4;
        }
    if ( Need( o, aLen, 8 ) != KErrNone || GetU32( aP + o ) != KTlVector )
        {
        iBusy = EFalse;
        iState = EIdle;
        iObserver.SessionStatusL( KOn );
        return;
        }
    const TInt nusers = (TInt)GetU32( aP + o + 4 );
    o += 8;
    iObserver.SessionClearContactsL();
    for ( i = 0; i < nusers; i++ )
        {
        TPeerName one;
        if ( !ConsumeUser( aP, aLen, o, one ) )
            {
            break;
            }
        if ( one.iSelf || one.iDeleted || one.iId == 0 )
            {
            continue;
            }
        TSymgramContact cont;
        cont.iId = one.iId;
        cont.iHash = one.iHash;
        if ( one.iName.Length() == 0 )
            {
            _LIT( KUser, "Контакт" );
            cont.iName.Copy( KUser );
            }
        else
            {
            cont.iName.Copy( one.iName );
            }
        iObserver.SessionAddContactL( cont );
        }
    iBusy = EFalse;
    iState = EIdle;
    iRpc = ERpcNone;
    iObserver.SessionStatusL( KOn );
    iObserver.SessionContactsReadyL();
    FlushPendingL();
    }

void CSymgramSession::HandleHistoryL( const TUint8* aP, TInt aLen )
    {
    TInt o = 4;
    const TUint32 root = GetU32( aP );
    if ( root == KMessagesSlice )
        {
        o += 4;
        }
    else if ( root == KChannelMsgs )
        {
        o += 12;
        }
    else if ( root != KMessages )
        {
        iBusy = EFalse;
        iRpc = ERpcNone;
        FlushPendingL();
        return;
        }
    if ( Need( o, aLen, 8 ) != KErrNone || GetU32( aP + o ) != KTlVector )
        {
        iBusy = EFalse;
        iRpc = ERpcNone;
        FlushPendingL();
        return;
        }
    const TInt mcount = (TInt)GetU32( aP + o + 4 );
    o += 8;
    iObserver.SessionBeginMessagesL( iPeerId );
    TInt scan = o;
    TInt got = 0;
    TPreview buf[ 12 ];
    while ( scan + 16 <= aLen && got < 12 && got < mcount )
        {
        const TUint32 c = GetU32( aP + scan );
        if ( c == 0x38116ee0u || c == 0x2b085862u ||
             c == 0x90a6ca84u || c == 0x83e5de54u )
            {
            TInt t = scan;
            TPreview one;
            if ( ReadMessage( aP, aLen, t, one ) == KErrNone )
                {
                if ( one.iId != 0 &&
                     one.iDate > 1000000000 && one.iDate < 2000000000 )
                    {
                    buf[ got ] = one;
                    got++;
                    }
                scan = t;
                continue;
                }
            }
        scan += 4;
        }
    TInt i = 0;
    for ( i = got - 1; i >= 0; i-- )
        {
        TSymgramMsg m;
        m.iId = buf[ i ].iId;
        m.iDate = buf[ i ].iDate;
        m.iOut = buf[ i ].iOut;
        m.iKind = buf[ i ].iKind;
        m.iDuration = buf[ i ].iDuration;
        m.iPhotoLoc = buf[ i ].iPhotoLoc;
        m.iFileId = buf[ i ].iFileId;
        m.iFileHash = buf[ i ].iFileHash;
        m.iFileRef.Copy( buf[ i ].iFileRef );
        m.iFileName.Copy( buf[ i ].iFileName );
        m.iText.Copy( buf[ i ].iText.Left( m.iText.MaxLength() ) );
        m.iBmp = NULL;
        iObserver.SessionAddMessageL( m );
        }
    iBusy = EFalse;
    iRpc = ERpcNone;
    iState = EIdle;
    _LIT( KOn, "В сети" );
    iObserver.SessionStatusL( KOn );
    iObserver.SessionMessagesReadyL( iPeerId );
    FlushPendingL();
    }

void CSymgramSession::HandleFileL( const TUint8* aP, TInt aLen )
    {
    if ( aLen < 4 )
        {
        ClearSave();
        iBusy = EFalse;
        iRpc = ERpcNone;
        FlushPendingL();
        return;
        }
    const TUint32 root = GetU32( aP );
    if ( root == KUploadFileCdn )
        {
        ClearSave();
        iBusy = EFalse;
        iRpc = ERpcNone;
        iState = EIdle;
        _LIT( KCdn, "Файл на CDN" );
        iObserver.SessionStatusL( KCdn );
        FlushPendingL();
        return;
        }
    if ( root != KUploadFile || aLen < 16 )
        {
        ClearSave();
        iBusy = EFalse;
        iRpc = ERpcNone;
        FlushPendingL();
        return;
        }
    TInt o = 12;
    const TUint8* data = NULL;
    TInt len = 0;
    if ( TlReadBytes( aP + o, aLen - o, data, len ) < 0 )
        {
        ClearSave();
        iBusy = EFalse;
        iRpc = ERpcNone;
        FlushPendingL();
        return;
        }
    if ( iSaveFull )
        {
        if ( len > 0 )
            {
            TPtrC8 chunk( data, len );
            if ( iSaveFile.Write( chunk ) != KErrNone ||
                 iSaveOff + len > KDownloadMax )
                {
                ClearSave();
                iBusy = EFalse;
                iRpc = ERpcNone;
                iState = EIdle;
                _LIT( KBig, "Файл слишком большой" );
                iObserver.SessionStatusL( KBig );
                FlushPendingL();
                return;
                }
            iSaveOff += len;
            }
        if ( len >= KDownloadChunk )
            {
            SendGetFileNowL();
            return;
            }
        iSaveFile.Close();
        TFileName path;
        path.Copy( iSavePath );
        const TBool open = iSaveOpen;
        iSaveFull = EFalse;
        iSaveOpen = EFalse;
        iSaveOff = 0;
        iBusy = EFalse;
        iRpc = ERpcNone;
        iState = EIdle;
        iObserver.SessionFileSavedL( path, open );
        FlushPendingL();
        return;
        }
    if ( len < 8 )
        {
        iBusy = EFalse;
        iRpc = ERpcNone;
        FlushPendingL();
        return;
        }
    TPtrC8 jpeg( data, len );
    iObserver.SessionThumbL( iFileMsgId, jpeg );
    iBusy = EFalse;
    iRpc = ERpcNone;
    iState = EIdle;
    _LIT( KOn, "В сети" );
    iObserver.SessionStatusL( KOn );
    FlushPendingL();
    }

void CSymgramSession::HandleSentL( const TUint8* aP, TInt aLen )
    {
    TInt id = 0;
    TInt date = UnixNow() + iTimeOffset;
    if ( aLen >= 24 && GetU32( aP ) == KUpdShortSent )
        {
        id = GetI32( aP + 8 );
        date = GetI32( aP + 20 );
        }
    ClearUpload();
    iBusy = EFalse;
    iRpc = ERpcNone;
    iState = EIdle;
    _LIT( KOn, "В сети" );
    iObserver.SessionStatusL( KOn );
    iObserver.SessionSentL( id, date );
    FlushPendingL();
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

    TBuf<80> hint;
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
    iResumeTries = 0;
    iBusy = EFalse;
    iState = EIdle;
    if ( iPwdUtf.Length() > 0 && iSrpTries < 2 )
        {
        iSrpTries++;
        SubmitPasswordL( iPwdUtf );
        return;
        }
    iSrpTries = 0;
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
        _LIT( KCrypt, "Шифр пакета" );
        FailTextL( KCrypt );
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
    HBufC8* raw = HBufC8::NewLC( 65536 );
    TPtr8 rp = raw->Des();
    rp.SetLength( 65536 );
    TInt n = 0;
    if ( InflateTlGzip( packed, plen, const_cast<TUint8*>( rp.Ptr() ),
                        65536, n ) != KErrNone || n < 4 )
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
        const TInt migrate = MigrateDcFromError( text );
        if ( migrate != 0 )
            {
            MigrateToDcL( migrate );
            return;
            }
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
        if ( text == KBadPwdErr )
            {
            iPwdUtf.Zero();
            iSrpTries = 0;
            iBusy = EFalse;
            iState = EIdle;
            iRpc = ERpcNone;
            _LIT( KBad, "Неверный пароль" );
            iObserver.SessionErrorL( KBad );
            return;
            }
        RpcFailL( text );
        return;
        }
    if ( iRpc == ERpcSavePart )
        {
        if ( c == KBoolTrue )
            {
            iUploadPart++;
            if ( iUploadPart < iUploadParts )
                {
                SendSavePartNowL();
                }
            else
                {
                SendMediaNowL();
                }
            return;
            }
        ClearUpload();
        iBusy = EFalse;
        iRpc = ERpcNone;
        iState = EIdle;
        _LIT( KFail, "Не удалось загрузить" );
        iObserver.SessionStatusL( KFail );
        FlushPendingL();
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
        iResumeTries = 0;
        iObserver.SessionCodeSentL();
        return;
        }
    if ( c == KAuthOk || c == KAuthOkOld )
        {
        iResumeTries = 0;
        iAuthed = ETrue;
        iPwdUtf.Zero();
        iSrpTries = 0;
        SaveSession();
        iObserver.SessionSignedInL();
        SendGetDialogsL();
        return;
        }
    if ( c == KDialogs || c == KDialogsSlice )
        {
        HandleDialogsL( p, n );
        return;
        }
    if ( c == KContacts || c == KContactsOld )
        {
        HandleContactsL( p, n );
        return;
        }
    if ( c == KContactsNotMod )
        {
        iBusy = EFalse;
        iState = EIdle;
        iRpc = ERpcNone;
        _LIT( KOn, "В сети" );
        iObserver.SessionStatusL( KOn );
        iObserver.SessionContactsReadyL();
        FlushPendingL();
        return;
        }
    if ( c == KMessages || c == KMessagesSlice || c == KChannelMsgs )
        {
        HandleHistoryL( p, n );
        return;
        }
    if ( c == KUploadFile || c == KUploadFileCdn )
        {
        HandleFileL( p, n );
        return;
        }
    if ( c == KUpdShortSent || c == KUpdates || c == KUpdatesComb ||
         c == KUpdateShort || c == KUpdShortMsg || c == KUpdShortChat ||
         c == KUpdatesTooLong )
        {
        if ( iRpc == ERpcSend )
            {
            HandleSentL( p, n );
            }
        else
            {
            iBusy = EFalse;
            iRpc = ERpcNone;
            iState = EIdle;
            _LIT( KOn, "В сети" );
            iObserver.SessionStatusL( KOn );
            FlushPendingL();
            }
        return;
        }
    if ( c == KAccountPwd || c == KAccountPwdOld || c == KAccountPwdSrp )
        {
        HandlePasswordL( p, n );
        return;
        }
    // sendMessage always returns some Updates object; treat an unknown
    // constructor as success rather than dropping the TCP session.
    if ( iRpc == ERpcSend )
        {
        HandleSentL( p, n );
        return;
        }
    if ( iAuthed )
        {
        iBusy = EFalse;
        iRpc = ERpcNone;
        iState = EIdle;
        TBuf<48> text;
        _LIT( KUnk, "Ответ " );
        text.Copy( KUnk );
        text.AppendNum( (TInt64)c, EHex );
        iObserver.SessionStatusL( text );
        FlushPendingL();
        return;
        }
    TBuf<48> text;
    _LIT( KUnkHs, "Ответ " );
    text.Copy( KUnkHs );
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
