#ifndef __SYMGRAMSTORE_H__
#define __SYMGRAMSTORE_H__

#include <e32base.h>
#include <f32file.h>
#include "SymgramTypes.h"

class CSymgramStore : public CBase
    {
    public:
        static CSymgramStore* NewL();
        ~CSymgramStore();

        void LoadChatsL( RArray<TSymgramChat>& aChats );
        void SaveChats( const RArray<TSymgramChat>& aChats );
        void LoadContactsL( RArray<TSymgramContact>& aContacts );
        void SaveContacts( const RArray<TSymgramContact>& aContacts );
        void LoadMessagesL( TInt64 aPeer, RArray<TSymgramMsg>& aMsgs );
        void SaveMessages( TInt64 aPeer, const RArray<TSymgramMsg>& aMsgs );
        void ClearAll();

    private:
        CSymgramStore();
        void ConstructL();
        TInt PrivateFile( const TDesC& aName, TFileName& aOut );
        void MsgFile( TInt64 aPeer, TFileName& aOut );

    private:
        RFs iFs;
    };

#endif
