#include "SymgramCrypto.h"

const TUint8 KTelegramRsaN[ 256 ] =
    {
    0xE8, 0xBB, 0x33, 0x05, 0xC0, 0xB5, 0x2C, 0x6C, 0xF2, 0xAF, 0xDF, 0x76, 0x37, 0x31, 0x34, 0x89,
    0xE6, 0x3E, 0x05, 0x26, 0x8E, 0x5B, 0xAD, 0xB6, 0x01, 0xAF, 0x41, 0x77, 0x86, 0x47, 0x2E, 0x5F,
    0x93, 0xB8, 0x54, 0x38, 0x96, 0x8E, 0x20, 0xE6, 0x72, 0x9A, 0x30, 0x1C, 0x0A, 0xFC, 0x12, 0x1B,
    0xF7, 0x15, 0x1F, 0x83, 0x44, 0x36, 0xF7, 0xFD, 0xA6, 0x80, 0x84, 0x7A, 0x66, 0xBF, 0x64, 0xAC,
    0xCE, 0xC7, 0x8E, 0xE2, 0x1C, 0x0B, 0x31, 0x6F, 0x0E, 0xDA, 0xFE, 0x2F, 0x41, 0x90, 0x8D, 0xA7,
    0xBD, 0x1F, 0x4A, 0x51, 0x07, 0x63, 0x8E, 0xEB, 0x67, 0x04, 0x0A, 0xCE, 0x47, 0x2A, 0x14, 0xF9,
    0x0D, 0x9F, 0x7C, 0x2B, 0x7D, 0xEF, 0x99, 0x68, 0x8B, 0xA3, 0x07, 0x3A, 0xDB, 0x57, 0x50, 0xBB,
    0x02, 0x96, 0x49, 0x02, 0xA3, 0x59, 0xFE, 0x74, 0x5D, 0x81, 0x70, 0xE3, 0x68, 0x76, 0xD4, 0xFD,
    0x8A, 0x5D, 0x41, 0xB2, 0xA7, 0x6C, 0xBF, 0xF9, 0xA1, 0x32, 0x67, 0xEB, 0x95, 0x80, 0xB2, 0xD0,
    0x6D, 0x10, 0x35, 0x74, 0x48, 0xD2, 0x0D, 0x9D, 0xA2, 0x19, 0x1C, 0xB5, 0xD8, 0xC9, 0x39, 0x82,
    0x96, 0x1C, 0xDF, 0xDE, 0xDA, 0x62, 0x9E, 0x37, 0xF1, 0xFB, 0x09, 0xA0, 0x72, 0x20, 0x27, 0x69,
    0x60, 0x32, 0xFE, 0x61, 0xED, 0x66, 0x3D, 0xB7, 0xA3, 0x7F, 0x6F, 0x26, 0x3D, 0x37, 0x0F, 0x69,
    0xDB, 0x53, 0xA0, 0xDC, 0x0A, 0x17, 0x48, 0xBD, 0xAA, 0xFF, 0x62, 0x09, 0xD5, 0x64, 0x54, 0x85,
    0xE6, 0xE0, 0x01, 0xD1, 0x95, 0x32, 0x55, 0x75, 0x7E, 0x4B, 0x8E, 0x42, 0x81, 0x33, 0x47, 0xB1,
    0x1D, 0xA6, 0xAB, 0x50, 0x0F, 0xD0, 0xAC, 0xE7, 0xE6, 0xDF, 0xA3, 0x73, 0x61, 0x99, 0xCC, 0xAF,
    0x93, 0x97, 0xED, 0x07, 0x45, 0xA4, 0x27, 0xDC, 0xFA, 0x6C, 0xD6, 0x7B, 0xCB, 0x1A, 0xCF, 0xF3
    };

