#ifndef __TELEGRAMN95APPUI_H__
#define __TELEGRAMN95APPUI_H__

#include <aknappui.h>

class CTelegramN95AppView;

class CTelegramN95AppUi : public CAknAppUi
    {
    public:
        void ConstructL();
        CTelegramN95AppUi();
        ~CTelegramN95AppUi();

    private:
        void HandleCommandL( TInt aCommand );
        void HandleStatusPaneSizeChange();

    private:
        void ShowAboutL();

    private:
        CTelegramN95AppView* iAppView;
    };

#endif // __TELEGRAMN95APPUI_H__
