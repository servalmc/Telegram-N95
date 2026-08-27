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

void BnModExp( TBn& aOut, const TBn& aBase, const TBn& aExp, const TBn& aMod );
void BnRsaPublic( TUint8 aOut[ 256 ], const TUint8 aIn[ 256 ],
                  const TUint8 aMod[ 256 ] );

TBool FactorPq( TUint64 aPq, TUint64& aP, TUint64& aQ );

void TlAppendBytes( TDes8& aOut, const TUint8* aData, TInt aLen );
TInt TlReadBytes( const TUint8* aP, TInt aRemain, const TUint8*& aData, TInt& aLen );

extern const TUint8 KTelegramRsaN[ 256 ];

#endif