namespace
    {
    inline TUint32 Rotr( TUint32 aX, TInt aN )
        {
        return ( aX >> aN ) | ( aX << ( 32 - aN ) );
        }

    inline TUint32 GetBe32( const TUint8* aP )
        {
        return ( (TUint32)aP[ 0 ] << 24 ) | ( (TUint32)aP[ 1 ] << 16 )
             | ( (TUint32)aP[ 2 ] << 8 ) | (TUint32)aP[ 3 ];
        }

    inline void PutBe32( TUint8* aP, TUint32 aV )
        {
        aP[ 0 ] = (TUint8)( aV >> 24 );
        aP[ 1 ] = (TUint8)( aV >> 16 );
        aP[ 2 ] = (TUint8)( aV >> 8 );
        aP[ 3 ] = (TUint8)aV;
        }

    inline TUint64 GetBe64( const TUint8* aP )
        {
        return ( (TUint64)GetBe32( aP ) << 32 ) | GetBe32( aP + 4 );
        }

    inline void PutBe64( TUint8* aP, TUint64 aV )
        {
        PutBe32( aP, (TUint32)( aV >> 32 ) );
        PutBe32( aP + 4, (TUint32)aV );
        }

    inline TUint64 Rotr64( TUint64 aX, TInt aN )
        {
        return ( aX >> aN ) | ( aX << ( 64 - aN ) );
        }

    const TUint32 KSha256K[ 64 ] =
        {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
        };

    const TUint8 KAesSbox[ 256 ] =
        {
        0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
        0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
        0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
        0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
        0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
        0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
        0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
        0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
        0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
        0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
        0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
        0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
        0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
        0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
        0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
        0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
        };

    const TUint8 KAesInvSbox[ 256 ] =
        {
        0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
        0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
        0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
        0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
        0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
        0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
        0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
        0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
        0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
        0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
        0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
        0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
        0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
        0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
        0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
        0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
        };

    const TUint8 KAesRcon[ 10 ] =
        { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36 };

    inline TUint8 Xt( TUint8 aX )
        {
        return (TUint8)( ( aX << 1 ) ^ ( ( aX & 0x80 ) ? 0x1b : 0 ) );
        }

    void AesKeyExpand( const TUint8 aKey[ 32 ], TUint32 aRk[ 60 ] )
        {
        for ( TInt i = 0; i < 8; i++ )
            {
            aRk[ i ] = GetBe32( aKey + 4 * i );
            }
        for ( TInt i = 8; i < 60; i++ )
            {
            TUint32 t = aRk[ i - 1 ];
            if ( ( i % 8 ) == 0 )
                {
                t = ( KAesSbox[ ( t >> 16 ) & 0xff ] << 24 )
                  | ( KAesSbox[ ( t >> 8 ) & 0xff ] << 16 )
                  | ( KAesSbox[ t & 0xff ] << 8 )
                  | KAesSbox[ t >> 24 ];
                t ^= ( (TUint32)KAesRcon[ i / 8 - 1 ] ) << 24;
                }
            else if ( ( i % 8 ) == 4 )
                {
                t = ( KAesSbox[ t >> 24 ] << 24 )
                  | ( KAesSbox[ ( t >> 16 ) & 0xff ] << 16 )
                  | ( KAesSbox[ ( t >> 8 ) & 0xff ] << 8 )
                  | KAesSbox[ t & 0xff ];
                }
            aRk[ i ] = aRk[ i - 8 ] ^ t;
            }
        }

    void AesEncryptBlock( const TUint32 aRk[ 60 ], const TUint8 aIn[ 16 ], TUint8 aOut[ 16 ] )
        {
        TUint32 s0 = GetBe32( aIn ) ^ aRk[ 0 ];
        TUint32 s1 = GetBe32( aIn + 4 ) ^ aRk[ 1 ];
        TUint32 s2 = GetBe32( aIn + 8 ) ^ aRk[ 2 ];
        TUint32 s3 = GetBe32( aIn + 12 ) ^ aRk[ 3 ];
        TUint32 t0, t1, t2, t3;
        for ( TInt r = 1; r < 14; r++ )
            {
            t0 = ( (TUint32)KAesSbox[ s0 >> 24 ] << 24 )
               | ( (TUint32)KAesSbox[ ( s1 >> 16 ) & 0xff ] << 16 )
               | ( (TUint32)KAesSbox[ ( s2 >> 8 ) & 0xff ] << 8 )
               | (TUint32)KAesSbox[ s3 & 0xff ];
            t1 = ( (TUint32)KAesSbox[ s1 >> 24 ] << 24 )
               | ( (TUint32)KAesSbox[ ( s2 >> 16 ) & 0xff ] << 16 )
               | ( (TUint32)KAesSbox[ ( s3 >> 8 ) & 0xff ] << 8 )
               | (TUint32)KAesSbox[ s0 & 0xff ];
            t2 = ( (TUint32)KAesSbox[ s2 >> 24 ] << 24 )
               | ( (TUint32)KAesSbox[ ( s3 >> 16 ) & 0xff ] << 16 )
               | ( (TUint32)KAesSbox[ ( s0 >> 8 ) & 0xff ] << 8 )
               | (TUint32)KAesSbox[ s1 & 0xff ];
            t3 = ( (TUint32)KAesSbox[ s3 >> 24 ] << 24 )
               | ( (TUint32)KAesSbox[ ( s0 >> 16 ) & 0xff ] << 16 )
               | ( (TUint32)KAesSbox[ ( s1 >> 8 ) & 0xff ] << 8 )
               | (TUint32)KAesSbox[ s2 & 0xff ];
            // MixColumns
            TUint32* ts[ 4 ] = { &t0, &t1, &t2, &t3 };
            for ( TInt col = 0; col < 4; col++ )
                {
                TUint32 v = *ts[ col ];
                TUint8 x0 = (TUint8)( v >> 24 ), x1 = (TUint8)( v >> 16 );
                TUint8 x2 = (TUint8)( v >> 8 ), x3 = (TUint8)v;
                TUint8 y0 = (TUint8)( Xt( x0 ) ^ Xt( x1 ) ^ x1 ^ x2 ^ x3 );
                TUint8 y1 = (TUint8)( x0 ^ Xt( x1 ) ^ Xt( x2 ) ^ x2 ^ x3 );
                TUint8 y2 = (TUint8)( x0 ^ x1 ^ Xt( x2 ) ^ Xt( x3 ) ^ x3 );
                TUint8 y3 = (TUint8)( Xt( x0 ) ^ x0 ^ x1 ^ x2 ^ Xt( x3 ) );
                *ts[ col ] = ( (TUint32)y0 << 24 ) | ( (TUint32)y1 << 16 )
                           | ( (TUint32)y2 << 8 ) | y3;
                }
            t0 ^= aRk[ 4 * r ];
            t1 ^= aRk[ 4 * r + 1 ];
            t2 ^= aRk[ 4 * r + 2 ];
            t3 ^= aRk[ 4 * r + 3 ];
            s0 = t0; s1 = t1; s2 = t2; s3 = t3;
            }
        t0 = ( (TUint32)KAesSbox[ s0 >> 24 ] << 24 )
           | ( (TUint32)KAesSbox[ ( s1 >> 16 ) & 0xff ] << 16 )
           | ( (TUint32)KAesSbox[ ( s2 >> 8 ) & 0xff ] << 8 )
           | (TUint32)KAesSbox[ s3 & 0xff ];
        t1 = ( (TUint32)KAesSbox[ s1 >> 24 ] << 24 )
           | ( (TUint32)KAesSbox[ ( s2 >> 16 ) & 0xff ] << 16 )
           | ( (TUint32)KAesSbox[ ( s3 >> 8 ) & 0xff ] << 8 )
           | (TUint32)KAesSbox[ s0 & 0xff ];
        t2 = ( (TUint32)KAesSbox[ s2 >> 24 ] << 24 )
           | ( (TUint32)KAesSbox[ ( s3 >> 16 ) & 0xff ] << 16 )
           | ( (TUint32)KAesSbox[ ( s0 >> 8 ) & 0xff ] << 8 )
           | (TUint32)KAesSbox[ s1 & 0xff ];
        t3 = ( (TUint32)KAesSbox[ s3 >> 24 ] << 24 )
           | ( (TUint32)KAesSbox[ ( s0 >> 16 ) & 0xff ] << 16 )
           | ( (TUint32)KAesSbox[ ( s1 >> 8 ) & 0xff ] << 8 )
           | (TUint32)KAesSbox[ s2 & 0xff ];
        t0 ^= aRk[ 56 ]; t1 ^= aRk[ 57 ]; t2 ^= aRk[ 58 ]; t3 ^= aRk[ 59 ];
        PutBe32( aOut, t0 );
        PutBe32( aOut + 4, t1 );
        PutBe32( aOut + 8, t2 );
        PutBe32( aOut + 12, t3 );
        }

    // InvMixColumns helper
    inline TUint8 XtimeN( TUint8 aX, TInt aN )
        {
        TUint8 r = 0, t = aX;
        while ( aN )
            {
            if ( aN & 1 )
                {
                r ^= t;
                }
            t = Xt( t );
            aN >>= 1;
            }
        return r;
        }

    void InvShiftSub( TUint32& a0, TUint32& a1, TUint32& a2, TUint32& a3 )
        {
        const TUint32 t0 = ( (TUint32)KAesInvSbox[ a0 >> 24 ] << 24 )
                         | ( (TUint32)KAesInvSbox[ ( a3 >> 16 ) & 0xff ] << 16 )
                         | ( (TUint32)KAesInvSbox[ ( a2 >> 8 ) & 0xff ] << 8 )
                         | (TUint32)KAesInvSbox[ a1 & 0xff ];
        const TUint32 t1 = ( (TUint32)KAesInvSbox[ a1 >> 24 ] << 24 )
                         | ( (TUint32)KAesInvSbox[ ( a0 >> 16 ) & 0xff ] << 16 )
                         | ( (TUint32)KAesInvSbox[ ( a3 >> 8 ) & 0xff ] << 8 )
                         | (TUint32)KAesInvSbox[ a2 & 0xff ];
        const TUint32 t2 = ( (TUint32)KAesInvSbox[ a2 >> 24 ] << 24 )
                         | ( (TUint32)KAesInvSbox[ ( a1 >> 16 ) & 0xff ] << 16 )
                         | ( (TUint32)KAesInvSbox[ ( a0 >> 8 ) & 0xff ] << 8 )
                         | (TUint32)KAesInvSbox[ a3 & 0xff ];
        const TUint32 t3 = ( (TUint32)KAesInvSbox[ a3 >> 24 ] << 24 )
                         | ( (TUint32)KAesInvSbox[ ( a2 >> 16 ) & 0xff ] << 16 )
                         | ( (TUint32)KAesInvSbox[ ( a1 >> 8 ) & 0xff ] << 8 )
                         | (TUint32)KAesInvSbox[ a0 & 0xff ];
        a0 = t0; a1 = t1; a2 = t2; a3 = t3;
        }

    void AesDecryptBlock( const TUint32 aRk[ 60 ], const TUint8 aIn[ 16 ], TUint8 aOut[ 16 ] )
        {
        TUint32 s0 = GetBe32( aIn ) ^ aRk[ 56 ];
        TUint32 s1 = GetBe32( aIn + 4 ) ^ aRk[ 57 ];
        TUint32 s2 = GetBe32( aIn + 8 ) ^ aRk[ 58 ];
        TUint32 s3 = GetBe32( aIn + 12 ) ^ aRk[ 59 ];
        InvShiftSub( s0, s1, s2, s3 );
        for ( TInt r = 13; r >= 1; r-- )
            {
            s0 ^= aRk[ 4 * r ];
            s1 ^= aRk[ 4 * r + 1 ];
            s2 ^= aRk[ 4 * r + 2 ];
            s3 ^= aRk[ 4 * r + 3 ];
            TUint32* ts[ 4 ] = { &s0, &s1, &s2, &s3 };
            for ( TInt col = 0; col < 4; col++ )
                {
                TUint32 v = *ts[ col ];
                TUint8 x0 = (TUint8)( v >> 24 ), x1 = (TUint8)( v >> 16 );
                TUint8 x2 = (TUint8)( v >> 8 ), x3 = (TUint8)v;
                TUint8 y0 = (TUint8)( XtimeN( x0, 0x0e ) ^ XtimeN( x1, 0x0b ) ^ XtimeN( x2, 0x0d ) ^ XtimeN( x3, 0x09 ) );
                TUint8 y1 = (TUint8)( XtimeN( x0, 0x09 ) ^ XtimeN( x1, 0x0e ) ^ XtimeN( x2, 0x0b ) ^ XtimeN( x3, 0x0d ) );
                TUint8 y2 = (TUint8)( XtimeN( x0, 0x0d ) ^ XtimeN( x1, 0x09 ) ^ XtimeN( x2, 0x0e ) ^ XtimeN( x3, 0x0b ) );
                TUint8 y3 = (TUint8)( XtimeN( x0, 0x0b ) ^ XtimeN( x1, 0x0d ) ^ XtimeN( x2, 0x09 ) ^ XtimeN( x3, 0x0e ) );
                *ts[ col ] = ( (TUint32)y0 << 24 ) | ( (TUint32)y1 << 16 )
                           | ( (TUint32)y2 << 8 ) | y3;
                }
            InvShiftSub( s0, s1, s2, s3 );
            }
        s0 ^= aRk[ 0 ];
        s1 ^= aRk[ 1 ];
        s2 ^= aRk[ 2 ];
        s3 ^= aRk[ 3 ];
        PutBe32( aOut, s0 );
        PutBe32( aOut + 4, s1 );
        PutBe32( aOut + 8, s2 );
        PutBe32( aOut + 12, s3 );
        }
    }

