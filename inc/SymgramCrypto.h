#ifndef __SYMGRAMCRYPTO_H__
#define __SYMGRAMCRYPTO_H__

#include <e32base.h>

const TInt KBnLimbs = 64; // 2048-bit

class TBn
    {
    public:
        TUint32 iD[ KBnLimbs ];
        void Zero();
        void FromBe( const TUint8* aBytes, TInt aLen );
        void ToBe( TUint8* aBytes, TInt aLen ) const;
        TInt Cmp( const TBn& aOther ) const;
        TBool IsZero() const;
    };

void Sha1( const TUint8* aData, TInt aLen, TUint8 aOut[ 20 ] );
void Sha256( const TUint8* aData, TInt aLen, TUint8 aOut[ 32 ] );
void Aes256IgeEncrypt( const TUint8* aIn, TUint8* aOut, TInt aLen,
                       const TUint8 aKey[ 32 ], const TUint8 aIv[ 32 ] );
void Aes256IgeDecrypt( const TUint8* aIn, TUint8* aOut, TInt aLen,
                       const TUint8 aKey[ 32 ], const TUint8 aIv[ 32 ] );

void Sha512( const TUint8* aData, TInt aLen, TUint8 aOut[ 64 ] );
void HmacSha512( const TUint8* aKey, TInt aKeyLen,
                 const TUint8* aData, TInt aLen, TUint8 aOut[ 64 ] );
void Pbkdf2HmacSha512( const TUint8* aPass, TInt aPassLen,
                       const TUint8* aSalt, TInt aSaltLen,
                       TInt aIter, TUint8 aOut[ 64 ] );
void Pbkdf2HmacSha512Begin( const TUint8* aPass, TInt aPassLen,
                            const TUint8* aSalt, TInt aSaltLen,
                            TUint8 aU[ 64 ], TUint8 aT[ 64 ] );
void Pbkdf2HmacSha512Rounds( const TUint8* aPass, TInt aPassLen,
                             TUint8 aU[ 64 ], TUint8 aT[ 64 ], TInt aCount );

void BnMulMod( TBn& aOut, const TBn& aA, const TBn& aB, const TBn& aMod );
void BnSubMod( TBn& aOut, const TBn& aA, const TBn& aB, const TBn& aMod );
void BnModExp( TBn& aOut, const TBn& aBase, const TBn& aExp, const TBn& aMod );
void BnModExpN( TBn& aOut, const TBn& aBase,
                const TUint32* aExp, TInt aExpLimbs, const TBn& aMod );
void BnRsaPublic( TUint8 aOut[ 256 ], const TUint8 aIn[ 256 ],
                  const TUint8 aMod[ 256 ] );

TBool FactorPq( TUint64 aPq, TUint64& aP, TUint64& aQ );

void TlAppendBytes( TDes8& aOut, const TUint8* aData, TInt aLen );
TInt TlReadBytes( const TUint8* aP, TInt aRemain, const TUint8*& aData, TInt& aLen );
TInt InflateTlGzip( const TUint8* aIn, TInt aInLen, TUint8* aOut, TInt aOutMax,
                    TInt& aOutLen );

extern const TUint8 KTelegramRsaN[ 256 ];

#endif
