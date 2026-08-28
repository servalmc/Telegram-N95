#ifndef __SYMGRAMTYPES_H__
#define __SYMGRAMTYPES_H__

#include <e32base.h>
#include <e32std.h>

enum TSymgramKind
    {
    ESgText = 0,
    ESgPhoto = 1,
    ESgVoice = 2,
    ESgAudio = 3,
    ESgVideo = 4,
    ESgSticker = 5,
    ESgFile = 6,
    ESgService = 7
    };

class TSymgramChat
    {
    public:
        TInt64   iId;
        TInt64   iHash;
        TInt     iPeerKind;
        TInt     iUnread;
        TInt     iDate;
        TBuf<40> iName;
        TBuf<80> iPreview;
        TBuf<12> iTime;
    };

class TSymgramContact
    {
    public:
        TInt64   iId;
        TInt64   iHash;
        TBuf<40> iName;
        TBuf<24> iPhone;
    };

class CFbsBitmap;

class TSymgramMsg
    {
    public:
        TInt         iId;
        TInt         iDate;
        TBool        iOut;
        TUint8       iKind;
        TInt         iDuration;
        TBool        iPhotoLoc;
        TInt64       iFileId;
        TInt64       iFileHash;
        TBuf8<96>    iFileRef;
        TBuf<40>     iFileName;
        TBuf<140>    iText;
        CFbsBitmap*  iBmp;
    };

#endif