void Sha256( const TUint8* aData, TInt aLen, TUint8 aOut[ 32 ] )
    {
    TUint32 h[ 8 ] =
        {
        0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
        };

    TInt zeros = ( 56 - ( ( aLen + 1 ) % 64 ) + 64 ) % 64;
    const TInt total = aLen + 1 + zeros + 8;

    TUint32 w[ 64 ];
    for ( TInt off = 0; off < total; off += 64 )
        {
        TUint8 block[ 64 ];
        for ( TInt i = 0; i < 64; i++ )
            {
            const TInt idx = off + i;
            if ( idx < aLen )
                {
                block[ i ] = aData[ idx ];
                }
            else if ( idx == aLen )
                {
                block[ i ] = 0x80;
                }
            else if ( idx < total - 8 )
                {
                block[ i ] = 0;
                }
            else
                {
                const TUint64 bits = (TUint64)aLen << 3;
                block[ i ] = (TUint8)( bits >> ( 8 * ( total - 1 - idx ) ) );
                }
            }

        for ( TInt i = 0; i < 16; i++ )
            {
            w[ i ] = GetBe32( block + 4 * i );
            }
        for ( TInt i = 16; i < 64; i++ )
            {
            const TUint32 s0 = Rotr( w[ i - 15 ], 7 ) ^ Rotr( w[ i - 15 ], 18 ) ^ ( w[ i - 15 ] >> 3 );
            const TUint32 s1 = Rotr( w[ i - 2 ], 17 ) ^ Rotr( w[ i - 2 ], 19 ) ^ ( w[ i - 2 ] >> 10 );
            w[ i ] = w[ i - 16 ] + s0 + w[ i - 7 ] + s1;
            }

        TUint32 a = h[ 0 ], b = h[ 1 ], c = h[ 2 ], d = h[ 3 ];
        TUint32 e = h[ 4 ], f = h[ 5 ], g = h[ 6 ], hh = h[ 7 ];
        for ( TInt i = 0; i < 64; i++ )
            {
            const TUint32 S1 = Rotr( e, 6 ) ^ Rotr( e, 11 ) ^ Rotr( e, 25 );
            const TUint32 ch = ( e & f ) ^ ( ( ~e ) & g );
            const TUint32 t1 = hh + S1 + ch + KSha256K[ i ] + w[ i ];
            const TUint32 S0 = Rotr( a, 2 ) ^ Rotr( a, 13 ) ^ Rotr( a, 22 );
            const TUint32 maj = ( a & b ) ^ ( a & c ) ^ ( b & c );
            const TUint32 t2 = S0 + maj;
            hh = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
            }
        h[ 0 ] += a; h[ 1 ] += b; h[ 2 ] += c; h[ 3 ] += d;
        h[ 4 ] += e; h[ 5 ] += f; h[ 6 ] += g; h[ 7 ] += hh;
        }

    for ( TInt i = 0; i < 8; i++ )
        {
        PutBe32( aOut + 4 * i, h[ i ] );
        }
    }

