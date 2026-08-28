#include "SymgramCrypto.h"

namespace
    {
    const TInt KMaxBits = 15;
    const TInt KMaxLit = 288;
    const TInt KMaxDist = 32;

    const TUint8 KLenExtra[ 29 ] =
        {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
        };
    const TUint16 KLenBase[ 29 ] =
        {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
        };
    const TUint8 KDistExtra[ 30 ] =
        {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
        };
    const TUint16 KDistBase[ 30 ] =
        {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577
        };
    const TUint8 KClenOrder[ 19 ] =
        {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
        };

    class TInf
        {
        public:
            const TUint8* iIn;
            TInt iInLen;
            TInt iInPos;
            TUint32 iAcc;
            TInt iBits;
            TUint8* iOut;
            TInt iOutMax;
            TInt iOutLen;

            TInt Need( TInt aBits );
            TUint32 Get( TInt aBits );
            void AlignByte();
        };

    TInt TInf::Need( TInt aBits )
        {
        while ( iBits < aBits )
            {
            if ( iInPos >= iInLen )
                {
                return KErrCorrupt;
                }
            iAcc |= ( (TUint32)iIn[ iInPos ] ) << iBits;
            iInPos++;
            iBits += 8;
            }
        return KErrNone;
        }

    TUint32 TInf::Get( TInt aBits )
        {
        const TUint32 mask = aBits ? ( ( (TUint32)1 << aBits ) - 1 ) : 0;
        const TUint32 v = iAcc & mask;
        iAcc >>= aBits;
        iBits -= aBits;
        return v;
        }

    void TInf::AlignByte()
        {
        while ( iBits >= 8 )
            {
            iInPos--;
            iBits -= 8;
            }
        iAcc = 0;
        iBits = 0;
        }

    class THuff
        {
        public:
            TInt16 iCount[ 16 ];
            TInt16 iSymbol[ KMaxLit ];

            TInt Build( const TUint8* aLen, TInt aN );
            TInt Decode( TInf& aS ) const;
        };

    TInt THuff::Build( const TUint8* aLen, TInt aN )
        {
        TInt n = 0;
        for ( n = 0; n <= KMaxBits; n++ )
            {
            iCount[ n ] = 0;
            }
        TInt sym = 0;
        for ( sym = 0; sym < aN; sym++ )
            {
            const TUint8 L = aLen[ sym ];
            if ( L > KMaxBits )
                {
                return KErrCorrupt;
                }
            iCount[ L ]++;
            }
        iCount[ 0 ] = 0;

        TInt left = 1;
        TInt len = 1;
        for ( len = 1; len <= KMaxBits; len++ )
            {
            left <<= 1;
            left -= iCount[ len ];
            if ( left < 0 )
                {
                return KErrCorrupt;
                }
            }

        TInt offs[ 16 ];
        offs[ 1 ] = 0;
        for ( len = 1; len < KMaxBits; len++ )
            {
            offs[ len + 1 ] = offs[ len ] + iCount[ len ];
            }
        for ( sym = 0; sym < aN; sym++ )
            {
            if ( aLen[ sym ] != 0 )
                {
                iSymbol[ offs[ aLen[ sym ] ]++ ] = (TInt16)sym;
                }
            }
        return KErrNone;
        }

    TInt THuff::Decode( TInf& aS ) const
        {
        TInt code = 0;
        TInt first = 0;
        TInt index = 0;
        TInt len = 1;
        for ( len = 1; len <= KMaxBits; len++ )
            {
            if ( aS.Need( 1 ) != KErrNone )
                {
                return -1;
                }
            code |= (TInt)aS.Get( 1 );
            const TInt count = iCount[ len ];
            if ( code - first < count )
                {
                return iSymbol[ index + ( code - first ) ];
                }
            index += count;
            first += count;
            first <<= 1;
            code <<= 1;
            }
        return -1;
        }

    TInt InflateBlock( TInf& aS, const THuff& aLit, const THuff& aDist )
        {
        for ( ;; )
            {
            const TInt sym = aLit.Decode( aS );
            if ( sym < 0 )
                {
                return KErrCorrupt;
                }
            if ( sym < 256 )
                {
                if ( aS.iOutLen >= aS.iOutMax )
                    {
                    return KErrOverflow;
                    }
                aS.iOut[ aS.iOutLen++ ] = (TUint8)sym;
                }
            else if ( sym == 256 )
                {
                return KErrNone;
                }
            else if ( sym > 285 )
                {
                return KErrCorrupt;
                }
            else
                {
                const TInt idx = sym - 257;
                if ( aS.Need( KLenExtra[ idx ] ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                const TInt copy = KLenBase[ idx ] + (TInt)aS.Get( KLenExtra[ idx ] );
                const TInt ds = aDist.Decode( aS );
                if ( ds < 0 || ds >= 30 )
                    {
                    return KErrCorrupt;
                    }
                if ( aS.Need( KDistExtra[ ds ] ) != KErrNone )
                    {
                    return KErrCorrupt;
                    }
                const TInt dist = KDistBase[ ds ] + (TInt)aS.Get( KDistExtra[ ds ] );
                if ( dist < 1 || dist > aS.iOutLen )
                    {
                    return KErrCorrupt;
                    }
                TInt n = 0;
                for ( n = 0; n < copy; n++ )
                    {
                    if ( aS.iOutLen >= aS.iOutMax )
                        {
                        return KErrOverflow;
                        }
                    aS.iOut[ aS.iOutLen ] = aS.iOut[ aS.iOutLen - dist ];
                    aS.iOutLen++;
                    }
                }
            }
        }

    TInt InflateStored( TInf& aS )
        {
        aS.AlignByte();
        if ( aS.iInPos + 4 > aS.iInLen )
            {
            return KErrCorrupt;
            }
        const TUint16 len = (TUint16)( aS.iIn[ aS.iInPos ] |
                                       ( aS.iIn[ aS.iInPos + 1 ] << 8 ) );
        const TUint16 nlen = (TUint16)( aS.iIn[ aS.iInPos + 2 ] |
                                        ( aS.iIn[ aS.iInPos + 3 ] << 8 ) );
        aS.iInPos += 4;
        if ( (TUint16)( len ^ nlen ) != 0xffff )
            {
            return KErrCorrupt;
            }
        if ( aS.iInPos + (TInt)len > aS.iInLen )
            {
            return KErrCorrupt;
            }
        if ( aS.iOutLen + (TInt)len > aS.iOutMax )
            {
            return KErrOverflow;
            }
        Mem::Copy( aS.iOut + aS.iOutLen, aS.iIn + aS.iInPos, len );
        aS.iOutLen += len;
        aS.iInPos += len;
        return KErrNone;
        }

    TInt InflateFixed( TInf& aS )
        {
        TUint8 lengths[ KMaxLit ];
        TInt s = 0;
        for ( s = 0; s < 144; s++ )
            {
            lengths[ s ] = 8;
            }
        for ( s = 144; s < 256; s++ )
            {
            lengths[ s ] = 9;
            }
        for ( s = 256; s < 280; s++ )
            {
            lengths[ s ] = 7;
            }
        for ( s = 280; s < 288; s++ )
            {
            lengths[ s ] = 8;
            }
        THuff lit;
        if ( lit.Build( lengths, 288 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        TUint8 distLen[ KMaxDist ];
        for ( s = 0; s < 30; s++ )
            {
            distLen[ s ] = 5;
            }
        THuff dist;
        if ( dist.Build( distLen, 30 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        return InflateBlock( aS, lit, dist );
        }

    TInt InflateDynamic( TInf& aS )
        {
        if ( aS.Need( 14 ) != KErrNone )
            {
            return KErrCorrupt;
            }
        const TInt hlit = (TInt)aS.Get( 5 ) + 257;
        const TInt hdist = (TInt)aS.Get( 5 ) + 1;
        const TInt hclen = (TInt)aS.Get( 4 ) + 4;
        if ( hlit > 286 || hdist > 32 || hclen > 19 )
            {
            return KErrCorrupt;
            }

        TUint8 clen[ 19 ];
        Mem::FillZ( clen, sizeof( clen ) );
        TInt i = 0;
        for ( i = 0; i < hclen; i++ )
            {
            if ( aS.Need( 3 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            clen[ KClenOrder[ i ] ] = (TUint8)aS.Get( 3 );
            }
        THuff cl;
        if ( cl.Build( clen, 19 ) != KErrNone )
            {
            return KErrCorrupt;
            }

        TUint8 lengths[ 286 + 32 ];
        Mem::FillZ( lengths, sizeof( lengths ) );
        TInt index = 0;
        const TInt total = hlit + hdist;
        while ( index < total )
            {
            const TInt sym = cl.Decode( aS );
            if ( sym < 0 )
                {
                return KErrCorrupt;
                }
            if ( sym < 16 )
                {
                lengths[ index++ ] = (TUint8)sym;
                }
            else
                {
                TInt rep = 0;
                TUint8 val = 0;
                if ( sym == 16 )
                    {
                    if ( index == 0 )
                        {
                        return KErrCorrupt;
                        }
                    val = lengths[ index - 1 ];
                    if ( aS.Need( 2 ) != KErrNone )
                        {
                        return KErrCorrupt;
                        }
                    rep = 3 + (TInt)aS.Get( 2 );
                    }
                else if ( sym == 17 )
                    {
                    if ( aS.Need( 3 ) != KErrNone )
                        {
                        return KErrCorrupt;
                        }
                    rep = 3 + (TInt)aS.Get( 3 );
                    }
                else if ( sym == 18 )
                    {
                    if ( aS.Need( 7 ) != KErrNone )
                        {
                        return KErrCorrupt;
                        }
                    rep = 11 + (TInt)aS.Get( 7 );
                    }
                else
                    {
                    return KErrCorrupt;
                    }
                if ( index + rep > total )
                    {
                    return KErrCorrupt;
                    }
                TInt r = 0;
                for ( r = 0; r < rep; r++ )
                    {
                    lengths[ index++ ] = val;
                    }
                }
            }

        THuff lit;
        if ( lit.Build( lengths, hlit ) != KErrNone )
            {
            return KErrCorrupt;
            }
        THuff dist;
        if ( dist.Build( lengths + hlit, hdist ) != KErrNone )
            {
            return KErrCorrupt;
            }
        return InflateBlock( aS, lit, dist );
        }

    TInt InflateRaw( TInf& aS )
        {
        TInt last = 0;
        do
            {
            if ( aS.Need( 3 ) != KErrNone )
                {
                return KErrCorrupt;
                }
            last = (TInt)aS.Get( 1 );
            const TInt type = (TInt)aS.Get( 2 );
            TInt err = KErrCorrupt;
            if ( type == 0 )
                {
                err = InflateStored( aS );
                }
            else if ( type == 1 )
                {
                err = InflateFixed( aS );
                }
            else if ( type == 2 )
                {
                err = InflateDynamic( aS );
                }
            if ( err != KErrNone )
                {
                return err;
                }
            }
        while ( !last );
        return KErrNone;
        }

    TInt SkipGzipHeader( const TUint8* aIn, TInt aLen, TInt& aOff )
        {
        if ( aLen < 10 || aIn[ 0 ] != 0x1f || aIn[ 1 ] != 0x8b || aIn[ 2 ] != 8 )
            {
            return KErrCorrupt;
            }
        const TUint8 flags = aIn[ 3 ];
        TInt o = 10;
        if ( flags & 4 )
            {
            if ( o + 2 > aLen )
                {
                return KErrCorrupt;
                }
            const TInt xlen = aIn[ o ] | ( aIn[ o + 1 ] << 8 );
            o += 2 + xlen;
            }
        if ( flags & 8 )
            {
            while ( o < aLen && aIn[ o ] != 0 )
                {
                o++;
                }
            o++;
            }
        if ( flags & 16 )
            {
            while ( o < aLen && aIn[ o ] != 0 )
                {
                o++;
                }
            o++;
            }
        if ( flags & 2 )
            {
            o += 2;
            }
        if ( o >= aLen )
            {
            return KErrCorrupt;
            }
        aOff = o;
        return KErrNone;
        }
    }

TInt InflateTlGzip( const TUint8* aIn, TInt aInLen, TUint8* aOut, TInt aOutMax,
                    TInt& aOutLen )
    {
    aOutLen = 0;
    if ( !aIn || aInLen < 2 || !aOut || aOutMax < 1 )
        {
        return KErrCorrupt;
        }

    TInf s;
    s.iIn = aIn;
    s.iInLen = aInLen;
    s.iInPos = 0;
    s.iAcc = 0;
    s.iBits = 0;
    s.iOut = aOut;
    s.iOutMax = aOutMax;
    s.iOutLen = 0;

    if ( aInLen >= 10 && aIn[ 0 ] == 0x1f && aIn[ 1 ] == 0x8b )
        {
        TInt off = 0;
        if ( SkipGzipHeader( aIn, aInLen, off ) != KErrNone )
            {
            return KErrCorrupt;
            }
        s.iInPos = off;
        // gzip trailer is 8 bytes; leave it unread after the last block.
        if ( aInLen >= off + 8 )
            {
            s.iInLen = aInLen - 8;
            }
        }
    else if ( aInLen >= 2 && ( aIn[ 0 ] & 0x0f ) == 8 &&
              ( ( (TUint16)aIn[ 0 ] << 8 ) | aIn[ 1 ] ) % 31 == 0 )
        {
        s.iInPos = 2;
        }

    const TInt err = InflateRaw( s );
    aOutLen = s.iOutLen;
    return err;
    }
