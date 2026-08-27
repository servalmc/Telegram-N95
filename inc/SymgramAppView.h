#ifndef __SYMGRAMAPPVIEW_H__
#define __SYMGRAMAPPVIEW_H__

#include <coecntrl.h>

class CSymgramAppView : public CCoeControl
    {
    public:
        static CSymgramAppView* NewL( const TRect& aRect );
        static CSymgramAppView* NewLC( const TRect& aRect );
        ~CSymgramAppView();

    public:
        void SetStatusL( const TDesC& aStatus );

    private:
        void Draw( const TRect& aRect ) const;
        void SizeChanged();

    private:
        void ConstructL( const TRect& aRect );
        CSymgramAppView();

    private:
        const CFont* iFont;
        HBufC* iStatus;
    };

#endif // __SYMGRAMAPPVIEW_H__