void Sha1( const TUint8* aData, TInt aLen, TUint8 aOut[ 20 ] )
    {
    TUint32 h0 = 0x67452301u;
    TUint32 h1 = 0xefcdab89u;
    TUint32 h2 = 0x98badcfeu;
    TUint32 h3 = 0x10325476u;
    TUint32 h4 = 0xc3d2e1f0u;

    TInt zeros = ( 56 - ( ( aLen + 1 ) % 64 ) + 64 ) % 64;
    const TInt total = aLen + 1 + zeros + 8;

    for ( TInt off = 0; off < total; off += 64 )
        {
        TUint8 block[ 64 ];
        for ( TInt i = 0; i < 64; i++ )
            {
            const TInt idx = off + i;
            if ( idx < aLen )
                {
                block[ i ] = aData[ idx ];
                }
            else if ( idx == aLen )
                {
                block[ i ] = 0x80;
                }
            else if ( idx < total - 8 )
                {
                block[ i ] = 0;
                }
            else
                {
                const TUint64 bits = (TUint64)aLen << 3;
                block[ i ] = (TUint8)( bits >> ( 8 * ( total - 1 - idx ) ) );
                }
            }

        TUint32 w[ 80 ];
        for ( TInt i = 0; i < 16; i++ )
            {
            w[ i ] = GetBe32( block + 4 * i );
            }
        for ( TInt i = 16; i < 80; i++ )
            {
            const TUint32 x = w[ i - 3 ] ^ w[ i - 8 ] ^ w[ i - 14 ] ^ w[ i - 16 ];
            w[ i ] = ( x << 1 ) | ( x >> 31 );
            }

        TUint32 a = h0, b = h1, c = h2, d = h3, e = h4;
        for ( TInt i = 0; i < 80; i++ )
            {
            TUint32 f, k;
            if ( i < 20 )
                {
                f = ( b & c ) | ( ( ~b ) & d );
                k = 0x5a827999u;
                }
            else if ( i < 40 )
                {
                f = b ^ c ^ d;
                k = 0x6ed9eba1u;
                }
            else if ( i < 60 )
                {
                f = ( b & c ) | ( b & d ) | ( c & d );
                k = 0x8f1bbcdcu;
                }
            else
                {
                f = b ^ c ^ d;
                k = 0xca62c1d6u;
                }
            const TUint32 t = ( ( a << 5 ) | ( a >> 27 ) ) + f + e + k + w[ i ];
            e = d;
            d = c;
            c = ( b << 30 ) | ( b >> 2 );
            b = a;
            a = t;
            }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
        }

    PutBe32( aOut, h0 );
    PutBe32( aOut + 4, h1 );
    PutBe32( aOut + 8, h2 );
    PutBe32( aOut + 12, h3 );
    PutBe32( aOut + 16, h4 );
    }

