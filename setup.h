#pragma once

#include <vdr/menu.h>
#include <vdr/tools.h>
#include "config.h"

class cFlatSetup : public cMenuSetupPage {
    public:
        cFlatSetup(void);
        virtual ~cFlatSetup();      
    private:
        cFlatConfig SetupConfig;
    
        int ItemLastSel;
        void Setup(void);
    protected:
        virtual void Store(void);
        virtual eOSState ProcessKey(eKeys Key);
};

class cMenuSetupSubMenu : public cOsdMenu {
    protected:
        cFlatConfig *SetupConfig;
        virtual void Setup(void) = 0;
        cOsdItem *InfoItem(const char *label, const char *value);
        int ItemLastSel;
    public:
        cMenuSetupSubMenu(const char *Title, cFlatConfig *data);
};

class cFlatSetupGeneral : public cMenuSetupSubMenu {
    protected:
        void Setup(void);
    public:
        cFlatSetupGeneral(cFlatConfig *data);
        virtual eOSState ProcessKey(eKeys Key);
};

class cFlatSetupChannelInfo : public cMenuSetupSubMenu {
    protected:
        void Setup(void);
    public:
        cFlatSetupChannelInfo(cFlatConfig *data);
        virtual eOSState ProcessKey(eKeys Key);
};

class cFlatSetupMenu : public cMenuSetupSubMenu {
    protected:
        void Setup(void);
    public:
        cFlatSetupMenu(cFlatConfig *data);
        virtual eOSState ProcessKey(eKeys Key);
};

class cFlatSetupReplay : public cMenuSetupSubMenu {
    protected:
        void Setup(void);
    public:
        cFlatSetupReplay(cFlatConfig *data);
        virtual eOSState ProcessKey(eKeys Key);
};

class cFlatSetupVolume : public cMenuSetupSubMenu {
    protected:
        void Setup(void);
    public:
        cFlatSetupVolume(cFlatConfig *data);
        virtual eOSState ProcessKey(eKeys Key);
};

class cFlatSetupTracks : public cMenuSetupSubMenu {
    protected:
        void Setup(void);
    public:
        cFlatSetupTracks(cFlatConfig *data);
        virtual eOSState ProcessKey(eKeys Key);
};