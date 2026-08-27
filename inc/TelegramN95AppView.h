#ifndef __TELEGRAMN95APPVIEW_H__
#define __TELEGRAMN95APPVIEW_H__

#include <coecntrl.h>

class CTelegramN95AppView : public CCoeControl
    {
    public:
        static CTelegramN95AppView* NewL( const TRect& aRect );
        static CTelegramN95AppView* NewLC( const TRect& aRect );
        ~CTelegramN95AppView();

    public:
        void SetStatusL( const TDesC& aStatus );

    private:
        void Draw( const TRect& aRect ) const;
        void SizeChanged();

    private:
        void ConstructL( const TRect& aRect );
        CTelegramN95AppView();

    private:
        const CFont* iFont;
        TBuf<64> iStatus;
    };

#endif // __TELEGRAMN95APPVIEW_H__