void Sha512( const TUint8* aData, TInt aLen, TUint8 aOut[ 64 ] )
    {
    static const TUint64 K[ 80 ] =
        {
        0x428a2f98d728ae22ull,0x7137449123ef65cdull,0xb5c0fbcfec4d3b2full,0xe9b5dba58189dbbcull,
        0x3956c25bf348b538ull,0x59f111f1b605d019ull,0x923f82a4af194f9bull,0xab1c5ed5da6d8118ull,
        0xd807aa98a3030242ull,0x12835b0145706fbeull,0x243185be4ee4b28cull,0x550c7dc3d5ffb4e2ull,
        0x72be5d74f27b896full,0x80deb1fe3b1696b1ull,0x9bdc06a725c71235ull,0xc19bf174cf692694ull,
        0xe49b69c19ef14ad2ull,0xefbe4786384f25e3ull,0x0fc19dc68b8cd5b5ull,0x240ca1cc77ac9c65ull,
        0x2de92c6f592b0275ull,0x4a7484aa6ea6e483ull,0x5cb0a9dcbd41fbd4ull,0x76f988da831153b5ull,
        0x983e5152ee66dfabull,0xa831c66d2db43210ull,0xb00327c898fb213full,0xbf597fc7beef0ee4ull,
        0xc6e00bf33da88fc2ull,0xd5a79147930aa725ull,0x06ca6351e003826full,0x142929670a0e6e70ull,
        0x27b70a8546d22ffcull,0x2e1b21385c26c926ull,0x4d2c6dfc5ac42aedull,0x53380d139d95b3dfull,
        0x650a73548baf63deull,0x766a0abb3c77b2a8ull,0x81c2c92e47edaee6ull,0x92722c851482353bull,
        0xa2bfe8a14cf10364ull,0xa81a664bbc423001ull,0xc24b8b70d0f89791ull,0xc76c51a30654be30ull,
        0xd192e819d6ef5218ull,0xd69906245565a910ull,0xf40e35855771202aull,0x106aa07032bbd1b8ull,
        0x19a4c116b8d2d0c8ull,0x1e376c085141ab53ull,0x2748774cdf8eeb99ull,0x34b0bcb5e19b48a8ull,
        0x391c0cb3c5c95a63ull,0x4ed8aa4ae3418acbull,0x5b9cca4f7763e373ull,0x682e6ff3d6b2b8a3ull,
        0x748f82ee5defb2fcull,0x78a5636f43172f60ull,0x84c87814a1f0ab72ull,0x8cc702081a6439ecull,
        0x90befffa23631e28ull,0xa4506cebde82bde9ull,0xbef9a3f7b2c67915ull,0xc67178f2e372532bull,
        0xca273eceea26619cull,0xd186b8c721c0c207ull,0xeada7dd6cde0eb1eull,0xf57d4f7fee6ed178ull,
        0x06f067aa72176fbaull,0x0a637dc5a2c898a6ull,0x113f9804bef90daeull,0x1b710b35131c471bull,
        0x28db77f523047d84ull,0x32caab7b40c72493ull,0x3c9ebe0a15c9bebcull,0x431d67c49c100d4cull,
        0x4cc5d4becb3e42b6ull,0x597f299cfc657e2aull,0x5fcb6fab3ad6faecull,0x6c44198c4a475817ull
        };

    TUint64 h[ 8 ];
    h[ 0 ] = 0x6a09e667f3bcc908ull;
    h[ 1 ] = 0xbb67ae8584caa73bull;
    h[ 2 ] = 0x3c6ef372fe94f82bull;
    h[ 3 ] = 0xa54ff53a5f1d36f1ull;
    h[ 4 ] = 0x510e527fade682d1ull;
    h[ 5 ] = 0x9b05688c2b3e6c1full;
    h[ 6 ] = 0x1f83d9abfb41bd6bull;
    h[ 7 ] = 0x5be0cd19137e2179ull;

    TInt zeros = ( 112 - ( ( aLen + 1 ) % 128 ) + 128 ) % 128;
    const TInt total = aLen + 1 + zeros + 16;

    TUint64 w[ 80 ];
    for ( TInt off = 0; off < total; off += 128 )
        {
        TUint8 block[ 128 ];
        for ( TInt i = 0; i < 128; i++ )
            {
            const TInt idx = off + i;
            if ( idx < aLen )
                {
                block[ i ] = aData[ idx ];
                }
            else if ( idx == aLen )
                {
                block[ i ] = 0x80;
                }
            else if ( idx < total - 16 )
                {
                block[ i ] = 0;
                }
            else
                {
                const TUint64 bits = (TUint64)aLen << 3;
                block[ i ] = (TUint8)( bits >> ( 8 * ( total - 1 - idx ) ) );
                }
            }
        for ( TInt i = 0; i < 16; i++ )
            {
            w[ i ] = GetBe64( block + 8 * i );
            }
        for ( TInt i = 16; i < 80; i++ )
            {
            const TUint64 s0 = Rotr64( w[ i - 15 ], 1 ) ^ Rotr64( w[ i - 15 ], 8 ) ^ ( w[ i - 15 ] >> 7 );
            const TUint64 s1 = Rotr64( w[ i - 2 ], 19 ) ^ Rotr64( w[ i - 2 ], 61 ) ^ ( w[ i - 2 ] >> 6 );
            w[ i ] = w[ i - 16 ] + s0 + w[ i - 7 ] + s1;
            }
        TUint64 a = h[ 0 ], b = h[ 1 ], c = h[ 2 ], d = h[ 3 ];
        TUint64 e = h[ 4 ], f = h[ 5 ], g = h[ 6 ], hh = h[ 7 ];
        for ( TInt i = 0; i < 80; i++ )
            {
            const TUint64 S1 = Rotr64( e, 14 ) ^ Rotr64( e, 18 ) ^ Rotr64( e, 41 );
            const TUint64 ch = ( e & f ) ^ ( ( ~e ) & g );
            const TUint64 t1 = hh + S1 + ch + K[ i ] + w[ i ];
            const TUint64 S0 = Rotr64( a, 28 ) ^ Rotr64( a, 34 ) ^ Rotr64( a, 39 );
            const TUint64 maj = ( a & b ) ^ ( a & c ) ^ ( b & c );
            const TUint64 t2 = S0 + maj;
            hh = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
            }
        h[ 0 ] += a; h[ 1 ] += b; h[ 2 ] += c; h[ 3 ] += d;
        h[ 4 ] += e; h[ 5 ] += f; h[ 6 ] += g; h[ 7 ] += hh;
        }
    for ( TInt i = 0; i < 8; i++ )
        {
        PutBe64( aOut + 8 * i, h[ i ] );
        }
    }

void HmacSha512( const TUint8* aKey, TInt aKeyLen,
                 const TUint8* aData, TInt aLen, TUint8 aOut[ 64 ] )
    {
    TUint8 key[ 128 ];
    Mem::FillZ( key, 128 );
    if ( aKeyLen > 128 )
        {
        Sha512( aKey, aKeyLen, key );
        }
    else if ( aKeyLen > 0 )
        {
        Mem::Copy( key, aKey, aKeyLen );
        }
    TUint8 ipad[ 128 ];
    TUint8 opad[ 128 ];
    TInt i = 0;
    for ( i = 0; i < 128; i++ )
        {
        ipad[ i ] = (TUint8)( key[ i ] ^ 0x36 );
        opad[ i ] = (TUint8)( key[ i ] ^ 0x5c );
        }
    TUint8 inner[ 512 ];
    Mem::Copy( inner, ipad, 128 );
    if ( aLen > 384 )
        {
        aLen = 384;
        }
    if ( aLen > 0 )
        {
        Mem::Copy( inner + 128, aData, aLen );
        }
    TUint8 innerHash[ 64 ];
    Sha512( inner, 128 + aLen, innerHash );
    TUint8 outer[ 192 ];
    Mem::Copy( outer, opad, 128 );
    Mem::Copy( outer + 128, innerHash, 64 );
    Sha512( outer, 192, aOut );
    }

void Pbkdf2HmacSha512Begin( const TUint8* aPass, TInt aPassLen,
                            const TUint8* aSalt, TInt aSaltLen,
                            TUint8 aU[ 64 ], TUint8 aT[ 64 ] )
    {
    if ( aSaltLen > 256 )
        {
        aSaltLen = 256;
        }
    TUint8 block[ 260 ];
    Mem::Copy( block, aSalt, aSaltLen );
    block[ aSaltLen ] = 0;
    block[ aSaltLen + 1 ] = 0;
    block[ aSaltLen + 2 ] = 0;
    block[ aSaltLen + 3 ] = 1;
    HmacSha512( aPass, aPassLen, block, aSaltLen + 4, aU );
    Mem::Copy( aT, aU, 64 );
    }

