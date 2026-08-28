#ifndef __SYMGRAMAPPUI_H__
#define __SYMGRAMAPPUI_H__

#include <aknappui.h>
#include <eikmenup.h>

class CSymgramAppView;

class CSymgramAppUi : public CAknAppUi
    {
    public:
        void ConstructL();
        CSymgramAppUi();
        ~CSymgramAppUi();

    private:
        void HandleCommandL( TInt aCommand );
        void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );
        void HandleStatusPaneSizeChange();
        void HandleWsEventL( const TWsEvent& aEvent, CCoeControl* aDestination );

    private:
        void ShowAboutL();
        TBool IsNaviKey( const TKeyEvent& aKey ) const;

    private:
        CSymgramAppView* iAppView;
    };

#endif // __SYMGRAMAPPUI_H__