void Pbkdf2HmacSha512Rounds( const TUint8* aPass, TInt aPassLen,
                             TUint8 aU[ 64 ], TUint8 aT[ 64 ], TInt aCount )
    {
    TInt n = 0;
    for ( n = 0; n < aCount; n++ )
        {
        HmacSha512( aPass, aPassLen, aU, 64, aU );
        TInt b = 0;
        for ( b = 0; b < 64; b++ )
            {
            aT[ b ] ^= aU[ b ];
            }
        }
    }

void Pbkdf2HmacSha512( const TUint8* aPass, TInt aPassLen,
                       const TUint8* aSalt, TInt aSaltLen,
                       TInt aIter, TUint8 aOut[ 64 ] )
    {
    TUint8 u[ 64 ];
    TUint8 t[ 64 ];
    Pbkdf2HmacSha512Begin( aPass, aPassLen, aSalt, aSaltLen, u, t );
    if ( aIter > 1 )
        {
        Pbkdf2HmacSha512Rounds( aPass, aPassLen, u, t, aIter - 1 );
        }
    Mem::Copy( aOut, t, 64 );
    }

void Aes256IgeEncrypt( const TUint8* aIn, TUint8* aOut, TInt aLen,
                       const TUint8 aKey[ 32 ], const TUint8 aIv[ 32 ] )
    {
    TUint32 rk[ 60 ];
    AesKeyExpand( aKey, rk );
    TUint8 prevC[ 16 ], prevP[ 16 ];
    Mem::Copy( prevC, aIv, 16 );
    Mem::Copy( prevP, aIv + 16, 16 );
    TUint8 block[ 16 ], xored[ 16 ];
    for ( TInt off = 0; off < aLen; off += 16 )
        {
        for ( TInt i = 0; i < 16; i++ )
            {
            xored[ i ] = (TUint8)( aIn[ off + i ] ^ prevC[ i ] );
            }
        AesEncryptBlock( rk, xored, block );
        for ( TInt i = 0; i < 16; i++ )
            {
            aOut[ off + i ] = (TUint8)( block[ i ] ^ prevP[ i ] );
            }
        Mem::Copy( prevP, aIn + off, 16 );
        Mem::Copy( prevC, aOut + off, 16 );
        }
    }

void Aes256IgeDecrypt( const TUint8* aIn, TUint8* aOut, TInt aLen,
                       const TUint8 aKey[ 32 ], const TUint8 aIv[ 32 ] )
    {
    TUint32 rk[ 60 ];
    AesKeyExpand( aKey, rk );
    TUint8 prevC[ 16 ], prevP[ 16 ];
    Mem::Copy( prevC, aIv, 16 );
    Mem::Copy( prevP, aIv + 16, 16 );
    TUint8 block[ 16 ], xored[ 16 ];
    for ( TInt off = 0; off < aLen; off += 16 )
        {
        for ( TInt i = 0; i < 16; i++ )
            {
            xored[ i ] = (TUint8)( aIn[ off + i ] ^ prevP[ i ] );
            }
        AesDecryptBlock( rk, xored, block );
        for ( TInt i = 0; i < 16; i++ )
            {
            aOut[ off + i ] = (TUint8)( block[ i ] ^ prevC[ i ] );
            }
        Mem::Copy( prevC, aIn + off, 16 );
        Mem::Copy( prevP, aOut + off, 16 );
        }
    }

void TBn::Zero()
    {
    for ( TInt i = 0; i < KBnLimbs; i++ )
        {
        iD[ i ] = 0;
        }
    }

void TBn::FromBe( const TUint8* aBytes, TInt aLen )
    {
    Zero();
    TInt o = aLen - 1;
    TInt limb = 0;
    TInt shift = 0;
    while ( o >= 0 && limb < KBnLimbs )
        {
        iD[ limb ] |= ( (TUint32)aBytes[ o ] ) << shift;
        shift += 8;
        if ( shift == 32 )
            {
            shift = 0;
            limb++;
            }
        o--;
        }
    }

void TBn::ToBe( TUint8* aBytes, TInt aLen ) const
    {
    Mem::FillZ( aBytes, aLen );
    TInt o = aLen - 1;
    TInt limb = 0;
    TInt shift = 0;
    while ( o >= 0 && limb < KBnLimbs )
        {
        aBytes[ o ] = (TUint8)( iD[ limb ] >> shift );
        shift += 8;
        if ( shift == 32 )
            {
            shift = 0;
            limb++;
            }
        o--;
        }
    }

TInt TBn::Cmp( const TBn& aOther ) const
    {
    for ( TInt i = KBnLimbs - 1; i >= 0; i-- )
        {
        if ( iD[ i ] < aOther.iD[ i ] )
            {
            return -1;
            }
        if ( iD[ i ] > aOther.iD[ i ] )
            {
            return 1;
            }
        }
    return 0;
    }

TBool TBn::IsZero() const
    {
    for ( TInt i = 0; i < KBnLimbs; i++ )
        {
        if ( iD[ i ] )
            {
            return EFalse;
            }
        }
    return ETrue;
    }

namespace
    {
    TUint32 MontInv32( TUint32 aN0 )
        {
        TUint32 x = aN0;
        x *= 2 - aN0 * x;
        x *= 2 - aN0 * x;
        x *= 2 - aN0 * x;
        x *= 2 - aN0 * x;
        x *= 2 - aN0 * x;
        return (TUint32)( 0 - x );
        }

    void MontMul( TBn& aOut, const TBn& aA, const TBn& aB, const TBn& aN, TUint32 aN0inv )
        {
        TUint32 t[ KBnLimbs + 2 ];
        for ( TInt i = 0; i < KBnLimbs + 2; i++ )
            {
            t[ i ] = 0;
            }
        for ( TInt i = 0; i < KBnLimbs; i++ )
            {
            TUint64 c = 0;
            const TUint32 ai = aA.iD[ i ];
            for ( TInt j = 0; j < KBnLimbs; j++ )
                {
                c += (TUint64)t[ j ] + (TUint64)ai * aB.iD[ j ];
                t[ j ] = (TUint32)c;
                c >>= 32;
                }
            c += t[ KBnLimbs ];
            t[ KBnLimbs ] = (TUint32)c;
            t[ KBnLimbs + 1 ] = (TUint32)( c >> 32 );

            const TUint32 m = t[ 0 ] * aN0inv;
            c = (TUint64)t[ 0 ] + (TUint64)m * aN.iD[ 0 ];
            c >>= 32;
            for ( TInt j = 1; j < KBnLimbs; j++ )
                {
                c += (TUint64)t[ j ] + (TUint64)m * aN.iD[ j ];
                t[ j - 1 ] = (TUint32)c;
                c >>= 32;
                }
            c += t[ KBnLimbs ];
            t[ KBnLimbs - 1 ] = (TUint32)c;
            t[ KBnLimbs ] = t[ KBnLimbs + 1 ] + (TUint32)( c >> 32 );
            }
        for ( TInt i = 0; i < KBnLimbs; i++ )
            {
            aOut.iD[ i ] = t[ i ];
            }
        TBn tmp;
        tmp.Zero();
        for ( TInt i = 0; i < KBnLimbs; i++ )
            {
            tmp.iD[ i ] = t[ i ];
            }
        if ( t[ KBnLimbs ] || tmp.Cmp( aN ) >= 0 )
            {
            TUint32 br = 0;
            for ( TInt i = 0; i < KBnLimbs; i++ )
                {
                const TUint64 yy = (TUint64)aN.iD[ i ] + br;
                br = ( tmp.iD[ i ] < yy ) ? 1 : 0;
                aOut.iD[ i ] = tmp.iD[ i ] - (TUint32)yy;
                }
            }
        }

    void ToMont( TBn& aOut, const TBn& aIn, const TBn& aN, TUint32 aN0inv )
        {
        TBn r2;
        r2.Zero();
        r2.iD[ 0 ] = 1;
        for ( TInt i = 0; i < 2 * KBnLimbs * 32; i++ )
            {
            TUint64 c = (TUint64)r2.iD[ 0 ] << 1;
            r2.iD[ 0 ] = (TUint32)c;
            c >>= 32;
            for ( TInt j = 1; j < KBnLimbs; j++ )
                {
                c += (TUint64)r2.iD[ j ] << 1;
                r2.iD[ j ] = (TUint32)c;
                c >>= 32;
                }
            if ( c || r2.Cmp( aN ) >= 0 )
                {
                TUint32 br = 0;
                for ( TInt j = 0; j < KBnLimbs; j++ )
                    {
                    const TUint64 yy = (TUint64)aN.iD[ j ] + br;
                    br = ( r2.iD[ j ] < yy ) ? 1 : 0;
                    r2.iD[ j ] = r2.iD[ j ] - (TUint32)yy;
                    }
                }
            }
        MontMul( aOut, aIn, r2, aN, aN0inv );
        }
    }

void BnExpMsb( const TUint32* aExp, TInt aLimbs, TInt& aLimb, TInt& aBit )
    {
    aLimb = 0;
    aBit = 0;
    if ( !aExp || aLimbs < 1 )
        {
        return;
        }
    TInt limb = aLimbs - 1;
    while ( limb > 0 && aExp[ limb ] == 0 )
        {
        limb--;
        }
    TInt bit = 31;
    while ( bit > 0 && ( aExp[ limb ] & ( 1u << bit ) ) == 0 )
        {
        bit--;
        }
    aLimb = limb;
    aBit = bit;
    }

void BnExpBegin( TBn& aAcc, TBn& aBaseM, TUint32& aN0inv,
                 const TBn& aBase, const TBn& aMod )
    {
    aN0inv = MontInv32( aMod.iD[ 0 ] );
    ToMont( aBaseM, aBase, aMod, aN0inv );
    TBn one;
    one.Zero();
    one.iD[ 0 ] = 1;
    ToMont( aAcc, one, aMod, aN0inv );
    }

TBool BnExpStep( TBn& aAcc, const TBn& aBaseM, const TBn& aMod, TUint32 aN0inv,
                 const TUint32* aExp, TInt& aLimb, TInt& aBit, TInt aBudget )
    {
    TInt n = 0;
    TBn sq;
    TBn mul;
    while ( n < aBudget && aLimb >= 0 )
        {
        MontMul( sq, aAcc, aAcc, aMod, aN0inv );
        Mem::Copy( aAcc.iD, sq.iD, sizeof( aAcc.iD ) );
        if ( aExp[ aLimb ] & ( 1u << aBit ) )
            {
            MontMul( mul, aAcc, aBaseM, aMod, aN0inv );
            Mem::Copy( aAcc.iD, mul.iD, sizeof( aAcc.iD ) );
            }
        n++;
        aBit--;
        if ( aBit < 0 )
            {
            aBit = 31;
            aLimb--;
            }
        }
    return aLimb < 0;
    }

void BnExpFinish( TBn& aOut, const TBn& aAcc, const TBn& aMod, TUint32 aN0inv )
    {
    TBn ident;
    ident.Zero();
    ident.iD[ 0 ] = 1;
    MontMul( aOut, aAcc, ident, aMod, aN0inv );
    }

void BnModExp( TBn& aOut, const TBn& aBase, const TBn& aExp, const TBn& aMod )
    {
    BnModExpN( aOut, aBase, aExp.iD, KBnLimbs, aMod );
    }

void BnModExpN( TBn& aOut, const TBn& aBase,
                const TUint32* aExp, TInt aExpLimbs, const TBn& aMod )
    {
    const TUint32 n0inv = MontInv32( aMod.iD[ 0 ] );
    TBn baseM;
    TBn acc;
    TBn one;
    ToMont( baseM, aBase, aMod, n0inv );
    one.Zero();
    one.iD[ 0 ] = 1;
    ToMont( acc, one, aMod, n0inv );

    TInt i = 0;
    for ( i = aExpLimbs - 1; i >= 0; i-- )
        {
        TInt b = 0;
        for ( b = 31; b >= 0; b-- )
            {
            TBn sq;
            MontMul( sq, acc, acc, aMod, n0inv );
            Mem::Copy( acc.iD, sq.iD, sizeof( acc.iD ) );
            if ( aExp[ i ] & ( 1u << b ) )
                {
                TBn m;
                MontMul( m, acc, baseM, aMod, n0inv );
                Mem::Copy( acc.iD, m.iD, sizeof( acc.iD ) );
                }
            }
        }

    TBn ident;
    ident.Zero();
    ident.iD[ 0 ] = 1;
    MontMul( aOut, acc, ident, aMod, n0inv );
    }

void BnMulMod( TBn& aOut, const TBn& aA, const TBn& aB, const TBn& aMod )
    {
    const TUint32 n0inv = MontInv32( aMod.iD[ 0 ] );
    TBn am, bm, prod, ident;
    ToMont( am, aA, aMod, n0inv );
    ToMont( bm, aB, aMod, n0inv );
    MontMul( prod, am, bm, aMod, n0inv );
    ident.Zero();
    ident.iD[ 0 ] = 1;
    MontMul( aOut, prod, ident, aMod, n0inv );
    }

void BnSubMod( TBn& aOut, const TBn& aA, const TBn& aB, const TBn& aMod )
    {
    if ( aA.Cmp( aB ) >= 0 )
        {
        TUint32 br = 0;
        TInt i = 0;
        for ( i = 0; i < KBnLimbs; i++ )
            {
            const TUint64 yy = (TUint64)aB.iD[ i ] + br;
            br = ( aA.iD[ i ] < yy ) ? 1 : 0;
            aOut.iD[ i ] = aA.iD[ i ] - (TUint32)yy;
            }
        }
    else
        {
        TUint32 c = 0;
        TInt i = 0;
        TBn sum;
        for ( i = 0; i < KBnLimbs; i++ )
            {
            const TUint64 s = (TUint64)aA.iD[ i ] + aMod.iD[ i ] + c;
            sum.iD[ i ] = (TUint32)s;
            c = (TUint32)( s >> 32 );
            }
        TUint32 br = 0;
        for ( i = 0; i < KBnLimbs; i++ )
            {
            const TUint64 yy = (TUint64)aB.iD[ i ] + br;
            br = ( sum.iD[ i ] < yy ) ? 1 : 0;
            aOut.iD[ i ] = sum.iD[ i ] - (TUint32)yy;
            }
        }
    }

void BnRsaPublic( TUint8 aOut[ 256 ], const TUint8 aIn[ 256 ],
                  const TUint8 aMod[ 256 ] )
    {
    TBn in, n, e, out;
    in.FromBe( aIn, 256 );
    n.FromBe( aMod, 256 );
    e.Zero();
    e.iD[ 0 ] = 65537;
    BnModExp( out, in, e, n );
    out.ToBe( aOut, 256 );
    }

TBool FactorPq( TUint64 aPq, TUint64& aP, TUint64& aQ )
    {
    // (a*b) % m without overflowing 64-bit.
    struct F
        {
        static TUint64 MulMod( TUint64 aA, TUint64 aB, TUint64 aM )
            {
            TUint64 r = 0;
            aA %= aM;
            while ( aB )
                {
                if ( aB & 1 )
                    {
                    r += aA;
                    if ( r >= aM || r < aA )
                        {
                        r -= aM;
                        }
                    }
                const TUint64 a2 = aA << 1;
                aA = ( aA >= ( aM - aA ) ) ? ( aA - ( aM - aA ) ) : a2;
                aB >>= 1;
                }
            return r;
            }
        };

    if ( ( aPq & 1 ) == 0 )
        {
        aP = 2;
        aQ = aPq / 2;
        return ETrue;
        }
    TUint64 y = 2, c = 1, m = 128;
    TUint64 g = 1, r = 1, q = 1, x = 0, ys = 0;
    while ( g == 1 )
        {
        x = y;
        for ( TUint64 i = 0; i < r; i++ )
            {
            y = F::MulMod( y, y, aPq );
            y += c;
            if ( y >= aPq )
                {
                y -= aPq;
                }
            }
        TUint64 k = 0;
        while ( k < r && g == 1 )
            {
            ys = y;
            const TUint64 lim = ( m < r - k ) ? m : ( r - k );
            for ( TUint64 i = 0; i < lim; i++ )
                {
                y = F::MulMod( y, y, aPq );
                y += c;
                if ( y >= aPq )
                    {
                    y -= aPq;
                    }
                const TUint64 diff = x > y ? x - y : y - x;
                q = F::MulMod( q, diff, aPq );
                }
            TUint64 u = q, v = aPq;
            while ( v )
                {
                const TUint64 t = u % v;
                u = v;
                v = t;
                }
            g = u;
            k += lim;
            }
        r *= 2;
        if ( r > ( (TUint64)1 << 20 ) )
            {
            return EFalse;
            }
        }
    if ( g == aPq )
        {
        do
            {
            ys = F::MulMod( ys, ys, aPq );
            ys += c;
            if ( ys >= aPq )
                {
                ys -= aPq;
                }
            const TUint64 diff = x > ys ? x - ys : ys - x;
            TUint64 u = diff, v = aPq;
            while ( v )
                {
                const TUint64 t = u % v;
                u = v;
                v = t;
                }
            g = u;
            }
        while ( g == 1 );
        }
    if ( g <= 1 || g >= aPq )
        {
        return EFalse;
        }
    aP = g;
    aQ = aPq / g;
    if ( aP > aQ )
        {
        const TUint64 t = aP;
        aP = aQ;
        aQ = t;
        }
    return ( aP * aQ == aPq );
    }

void TlAppendBytes( TDes8& aOut, const TUint8* aData, TInt aLen )
    {
    if ( aLen < 254 )
        {
        aOut.Append( (TUint8)aLen );
        aOut.Append( aData, aLen );
        while ( aOut.Length() % 4 )
            {
            aOut.Append( 0 );
            }
        }
    else
        {
        aOut.Append( 254 );
        aOut.Append( (TUint8)aLen );
        aOut.Append( (TUint8)( aLen >> 8 ) );
        aOut.Append( (TUint8)( aLen >> 16 ) );
        aOut.Append( aData, aLen );
        while ( aOut.Length() % 4 )
            {
            aOut.Append( 0 );
            }
        }
    }

TInt TlReadBytes( const TUint8* aP, TInt aRemain, const TUint8*& aData, TInt& aLen )
    {
    if ( aRemain < 1 )
        {
        return -1;
        }
    TInt header = 1;
    if ( aP[ 0 ] == 254 )
        {
        if ( aRemain < 4 )
            {
            return -1;
            }
        aLen = aP[ 1 ] | ( aP[ 2 ] << 8 ) | ( aP[ 3 ] << 16 );
        header = 4;
        }
    else
        {
        aLen = aP[ 0 ];
        }
    const TInt padded = ( header + aLen + 3 ) & ~3;
    if ( padded > aRemain )
        {
        return -1;
        }
    aData = aP + header;
    return padded;
    }
