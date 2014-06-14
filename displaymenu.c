#include "displaymenu.h"
#include "services/scraper2vdr.h"
#include "services/epgsearch.h"

#ifndef VDRLOGO
    #define VDRLOGO "vdrlogo_default"
#endif

#include "symbols/1080/Cnew.xpm"
#include "symbols/1080/Carrowturn.xpm"
#include "symbols/1080/Crec.xpm"
#include "symbols/1080/Cclock.xpm"
#include "symbols/1080/Cclocksml.xpm"
#include "symbols/1080/Cvpssml.xpm"
#include "flat.h"
#include "locale"

cBitmap cFlatDisplayMenu::bmCNew(Cnew_xpm);
cBitmap cFlatDisplayMenu::bmCArrowTurn(Carrowturn_xpm);
cBitmap cFlatDisplayMenu::bmCRec(Crec_xpm);
cBitmap cFlatDisplayMenu::bmCClock(Cclock_xpm);
cBitmap cFlatDisplayMenu::bmCClocksml(Cclocksml_xpm);
cBitmap cFlatDisplayMenu::bmCVPS(Cvpssml_xpm);

/* Possible values of the stream content descriptor according to ETSI EN 300 468 */
enum stream_content
{
	sc_reserved       = 0x00,
	sc_video_MPEG2    = 0x01,
	sc_audio_MP2      = 0x02, // MPEG 1 Layer 2 audio
	sc_subtitle       = 0x03,
	sc_audio_AC3      = 0x04,
	sc_video_H264_AVC = 0x05,
	sc_audio_HEAAC    = 0x06,
};

cFlatDisplayMenu::cFlatDisplayMenu(void) {
    CreateFullOsd();
    TopBarCreate();
    ButtonsCreate();
    MessageCreate();

    VideoDiskUsageState = -1;

    itemHeight = fontHeight + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;
    itemChannelHeight = fontHeight + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;
    itemTimerHeight = fontHeight + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;

    scrollBarWidth = ScrollBarWidth() + marginItem;
    scrollBarHeight = osdHeight - (topBarHeight + Config.decorBorderTopBarSize*2 + marginItem*3 + buttonsHeight + Config.decorBorderButtonSize*2);

    scrollBarTop = topBarHeight + marginItem + Config.decorBorderTopBarSize*2;
    isScrolling = false;
    ShowEvent = false;
    ShowRecording = false;
    ShowText = false;

    ItemEventLastChannelName = "";

    menuWidth = osdWidth;
    menuTop = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuItemSize;
    menuPixmap = osd->CreatePixmap(1, cRect(0, menuTop, menuWidth, scrollBarHeight ));

    menuIconsBGPixmap = osd->CreatePixmap(2, cRect(0, menuTop, menuWidth, scrollBarHeight ));
    menuIconsPixmap = osd->CreatePixmap(3, cRect(0, menuTop, menuWidth, scrollBarHeight ));

    chLeft = Config.decorBorderMenuContentHeadSize;
    chTop = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuContentHeadSize;
    chWidth = menuWidth - Config.decorBorderMenuContentHeadSize*2;
    chHeight = fontHeight + fontSmlHeight*2 + marginItem*2;
    contentHeadPixmap = osd->CreatePixmap(1, cRect(chLeft, chTop, chWidth, chHeight));

    scrollbarPixmap = osd->CreatePixmap(2, cRect(0, scrollBarTop, menuWidth, scrollBarHeight + buttonsHeight + Config.decorBorderButtonSize*2));

    menuPixmap->Fill(clrTransparent);
    menuIconsPixmap->Fill(clrTransparent);
    menuIconsBGPixmap->Fill(clrTransparent);
    scrollbarPixmap->Fill(clrTransparent);

    menuCategory = mcUndefined;

    bmNew       = &bmCNew;
    bmRec       = &bmCRec;
    bmArrowTurn = &bmCArrowTurn;
    bmClock     = &bmCClock;
    bmClocksml  = &bmCClocksml;
    bmVPS       = &bmCVPS;
}

cFlatDisplayMenu::~cFlatDisplayMenu() {
    osd->DestroyPixmap(menuPixmap);
    osd->DestroyPixmap(menuIconsPixmap);
    osd->DestroyPixmap(menuIconsBGPixmap);
    osd->DestroyPixmap(scrollbarPixmap);
    osd->DestroyPixmap(contentHeadPixmap);
}

void cFlatDisplayMenu::SetMenuCategory(eMenuCategory MenuCategory) {
    ItemBorderClear();
    isScrolling = false;
    ShowRecording = ShowEvent = ShowText = false;

    menuCategory = MenuCategory;

    if( menuCategory == mcChannel ) {
        if( Config.MenuChannelView == 0 || Config.MenuChannelView == 1 || Config.MenuChannelView == 2 )
            itemChannelHeight = fontHeight + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;
        else if( Config.MenuChannelView == 3 || Config.MenuChannelView == 4 )
            itemChannelHeight = fontHeight + fontSmlHeight + marginItem + Config.decorProgressMenuItemSize + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;
    } else if( menuCategory == mcTimer ) {
        if( Config.MenuTimerView == 0 || Config.MenuTimerView == 1 )
            itemTimerHeight = fontHeight + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;
        else if( Config.MenuTimerView == 2 || Config.MenuTimerView == 3 )
            itemTimerHeight = fontHeight + fontSmlHeight + marginItem + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;
    } else if( menuCategory == mcSchedule || menuCategory == mcScheduleNow || menuCategory == mcScheduleNext ) {
        if( Config.MenuEventView == 0 || Config.MenuEventView == 1 )
            itemEventHeight = fontHeight + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;
        else if( Config.MenuEventView == 2 || Config.MenuEventView == 3 )
            itemEventHeight = fontHeight + fontSmlHeight + marginItem*2 + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2 + Config.decorProgressMenuItemSize/2;
    } else if( menuCategory == mcRecording ) {
        if( Config.MenuRecordingView == 0 || Config.MenuRecordingView == 1 )
            itemRecordingHeight = fontHeight + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;
        else if( Config.MenuRecordingView == 2 || Config.MenuRecordingView == 3 )
            itemRecordingHeight = fontHeight + fontSmlHeight + marginItem + Config.MenuItemPadding + Config.decorBorderMenuItemSize*2;
    }

}

void cFlatDisplayMenu::SetScrollbar(int Total, int Offset) {
    DrawScrollbar(Total, Offset, MaxItems(), 0, ItemsHeight(), Offset > 0, Offset + MaxItems() < Total);
}

void cFlatDisplayMenu::DrawScrollbar(int Total, int Offset, int Shown, int Top, int Height, bool CanScrollUp, bool CanScrollDown, bool isContent) {
    //dsyslog("Total: %d Offset: %d Shown: %d Top: %d Height: %d", Total, Offset, Shown, Top, Height);

    if( Total > 0 && Total > Shown ) {
        if( isScrolling == false && ShowEvent == false && ShowRecording == false && ShowText == false ) {
            isScrolling = true;
            DecorBorderClearByFrom(BorderMenuItem);
            ItemBorderDrawAllWithScrollbar();
            ItemBorderClear();
            if( isContent )
                menuPixmap->DrawRectangle(cRect(menuItemWidth - scrollBarWidth + Config.decorBorderMenuContentSize, 0, scrollBarWidth + marginItem, scrollBarHeight), clrTransparent);
            else
                menuPixmap->DrawRectangle(cRect(menuItemWidth - scrollBarWidth + Config.decorBorderMenuItemSize, 0, scrollBarWidth + marginItem, scrollBarHeight), clrTransparent);
        }
    } else if( ShowEvent == false && ShowRecording == false && ShowText == false ) {
        isScrolling = false;
    }

    if( isContent )
        ScrollbarDraw(scrollbarPixmap, menuItemWidth - scrollBarWidth + Config.decorBorderMenuContentSize*2 + marginItem, Top, Height, Total, Offset, Shown, CanScrollUp, CanScrollDown);
    else
        ScrollbarDraw(scrollbarPixmap, menuItemWidth - scrollBarWidth + Config.decorBorderMenuItemSize*2 + marginItem, Top, Height, Total, Offset, Shown, CanScrollUp, CanScrollDown);

}

void cFlatDisplayMenu::Scroll(bool Up, bool Page) {
    // Wird das Men� gescrollt oder Content?
    if( ComplexContent.IsShown() && ComplexContent.IsScrollingActive() && ComplexContent.Scrollable() ) {
        bool scrolled = ComplexContent.Scroll(Up, Page);
        if( scrolled ) {
            DrawScrollbar(ComplexContent.ScrollTotal(), ComplexContent.ScrollOffset(), ComplexContent.ScrollShown(), ComplexContent.Top() - scrollBarTop, ComplexContent.Height(), ComplexContent.ScrollOffset() > 0, ComplexContent.ScrollOffset() + ComplexContent.ScrollShown() < ComplexContent.ScrollTotal(), true);
        }
    } else {
        cSkinDisplayMenu::Scroll(Up, Page);
    }
}

int cFlatDisplayMenu::MaxItems(void) {
    if( menuCategory == mcChannel )
        return scrollBarHeight / itemChannelHeight;
    else if( menuCategory == mcTimer )
        return scrollBarHeight / itemTimerHeight;
    else if( menuCategory == mcSchedule || menuCategory == mcScheduleNow || menuCategory == mcScheduleNext )
        return scrollBarHeight / itemEventHeight;
    else if( menuCategory == mcRecording )
        return scrollBarHeight / itemRecordingHeight;

    return scrollBarHeight / itemHeight;
}

int cFlatDisplayMenu::ItemsHeight(void) {
    if( menuCategory == mcChannel )
        return MaxItems() * itemChannelHeight - Config.MenuItemPadding;
    else if( menuCategory == mcTimer )
        return MaxItems() * itemTimerHeight - Config.MenuItemPadding;
    else if( menuCategory == mcSchedule || menuCategory == mcScheduleNow || menuCategory == mcScheduleNext )
        return MaxItems() * itemEventHeight - Config.MenuItemPadding;
    else if( menuCategory == mcRecording )
        return MaxItems() * itemRecordingHeight - Config.MenuItemPadding;

    return MaxItems() * itemHeight - Config.MenuItemPadding;
}

void cFlatDisplayMenu::Clear(void) {
    textScroller.Reset();
    menuPixmap->Fill(clrTransparent);
    menuIconsPixmap->Fill(clrTransparent);
    menuIconsBGPixmap->Fill(clrTransparent);
    scrollbarPixmap->Fill(clrTransparent);
    contentHeadPixmap->Fill(clrTransparent);
    DecorBorderClearByFrom(BorderMenuItem);
    DecorBorderClearByFrom(BorderContent);
    DecorBorderClearAll();
    isScrolling = false;

    ComplexContent.Clear();
    ShowRecording = ShowEvent = ShowText = false;
}

void cFlatDisplayMenu::SetTitle(const char *Title) {
    TopBarSetTitle(Title);

    if( Config.TopBarMenuIconShow ) {
        cString icon;
        switch( menuCategory ) {
            case mcMain:
                TopBarSetTitle("");
                icon = cString::sprintf("menuIcons/%s", VDRLOGO);
                break;
            case mcSchedule:
            case mcScheduleNow:
            case mcScheduleNext:
                icon = "menuIcons/Schedule";
                break;
            case mcChannel:
                icon = "menuIcons/Channels";
                break;
            case mcTimer:
                icon = "menuIcons/Timers";
                break;
            case mcRecording:
                icon = "menuIcons/Recordings";
                break;
            case mcSetup:
                icon = "menuIcons/Setup";
                break;
            case mcCommand:
                icon = "menuIcons/Commands";
                break;
            case mcEvent:
                icon = "extraIcons/Info";
                break;
            case mcRecordingInfo:
                icon = "extraIcons/PlayInfo";
                break;
            default:
                icon = "";
        }
        TopBarSetMenuIcon(icon);

        if( (menuCategory == mcRecording || menuCategory == mcTimer) && Config.DiskUsageShow == 1 || Config.DiskUsageShow == 2 | Config.DiskUsageShow == 3) {
            TopBarEnableDiskUsage();
        }
    }
}

void cFlatDisplayMenu::SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue) {
    ButtonsSet(Red, Green, Yellow, Blue);
}

void cFlatDisplayMenu::SetMessage(eMessageType Type, const char *Text) {
    if (Text)
        MessageSet(Type, Text);
    else
        MessageClear();
}

void cFlatDisplayMenu::SetItem(const char *Text, int Index, bool Current, bool Selectable) {
    ShowEvent = false;
    ShowRecording = false;
    ShowText = false;

    int y = Index * itemHeight;
    menuItemWidth = menuWidth - Config.decorBorderMenuItemSize*2;

    if( menuCategory == mcMain )
        menuItemWidth *= Config.MainMenuItemScale;

    int AvailableTextWidth = menuItemWidth - scrollBarWidth;
    if( isScrolling )
        menuItemWidth -= scrollBarWidth;

    tColor ColorFg, ColorBg, ColorExtraTextFg;
    ColorExtraTextFg = Theme.Color(clrMenuItemExtraTextFont);
    if (Current) {
        ColorFg = Theme.Color(clrItemCurrentFont);
        ColorBg = Theme.Color(clrItemCurrentBg);
        ColorExtraTextFg = Theme.Color(clrMenuItemExtraTextCurrentFont);
    }
    else {
        if( Selectable ) {
            ColorFg = Theme.Color(clrItemSelableFont);
            ColorBg = Theme.Color(clrItemSelableBg);
        } else {
            ColorFg = Theme.Color(clrItemFont);
            ColorBg = Theme.Color(clrItemBg);
        }
    }
    menuPixmap->DrawRectangle(cRect(Config.decorBorderMenuItemSize, y, menuItemWidth, fontHeight), ColorBg);
    int lh = fontHeight;
    int x1 = 0;
    int xOff = x1;
    for (int i = 0; i < MaxTabs; i++) {
        const char *s = GetTabbedText(Text, i);
        if( s ) {
            // from skinelchi
            char buffer[9];
            bool istimer = false;
            bool isnewrecording = false;
            bool hasEventtimer = false;
            bool haspartEventtimer = false;
            bool isRecording = false;
            bool hasVPS = false;
            bool isRunning = false;

            int xt = Tab(i);
            int xt2 = Tab(i+1);
            if( xt2 == 0 || i == MaxTabs || (GetTabbedText(Text, i+1) == NULL) )
                xt2 = menuItemWidth;

            if( xt >= menuItemWidth )
                continue;
            if( true ) {
                // check if timer info symbols: " !#>"
                if (i == 0 && strlen(s) == 1 && strchr(" !#>", s[0])) {
                    istimer = true; // update status
                } else if (  (strlen(s) == 6 && s[5] == '*' && s[2] == ':' && isdigit(*s)
                    && isdigit(*(s + 1)) && isdigit(*(s + 3)) && isdigit(*(s + 4)))
                    || (strlen(s) == 5 && s[4] == '*' && s[1] == ':' && isdigit(*s)
                    && isdigit(*(s + 2)) && isdigit(*(s + 3)))
                    || (strlen(s) == 9 && s[8] == '*' && s[5] == '.' && s[2] == '.'
                    && isdigit(*s) && isdigit(*(s + 1)) && isdigit(*(s + 3))
                    && isdigit(*(s + 4)) && isdigit(*(s + 6)) && isdigit(*(s + 7)))) {
                    // check if new recording: "10:10*", "1:10*", "01.01.06*"

                    isnewrecording = true;  // update status
                    strncpy(buffer, s, strlen(s));   // make a copy
                    buffer[strlen(s) - 1] = '\0';  // remove the '*' character
                } else if ( (strlen(s) == 3) && ( i == 2 || i == 3 || i == 4) ) {
                     if (s[0] == 'R') isRecording = true;
                     if (s[0] == 'T') hasEventtimer = true;
                     if (s[0] == 't') haspartEventtimer = true;
                     if (s[1] == 'V') hasVPS = true;
                     if (s[2] == '*') isRunning = true;
                } else if ( (strlen(s) == 4) && ( i == 3 ) ) { //epgsearch What's on now default
                        if (s[1] == 'R') isRecording = true;
                        if (s[1] == 'T') hasEventtimer = true;
                        if (s[1] == 't') haspartEventtimer = true;
                        if (s[2] == 'V') hasVPS = true;
                        if (s[3] == '*') isRunning = true;
                }
            }
            xOff = x1 + Tab(i) + Config.decorBorderMenuItemSize;

            if( istimer ) {
                // timer menu
                switch( s[0] ) {
                    case '!':
                       menuPixmap->DrawBitmap(cPoint(xOff, y + (lh - bmArrowTurn->Height()) / 2), *bmArrowTurn, ColorFg, ColorBg);
                       break;
                    case '#':
                       menuPixmap->DrawBitmap(cPoint(xOff, y + (lh - bmRec->Height()) / 2), *bmRec, ColorFg, ColorBg);
                       break;
                    case '>':
                       menuPixmap->DrawBitmap(cPoint(xOff, y + (lh - bmClock->Height()) / 2), *bmClock, ColorFg, ColorBg);
                       break;
                    case ' ':
                    default:
                       break;
                }
            } else if( isRecording || hasEventtimer || haspartEventtimer || hasVPS || isRunning ) {
                // program schedule menu
                if( isRecording )
                   menuPixmap->DrawBitmap(cPoint(xOff, y + (lh - bmRec->Height()) / 2), *bmRec, ColorFg, ColorBg);
                else {
                   if( hasEventtimer )
                      menuPixmap->DrawBitmap(cPoint(xOff, y + (lh - bmClock->Height()) / 2), *bmClock, ColorFg, ColorBg);
                   if( haspartEventtimer )
                      menuPixmap->DrawBitmap(cPoint(xOff + (bmClock->Height() - bmClocksml->Height()) / 2, y + (lh - bmClocksml->Height()) / 2), *bmClocksml, ColorFg, ColorBg);
                }
                xOff += bmClock->Width(); // clock is wider than rec

                if( hasVPS )
                   menuPixmap->DrawBitmap(cPoint(xOff, y + (lh - bmVPS->Height()) / 2), *bmVPS, ColorFg, ColorBg);
                xOff += bmVPS->Width();

                if( isRunning )
                    menuPixmap->DrawText(cPoint(xOff, y), "*", ColorFg, ColorBg, font, AvailableTextWidth - xOff);

            } else if( isnewrecording ) {
                // recordings menu
                menuPixmap->DrawText(cPoint(xOff, y), buffer, ColorFg, ColorBg, font, AvailableTextWidth - xOff);

                // draw symbol "new" centered
                int gap = std::max(0, (Tab(i+1)-Tab(i)- font->Width(buffer) - bmNew->Width()) / 2);
                menuPixmap->DrawBitmap(cPoint(xOff + font->Width(buffer) + gap, y + (lh - bmNew->Height()) / 2), *bmNew, ColorFg, ColorBg);
            } else if( CheckProgressBar( s ) ) {
                int colWidth = Tab(i+1) - Tab(i);

                tColor ColorFg = Config.decorProgressMenuItemFg;
                tColor ColorBarFg = Config.decorProgressMenuItemBarFg;
                tColor ColorBg = Config.decorProgressMenuItemBg;
                if( Current ) {
                    ColorFg = Config.decorProgressMenuItemCurFg;
                    ColorBarFg = Config.decorProgressMenuItemCurBarFg;
                    ColorBg = Config.decorProgressMenuItemCurBg;
                }
                cRect rec = cRect(xt + Config.decorBorderMenuItemSize,
                    y + (itemHeight-Config.MenuItemPadding)/2 - Config.decorProgressMenuItemSize/2 - Config.decorBorderMenuItemSize,
                    colWidth, Config.decorProgressMenuItemSize);
                cRect recBG = cRect(xt + Config.decorBorderMenuItemSize - marginItem, y,
                    colWidth + marginItem*2, fontHeight);

                DrawProgressBarFromText(rec, recBG, s, ColorFg, ColorBarFg, ColorBg);
            } else {
                if( (menuCategory == mcMain || menuCategory == mcSetup) && Config.MenuItemIconsShow) {
                    cString cIcon = GetIconName( MainMenuText(s) );
                    cImageLoader imgLoader;
                    cImage *img = imgLoader.LoadIcon(*cIcon, fontHeight - marginItem*2, fontHeight - marginItem*2);
                    if( img ) {
                        menuIconsPixmap->DrawImage(cPoint(xt + Config.decorBorderMenuItemSize + marginItem, y + marginItem), *img);
                    } else {
                        img = imgLoader.LoadIcon("menuIcons/blank", fontHeight - marginItem*2, fontHeight - marginItem*2);
                        if( img ) {
                            menuIconsPixmap->DrawImage(cPoint(xt + Config.decorBorderMenuItemSize + marginItem, y + marginItem), *img);
                        }
                    }
                    menuPixmap->DrawText(cPoint(fontHeight + marginItem*2 + xt + Config.decorBorderMenuItemSize, y), s, ColorFg, ColorBg, font,
                        AvailableTextWidth - xt - marginItem*2 - fontHeight);
                } else {
                    if( Config.MenuItemParseTilde ) {
                        std::string tilde = s;
                        size_t found = tilde.find(" ~ ");
                        size_t found2 = tilde.find("~");
                        if( found != string::npos ) {
                            std::string first = tilde.substr(0, found);
                            std::string second = tilde.substr(found +2, tilde.length() );

                            menuPixmap->DrawText(cPoint(xt + Config.decorBorderMenuItemSize, y), first.c_str(), ColorFg, ColorBg, font);
                            int l = font->Width( first.c_str() );
                            menuPixmap->DrawText(cPoint(xt + Config.decorBorderMenuItemSize + l, y), second.c_str(), ColorExtraTextFg, ColorBg, font, xt2 - xt);
                        } else if ( found2 != string::npos ) {
                            std::string first = tilde.substr(0, found2);
                            std::string second = tilde.substr(found2 +1, tilde.length() );

                            menuPixmap->DrawText(cPoint(xt + Config.decorBorderMenuItemSize, y), first.c_str(), ColorFg, ColorBg, font, xt2 - xt);
                            int l = font->Width( first.c_str() );
                            l += font->Width("X");
                            menuPixmap->DrawText(cPoint(xt + Config.decorBorderMenuItemSize + l, y), second.c_str(), ColorExtraTextFg, ColorBg, font, xt2 - xt);
                        } else
                            menuPixmap->DrawText(cPoint(xt + Config.decorBorderMenuItemSize, y), s, ColorFg, ColorBg, font, xt2 - xt);
                    } else
                        menuPixmap->DrawText(cPoint(xt + Config.decorBorderMenuItemSize, y), s, ColorFg, ColorBg, font, xt2 - xt);
                }
            }
        }
        if (!Tab(i + 1))
            break;
    }

    sDecorBorder ib;
    ib.Left = Config.decorBorderMenuItemSize;
    ib.Top = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuItemSize + y;

    ib.Width = menuWidth - Config.decorBorderMenuItemSize*2;
    if( isScrolling ) {
        ib.Width -= scrollBarWidth;
    }

    ib.Width = menuItemWidth;

    ib.Height = fontHeight;
    ib.Size = Config.decorBorderMenuItemSize;
    ib.Type = Config.decorBorderMenuItemType;

    if( Current ) {
        ib.ColorFg = Config.decorBorderMenuItemCurFg;
        ib.ColorBg = Config.decorBorderMenuItemCurBg;
    } else {
        if( Selectable ) {
            ib.ColorFg = Config.decorBorderMenuItemSelFg;
            ib.ColorBg = Config.decorBorderMenuItemSelBg;
        } else {
            ib.ColorFg = Config.decorBorderMenuItemFg;
            ib.ColorBg = Config.decorBorderMenuItemBg;
        }
    }

    DecorBorderDraw(ib.Left, ib.Top, ib.Width, ib.Height,
        ib.Size, ib.Type, ib.ColorFg, ib.ColorBg, BorderMenuItem);

    if( !isScrolling ) {
        ItemBorderInsertUnique(ib);
    }

    SetEditableWidth(menuWidth - Tab(1));
}

std::string cFlatDisplayMenu::items[16] = { "Schedule", "Channels", "Timers", "Recordings", "Setup", "Commands",
                                                "OSD", "EPG", "DVB", "LNB", "CAM", "Recording", "Replay", "Miscellaneous", "Plugins", "Restart"};

std::string cFlatDisplayMenu::MainMenuText(std::string Text) {
    std::string text = skipspace(Text.c_str());
    std::string menuEntry;
    std::string menuNumber;
    bool found = false;
    bool doBreak = false;
    size_t i = 0;
    for (; i < text.length(); i++) {
        char s = text.at(i);
        if (i==0) {
            //if text directly starts with nonnumeric, break
            if (!(s >= '0' && s <= '9')) {
                break;
            }
        }
        if (found) {
            if (!(s >= '0' && s <= '9')) {
                doBreak = true;
            }
        }
        if (s >= '0' && s <= '9') {
            found = true;
        }
        if (doBreak)
            break;
        if (i>4)
            break;
    }
    if (found) {
        menuNumber = skipspace(text.substr(0,i).c_str());
        menuEntry = skipspace(text.substr(i).c_str());
    } else {
        menuNumber = "";
        menuEntry = text.c_str();
    }
    return menuEntry;
}

cString cFlatDisplayMenu::GetIconName(std::string element) {
    //check for standard menu entries
    for (int i=0; i<16; i++) {
        std::string s = trVDR(items[i].c_str());
        if (s == element) {
            cString menuIcon = cString::sprintf("menuIcons/%s", items[i].c_str());
            return menuIcon;
        }
    }
    //check for special main menu entries "stop recording", "stop replay"
    std::string stopRecording = skipspace(trVDR(" Stop recording "));
    std::string stopReplay = skipspace(trVDR(" Stop replaying"));
    try {
        if (element.substr(0, stopRecording.size()) == stopRecording)
            return "menuIcons/StopRecording";
        if (element.substr(0, stopReplay.size()) == stopReplay)
            return "menuIcons/StopReplay";
    } catch (...) {}
    //check for Plugins
    for (int i = 0; ; i++) {
        cPlugin *p = cPluginManager::GetPlugin(i);
        if (p) {
            const char *mainMenuEntry = p->MainMenuEntry();
            if (mainMenuEntry) {
                std::string plugMainEntry = mainMenuEntry;
                try {
                    if (element.substr(0, plugMainEntry.size()) == plugMainEntry) {
                        return cString::sprintf("pluginIcons/%s", p->Name());
                    }
                } catch (...) {}
            }
        } else
            break;
    }
    return cString::sprintf("extraIcons/%s", element.c_str());
}

bool cFlatDisplayMenu::CheckProgressBar(const char *text) {
    if (strlen(text) > 5
        && text[0] == '['
        && ((text[1] == '|')||(text[1] == ' '))
        && ((text[2] == '|')||(text[2] == ' '))
        && text[strlen(text) - 1] == ']')
        return true;
    return false;
}

void cFlatDisplayMenu::DrawProgressBarFromText(cRect rec, cRect recBg, const char *bar, tColor ColorFg, tColor ColorBarFg, tColor ColorBg) {
    const char *p = bar + 1;
    bool isProgressbar = true;
    int total = 0;
    int now = 0;
    for (; *p != ']'; ++p) {
        if (*p == ' ' || *p == '|') {
            ++total;
            if (*p == '|')
                ++now;
        } else {
            isProgressbar = false;
            break;
        }
    }
    if (isProgressbar) {
        double progress = (double)now / (double)total;
        ProgressBarDrawRaw(menuPixmap, menuPixmap, rec, recBg, progress*total, total,
            ColorFg, ColorBarFg, ColorBg, Config.decorProgressMenuItemType, true);
    }
}

bool cFlatDisplayMenu::SetItemChannel(const cChannel *Channel, int Index, bool Current, bool Selectable, bool WithProvider) {
    if( Config.MenuChannelView == 0 || !Channel )
        return false;

    cSchedulesLock schedulesLock;
    const cSchedules *schedules = cSchedules::Schedules(schedulesLock);
    const cEvent *Event = NULL;

    bool DrawProgress = true;
    cString buffer;
    int y = Index * itemChannelHeight;

    int Height = fontHeight;
    if( Config.MenuChannelView == 3 || Config.MenuChannelView == 4 )
        Height = fontHeight + fontSmlHeight + marginItem + Config.decorProgressMenuItemSize;

    menuItemWidth = menuWidth - Config.decorBorderMenuItemSize*2;
    if( Config.MenuChannelView == 3 || Config.MenuChannelView == 4 )
        menuItemWidth *= 0.5;

    if( isScrolling )
        menuItemWidth -= scrollBarWidth;

    tColor ColorFg, ColorBg;
    if (Current) {
        ColorFg = Theme.Color(clrItemCurrentFont);
        ColorBg = Theme.Color(clrItemCurrentBg);
    }
    else {
        if( Selectable ) {
            ColorFg = Theme.Color(clrItemSelableFont);
            ColorBg = Theme.Color(clrItemSelableBg);
        } else {
            ColorFg = Theme.Color(clrItemFont);
            ColorBg = Theme.Color(clrItemBg);
        }
    }

    menuPixmap->DrawRectangle(cRect(Config.decorBorderMenuItemSize, y, menuItemWidth, Height), ColorBg);

    int Left, Top, Width;
    int LeftName;
    Left = Config.decorBorderMenuItemSize + marginItem;
    Top = y;

    if( Channel->GroupSep() )
        DrawProgress = false;
    float progress = 0.0;
    cString EventTitle = "";

    cString ws = cString::sprintf("%d", Channels.MaxNumber());
    int w = font->Width(ws);
    if( !Channel->GroupSep() )
        buffer = cString::sprintf("%d", Channel->Number());
    else
        buffer = "";
    Width = font->Width(buffer);
    if( Width < w )
        Width = w;
    menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, font, Width, fontHeight, taRight);
    Left += Width + marginItem;

    int imageHeight = fontHeight;
    int imageLeft = Left;
    int imageTop = Top;
    int imageBGHeight = imageHeight;
    int imageBGWidth = imageHeight*1.34;

    if( !Channel->GroupSep() ) {
        cImage *imgBG = imgLoader.LoadIcon("logo_background", imageHeight*1.34, imageHeight);
        if( imgBG ) {
            imageBGHeight = imgBG->Height();
            imageBGWidth = imgBG->Width();
            imageTop = Top + (fontHeight - imgBG->Height()) / 2;
            menuIconsBGPixmap->DrawImage( cPoint(imageLeft, imageTop), *imgBG );
        }
    }
    cImage *img = imgLoader.LoadLogo(Channel->Name(), imageBGWidth - 4, imageBGHeight - 4);
    if( img ) {
        imageTop = Top + (imageBGHeight - img->Height()) / 2;
        imageLeft = Left + (imageBGWidth - img->Width()) / 2;

        menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
        Left += imageBGWidth + marginItem * 2;
    } else {
        bool isRadioChannel = ((!Channel->Vpid())&&(Channel->Apid(0))) ? true : false;

        if( isRadioChannel ) {
            img = imgLoader.LoadIcon("radio", imageBGWidth - 10, imageBGHeight - 10);
            if( img ) {
                imageTop = Top + (imageBGHeight - img->Height()) / 2;
                imageLeft = Left + (imageBGWidth - img->Width()) / 2;
                menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
                Left += imageBGWidth + marginItem * 2;
            }
        } else if( Channel->GroupSep() ) {
            img = imgLoader.LoadIcon("changroup", imageBGWidth - 10, imageBGHeight - 10);
            if( img ) {
                imageTop = Top + (imageBGHeight - img->Height()) / 2;
                imageLeft = Left + (imageBGWidth - img->Width()) / 2;
                menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
                Left += imageBGWidth + marginItem * 2;
            }
        } else {
            img = imgLoader.LoadIcon("tv", imageBGWidth - 10, imageBGHeight - 10);
            if( img ) {
                imageTop = Top + (imageBGHeight - img->Height()) / 2;
                imageLeft = Left + (imageBGWidth - img->Width()) / 2;
                menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
                Left += imageBGWidth + marginItem * 2;
            }
        }
    }

    LeftName = Left;

    // event from channel
    const cSchedule *Schedule = schedules->GetSchedule( Channel->GetChannelID() );
    if( Schedule ) {
        Event = Schedule->GetPresentEvent();
        if( Event ) {
            // calculate progress bar
            progress = (int)roundf( (float)(time(NULL) - Event->StartTime()) / (float) (Event->Duration()) * 100.0);
            if(progress < 0)
                progress = 0.;
            else if(progress > 100)
                progress = 100;

            EventTitle = Event->Title();
        }
    }

    if( WithProvider )
        buffer = cString::sprintf("%s - %s", Channel->Provider(), Channel->Name());
    else
        buffer = cString::sprintf("%s", Channel->Name());

    if( Config.MenuChannelView == 1 ) {
        Width = menuItemWidth - LeftName;
        if( Channel->GroupSep() ) {
            int lineTop = Top + (fontHeight - 3) / 2;
            menuPixmap->DrawRectangle(cRect( Left, lineTop, menuItemWidth - Left, 3), ColorFg);
            cString groupname = cString::sprintf(" %s ", *buffer);
            menuPixmap->DrawText(cPoint(Left + (menuItemWidth / 10 * 2), Top), groupname, ColorFg, ColorBg, font, 0, 0, taCenter);
        } else
            menuPixmap->DrawText(cPoint(LeftName, Top), buffer, ColorFg, ColorBg, font, Width);
    } else {
        Width = menuItemWidth / 10*2;
        if( isScrolling )
            Width = (menuItemWidth + scrollBarWidth) / 10*2;

        if( Config.MenuChannelView == 3 || Config.MenuChannelView == 4 )
            Width = menuItemWidth - LeftName;

        if( Channel->GroupSep() ) {
            int lineTop = Top + (fontHeight - 3) / 2;
            menuPixmap->DrawRectangle(cRect( Left, lineTop, menuItemWidth - Left, 3), ColorFg);
            cString groupname = cString::sprintf(" %s ", *buffer);
            menuPixmap->DrawText(cPoint(Left + (menuItemWidth / 10 * 2), Top), groupname, ColorFg, ColorBg, font, 0, 0, taCenter);
        } else {
            menuPixmap->DrawText(cPoint(LeftName, Top), buffer, ColorFg, ColorBg, font, Width);

            Left += Width + marginItem;

            if( DrawProgress ) {
                int PBTop = y + (itemChannelHeight-Config.MenuItemPadding)/2 - Config.decorProgressMenuItemSize/2 - Config.decorBorderMenuItemSize;
                int PBLeft = Left;
                int PBWidth = menuItemWidth/10;
                if( Config.MenuChannelView == 3 || Config.MenuChannelView == 4 ) {
                    PBTop = Top + fontHeight + fontSmlHeight;
                    PBLeft = Left - Width - marginItem;
                    PBWidth = menuItemWidth - LeftName - marginItem*2 - Config.decorBorderMenuItemSize - scrollBarWidth;

                    if( isScrolling )
                        PBWidth += scrollBarWidth;
                }

                Width = menuItemWidth/10;
                if( isScrolling )
                    Width = (menuItemWidth + scrollBarWidth) / 10;
                if( Current )
                    ProgressBarDrawRaw(menuPixmap, menuPixmap, cRect( PBLeft, PBTop, PBWidth, Config.decorProgressMenuItemSize),
                        cRect( PBLeft, PBTop, PBWidth, Config.decorProgressMenuItemSize), progress, 100,
                        Config.decorProgressMenuItemCurFg, Config.decorProgressMenuItemCurBarFg, Config.decorProgressMenuItemCurBg, Config.decorProgressMenuItemType, false);
                else
                    ProgressBarDrawRaw(menuPixmap, menuPixmap, cRect( PBLeft, PBTop, PBWidth, Config.decorProgressMenuItemSize),
                        cRect( PBLeft, PBTop, PBWidth, Config.decorProgressMenuItemSize), progress, 100,
                        Config.decorProgressMenuItemFg, Config.decorProgressMenuItemBarFg, Config.decorProgressMenuItemBg, Config.decorProgressMenuItemType, false);
                Left += Width + marginItem;
            }

            if( Config.MenuChannelView == 3 || Config.MenuChannelView == 4 ) {
                Left = LeftName;
                Top += fontHeight;
                menuPixmap->DrawText(cPoint(Left, Top), EventTitle, ColorFg, ColorBg, fontSml, menuItemWidth - Left - marginItem );
            } else
                menuPixmap->DrawText(cPoint(Left, Top), EventTitle, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem );
        }
    }

    sDecorBorder ib;
    ib.Left = Config.decorBorderMenuItemSize;
    ib.Top = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuItemSize + y;

    ib.Width = menuItemWidth - Config.decorBorderMenuItemSize*2;

    if( isScrolling ) {
        ib.Width -= scrollBarWidth;
    }

    ib.Width = menuItemWidth;

    ib.Height = Height;
    ib.Size = Config.decorBorderMenuItemSize;
    ib.Type = Config.decorBorderMenuItemType;

    if( Current ) {
        ib.ColorFg = Config.decorBorderMenuItemCurFg;
        ib.ColorBg = Config.decorBorderMenuItemCurBg;
    } else {
        if( Selectable ) {
            ib.ColorFg = Config.decorBorderMenuItemSelFg;
            ib.ColorBg = Config.decorBorderMenuItemSelBg;
        } else {
            ib.ColorFg = Config.decorBorderMenuItemFg;
            ib.ColorBg = Config.decorBorderMenuItemBg;
        }
    }

    DecorBorderDraw(ib.Left, ib.Top, ib.Width, ib.Height,
        ib.Size, ib.Type, ib.ColorFg, ib.ColorBg, BorderMenuItem);

    if( !isScrolling ) {
        ItemBorderInsertUnique(ib);
    }

    if( Config.MenuChannelView == 4 && Current ) {
        DrawItemExtraEvent(Event, "");
    }

    return true;
}

void cFlatDisplayMenu::DrawItemExtraEvent(const cEvent *Event, cString EmptyText) {
    cLeft = menuItemWidth + Config.decorBorderMenuItemSize*2 + Config.decorBorderMenuContentSize + marginItem;
    if( isScrolling )
        cLeft += scrollBarWidth;
    cTop = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuContentSize;
    cWidth = menuWidth - cLeft - Config.decorBorderMenuContentSize;
    cHeight = osdHeight - (topBarHeight + Config.decorBorderTopBarSize*2 +
        buttonsHeight + Config.decorBorderButtonSize*2 + marginItem*3 + Config.decorBorderMenuContentSize*2);

    // Description
    ostringstream text;
    if( Event ) {
        if( !isempty(Event->Description()) ) {
            text << Event->Description();
        }

        if( Config.EpgAdditionalInfoShow ) {
            text << endl;
            const cComponents *Components = Event->Components();
            if (Components) {
                ostringstream audio;
                bool firstAudio = true;
                const char *audio_type = NULL;
                ostringstream subtitle;
                bool firstSubtitle = true;
                for (int i = 0; i < Components->NumComponents(); i++) {
                    const tComponent *p = Components->Component(i);
                    switch (p->stream) {
                        case sc_video_MPEG2:
                            if (p->description)
                                text << endl << tr("Video") << ": " <<  p->description << " (MPEG2)";
                            else
                                text << endl << tr("Video") << ": MPEG2";
                            break;
                        case sc_video_H264_AVC:
                            if (p->description)
                                text << endl << tr("Video") << ": " <<  p->description << " (H.264)";
                            else
                                text << endl << tr("Video") << ": H.264";
                            break;

                        case sc_audio_MP2:
                        case sc_audio_AC3:
                        case sc_audio_HEAAC:
                            if (firstAudio)
                                firstAudio = false;
                            else
                                audio << ", ";
                            switch (p->stream) {
                                case sc_audio_MP2:
                                    // workaround for wrongfully used stream type X 02 05 for AC3
                                    if (p->type == 5)
                                        audio_type = "AC3";
                                    else
                                        audio_type = "MP2";
                                    break;
                                case sc_audio_AC3:
                                    audio_type = "AC3"; break;
                                case sc_audio_HEAAC:
                                    audio_type = "HEAAC"; break;
                            }
                            if (p->description)
                                audio << p->description << " (" << audio_type << ", " << p->language << ")";
                            else
                                audio << p->language << " (" << audio_type << ")";
                            break;
                        case sc_subtitle:
                            if (firstSubtitle)
                                firstSubtitle = false;
                            else
                                subtitle << ", ";
                            if (p->description)
                                subtitle << p->description << " (" << p->language << ")";
                            else
                                subtitle << p->language;
                            break;
                    }
                }
                if (audio.str().length() > 0)
                    text << endl << tr("Audio") << ": "<< audio.str();
                if (subtitle.str().length() > 0)
                    text << endl << tr("Subtitle") << ": "<< subtitle.str();
            }
        }
    } else
        text << *EmptyText;

    ComplexContent.Clear();
    ComplexContent.SetScrollSize(fontSmlHeight);
    ComplexContent.SetScrollingActive(false);
    ComplexContent.SetOsd(osd);
    ComplexContent.SetPosition(cRect(cLeft, cTop, cWidth, cHeight));
    ComplexContent.SetBGColor(Theme.Color(clrMenuEventBg));

    std::string mediaPath;
    int mediaWidth = 0;
    int mediaHeight = 0;
    int mediaType = 0;

    // first try scraper2vdr
    static cPlugin *pScraper = cPluginManager::GetPlugin("scraper2vdr");
    if( !pScraper ) // if it doesn't exit, try tvscraper
        pScraper = cPluginManager::GetPlugin("tvscraper");

    if( Config.TVScraperEPGInfoShowPoster && pScraper ) {
        ScraperGetPosterBanner call;
        call.event = Event;
        if (pScraper->Service("GetPosterBanner", &call)) {
            if ((call.type == tSeries) && call.banner.path.size() > 0) {
                mediaWidth = cWidth - marginItem*2;
                mediaHeight = 999;
                mediaPath = call.banner.path;
                mediaType = 1;
            } else if (call.type == tMovie && call.poster.path.size() > 0) {
                mediaWidth = cWidth/2 - marginItem*3;
                mediaHeight = 999;
                mediaPath = call.poster.path;
                mediaType = 2;
            }
        }
    }

    if( mediaPath.length() > 0 ) {
        cImage *img = imgLoader.LoadFile(mediaPath.c_str(), mediaWidth, mediaHeight);
        if( img && mediaType == 2 ) {
            ComplexContent.AddImageWithFloatedText(img, CIP_Right, text.str().c_str(), cRect(marginItem, marginItem, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), fontSml);
        } else if( img && mediaType == 1 ) {
            ComplexContent.AddImage(img, cRect(marginItem, marginItem, img->Width(), img->Height()) );
            ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, marginItem + img->Height(), cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), fontSml);
        } else {
            ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, marginItem, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), fontSml);
        }
    } else {
        ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, marginItem, cWidth - marginItem*2, cHeight - marginItem*2),
            Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), fontSml);
    }

    ComplexContent.CreatePixmaps(Config.MenuContentFullSize);

    ComplexContent.Draw();

    DecorBorderClearByFrom(BorderContent);
    if( Config.MenuContentFullSize )
        DecorBorderDraw(cLeft, cTop, cWidth, ComplexContent.ContentHeight(true), Config.decorBorderMenuContentSize, Config.decorBorderMenuContentType,
            Config.decorBorderMenuContentFg, Config.decorBorderMenuContentBg, BorderContent);
    else
        DecorBorderDraw(cLeft, cTop, cWidth, ComplexContent.ContentHeight(false), Config.decorBorderMenuContentSize, Config.decorBorderMenuContentType,
            Config.decorBorderMenuContentFg, Config.decorBorderMenuContentBg, BorderContent);
}

bool cFlatDisplayMenu::SetItemTimer(const cTimer *Timer, int Index, bool Current, bool Selectable) {
    if( Config.MenuTimerView == 0 || !Timer )
        return false;
    const cChannel *Channel = Timer->Channel();
    const cEvent *Event = Timer->Event();

    cString buffer;
    int y = Index * itemTimerHeight;

    int Height = fontHeight;
    if( Config.MenuTimerView == 2 || Config.MenuTimerView == 3 )
        Height = fontHeight + fontSmlHeight + marginItem;

    menuItemWidth = menuWidth - Config.decorBorderMenuItemSize*2;
    if( Config.MenuTimerView == 2 || Config.MenuTimerView == 3 )
        menuItemWidth *= 0.5;

    if( isScrolling )
        menuItemWidth -= scrollBarWidth;

    tColor ColorFg, ColorBg, ColorExtraTextFg;
    ColorExtraTextFg = Theme.Color(clrMenuItemExtraTextFont);
    if (Current) {
        ColorFg = Theme.Color(clrItemCurrentFont);
        ColorBg = Theme.Color(clrItemCurrentBg);
        ColorExtraTextFg = Theme.Color(clrMenuItemExtraTextCurrentFont);
    }
    else {
        if( Selectable ) {
            ColorFg = Theme.Color(clrItemSelableFont);
            ColorBg = Theme.Color(clrItemSelableBg);
        } else {
            ColorFg = Theme.Color(clrItemFont);
            ColorBg = Theme.Color(clrItemBg);
        }
    }

    menuPixmap->DrawRectangle(cRect(Config.decorBorderMenuItemSize, y, menuItemWidth, Height), ColorBg);
    cImage *img = NULL;
    int Left, Top;
    Left = Config.decorBorderMenuItemSize + marginItem;
    Top = y;

    int imageHeight = fontHeight;
    int imageLeft = Left;
    int imageTop = Top;

    cString TimerIconName("");
    if (!(Timer->HasFlags(tfActive))) {
        TimerIconName = "timerInactive";
        ColorFg = Theme.Color( clrMenuTimerItemDisabledFont );
    } else if (Timer->Recording()) {
        TimerIconName = "timerRecording";
        ColorFg = Theme.Color( clrMenuTimerItemRecordingFont );
    } else
        TimerIconName = "timerActive";

    img = imgLoader.LoadIcon(TimerIconName, imageHeight, imageHeight);
    if( img ) {
        imageTop = Top + (fontHeight - img->Height()) / 2;
        menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
        Left += imageHeight + marginItem * 2;
    }

    cString ws = cString::sprintf("%d", Channels.MaxNumber());
    int w = font->Width(ws);
    buffer = cString::sprintf("%d", Channel->Number());
    int Width = font->Width(buffer);
    if( Width < w )
        Width = w;
    menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, font, Width, fontHeight, taRight);
    Left += Width + marginItem;

    imageLeft = Left;

    int imageBGHeight = imageHeight;
    int imageBGWidth = imageHeight*1.34;

    cImage *imgBG = imgLoader.LoadIcon("logo_background", imageBGWidth, imageBGHeight);
    if( imgBG ) {
        imageBGHeight = imgBG->Height();
        imageBGWidth = imgBG->Width();
        imageTop = Top + (fontHeight - imgBG->Height()) / 2;
        menuIconsBGPixmap->DrawImage( cPoint(imageLeft, imageTop), *imgBG );
    }

    img = imgLoader.LoadLogo(Channel->Name(), imageBGWidth - 4, imageBGHeight - 4);
    if( img ) {
        imageTop = Top + (imageBGHeight - img->Height()) / 2;
        imageLeft = Left + (imageBGWidth - img->Width()) / 2;
        menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
    } else {
        bool isRadioChannel = ((!Channel->Vpid())&&(Channel->Apid(0))) ? true : false;

        if( isRadioChannel ) {
            img = imgLoader.LoadIcon("radio", imageBGWidth - 10, imageBGHeight - 10);
            if( img ) {
                imageTop = Top + (imageBGHeight - img->Height()) / 2;
                imageLeft = Left + (imageBGWidth - img->Width()) / 2;
                menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
            }
        } else if( Channel->GroupSep() ) {
            img = imgLoader.LoadIcon("changroup", imageBGWidth - 10, imageBGHeight - 10);
            if( img ) {
                imageTop = Top + (imageBGHeight - img->Height()) / 2;
                imageLeft = Left + (imageBGWidth - img->Width()) / 2;
                menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
            }
        } else {
            img = imgLoader.LoadIcon("tv", imageBGWidth - 10, imageBGHeight - 10);
            if( img ) {
                imageTop = Top + (imageBGHeight - img->Height()) / 2;
                imageLeft = Left + (imageBGWidth - img->Width()) / 2;
                menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
            }
        }
    }
    Left += imageBGWidth + marginItem * 2;

    cString day, name("");
    if (Timer->WeekDays())
        day = Timer->PrintDay(0, Timer->WeekDays(), false);
    else if (Timer->Day() - time(NULL) < 28 * SECSINDAY) {
        day = itoa(Timer->GetMDay(Timer->Day()));
        name = WeekDayName(Timer->Day());
    } else {
        struct tm tm_r;
        time_t Day = Timer->Day();
        localtime_r(&Day, &tm_r);
        char buffer[16];
        strftime(buffer, sizeof(buffer), "%Y%m%d", &tm_r);
        day = buffer;
    }
    const char *File = Setup.FoldersInTimerMenu ? NULL : strrchr(Timer->File(), FOLDERDELIMCHAR);
    if (File && strcmp(File + 1, TIMERMACRO_TITLE) && strcmp(File + 1, TIMERMACRO_EPISODE))
        File++;
    else
        File = Timer->File();

    if( Config.MenuTimerView == 1 ) {
        buffer = cString::sprintf("%s%s%s.", *name, *name && **name ? " " : "", *day);
        menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
        Left += font->Width("XXX 99.  ");
        buffer = cString::sprintf("%02d:%02d - %02d:%02d",Timer->Start() / 100, Timer->Start() % 100, Timer->Stop() / 100, Timer->Stop() % 100);
        menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
        Left += font->Width("99:99 - 99:99  ");
        if( Config.MenuItemParseTilde ) {
            std::string tilde = File;
            size_t found = tilde.find(" ~ ");
            size_t found2 = tilde.find("~");
            if( found != string::npos ) {
                std::string first = tilde.substr(0, found);
                std::string second = tilde.substr(found +2, tilde.length() );

                menuPixmap->DrawText(cPoint(Left, Top), first.c_str(), ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
                int l = font->Width( first.c_str() );
                menuPixmap->DrawText(cPoint(Left + l, Top), second.c_str(), ColorExtraTextFg, ColorBg, font, menuItemWidth - Left - l - marginItem);
            } else if ( found2 != string::npos ) {
                std::string first = tilde.substr(0, found2);
                std::string second = tilde.substr(found2 +1, tilde.length() );

                menuPixmap->DrawText(cPoint(Left, Top), first.c_str(), ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
                int l = font->Width( first.c_str() );
                l += font->Width("X");
                menuPixmap->DrawText(cPoint(Left + l, Top), second.c_str(), ColorExtraTextFg, ColorBg, font, menuItemWidth - Left - l - marginItem);
            } else
                menuPixmap->DrawText(cPoint(Left, Top), File, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
        } else {
            menuPixmap->DrawText(cPoint(Left, Top), File, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
        }
    } else if( Config.MenuTimerView == 2 || Config.MenuTimerView == 3 ) {
        buffer = cString::sprintf("%s%s%s.  %02d:%02d - %02d:%02d",
                    *name, *name && **name ? " " : "", *day,
                    Timer->Start() / 100, Timer->Start() % 100,
                    Timer->Stop() / 100, Timer->Stop() % 100);
        menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);

        if( Config.MenuItemParseTilde ) {
            std::string tilde = File;
            size_t found = tilde.find(" ~ ");
            size_t found2 = tilde.find("~");
            if( found != string::npos ) {
                std::string first = tilde.substr(0, found);
                std::string second = tilde.substr(found +2, tilde.length() );

                menuPixmap->DrawText(cPoint(Left, Top + fontHeight), first.c_str(), ColorFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);
                int l = fontSml->Width( first.c_str() );
                menuPixmap->DrawText(cPoint(Left + l, Top + fontHeight), second.c_str(), ColorExtraTextFg, ColorBg, fontSml, menuItemWidth - Left - l - marginItem);
            } else if ( found2 != string::npos ) {
                std::string first = tilde.substr(0, found2);
                std::string second = tilde.substr(found2 +1, tilde.length() );

                menuPixmap->DrawText(cPoint(Left, Top + fontHeight), first.c_str(), ColorFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);
                int l = fontSml->Width( first.c_str() );
                l += fontSml->Width("X");
                menuPixmap->DrawText(cPoint(Left + l, Top + fontHeight), second.c_str(), ColorExtraTextFg, ColorBg, fontSml, menuItemWidth - Left - l - marginItem);
            } else
                menuPixmap->DrawText(cPoint(Left, Top + fontHeight), File, ColorFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);
        } else {
            menuPixmap->DrawText(cPoint(Left, Top + fontHeight), File, ColorFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);
        }
    }

    sDecorBorder ib;
    ib.Left = Config.decorBorderMenuItemSize;
    ib.Top = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuItemSize + y;

    ib.Width = menuItemWidth - Config.decorBorderMenuItemSize*2;

    if( isScrolling ) {
        ib.Width -= scrollBarWidth;
    }

    ib.Width = menuItemWidth;

    ib.Height = Height;
    ib.Size = Config.decorBorderMenuItemSize;
    ib.Type = Config.decorBorderMenuItemType;

    if( Current ) {
        ib.ColorFg = Config.decorBorderMenuItemCurFg;
        ib.ColorBg = Config.decorBorderMenuItemCurBg;
    } else {
        if( Selectable ) {
            ib.ColorFg = Config.decorBorderMenuItemSelFg;
            ib.ColorBg = Config.decorBorderMenuItemSelBg;
        } else {
            ib.ColorFg = Config.decorBorderMenuItemFg;
            ib.ColorBg = Config.decorBorderMenuItemBg;
        }
    }

    DecorBorderDraw(ib.Left, ib.Top, ib.Width, ib.Height,
        ib.Size, ib.Type, ib.ColorFg, ib.ColorBg, BorderMenuItem);

    if( !isScrolling ) {
        ItemBorderInsertUnique(ib);
    }

    if( Config.MenuTimerView == 3 && Current ) {
        DrawItemExtraEvent(Event, tr("timer not enabled"));
    }

    return true;
}

bool cFlatDisplayMenu::SetItemEvent(const cEvent *Event, int Index, bool Current, bool Selectable, const cChannel *Channel, bool WithDate, eTimerMatch TimerMatch) {
    if( Config.MenuEventView == 0 )
        return false;

    cImage *img = NULL;
    cString buffer;
    int y = Index * itemEventHeight;

    int Height = fontHeight;
    if( Config.MenuEventView == 2 || Config.MenuEventView == 3 )
        Height = fontHeight + fontSmlHeight + marginItem*2 + Config.decorProgressMenuItemSize/2;

    menuItemWidth = menuWidth - Config.decorBorderMenuItemSize*2;
    if( Config.MenuEventView == 2 || Config.MenuEventView == 3 )
        menuItemWidth *= 0.6;

    if( isScrolling )
        menuItemWidth -= scrollBarWidth;

    tColor ColorFg, ColorBg, ColorExtraTextFg;
    ColorExtraTextFg = Theme.Color(clrMenuItemExtraTextFont);
    if (Current) {
        ColorFg = Theme.Color(clrItemCurrentFont);
        ColorBg = Theme.Color(clrItemCurrentBg);
        ColorExtraTextFg = Theme.Color(clrMenuItemExtraTextCurrentFont);
    }
    else {
        if( Selectable ) {
            ColorFg = Theme.Color(clrItemSelableFont);
            ColorBg = Theme.Color(clrItemSelableBg);
        } else {
            ColorFg = Theme.Color(clrItemFont);
            ColorBg = Theme.Color(clrItemBg);
        }
    }

    menuPixmap->DrawRectangle(cRect(Config.decorBorderMenuItemSize, y, menuItemWidth, Height), ColorBg);

    int Left = 0, Top = 0, LeftSecond = 0;
    LeftSecond = Left = Config.decorBorderMenuItemSize + marginItem;
    Top = y;
    int imageTop = Top;
    int w = 0;

    if( !Channel ) {
        TopBarSetMenuLogo( ItemEventLastChannelName );
    }

    if( Channel ) {
        ItemEventLastChannelName = Channel->Name();
        cString ws = cString::sprintf("%d", Channels.MaxNumber());
        w = font->Width(ws);
        if( !Channel->GroupSep() ) {
            buffer = cString::sprintf("%d", Channel->Number());
            int Width = font->Width(buffer);
            if( Width < w )
                Width = w;
            menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, font, Width, fontHeight, taRight);
        }
        Left += w + marginItem;

        int imageLeft = Left;
        int imageBGHeight = fontHeight;
        int imageBGWidth = fontHeight*1.34;
        cImage *imgBG = imgLoader.LoadIcon("logo_background", imageBGWidth, imageBGHeight);
        if( imgBG && !Channel->GroupSep() ) {
            imageBGHeight = imgBG->Height();
            imageBGWidth = imgBG->Width();
            imageTop = Top + (fontHeight - imgBG->Height()) / 2;
            menuIconsBGPixmap->DrawImage( cPoint(imageLeft, imageTop), *imgBG );
        }
        img = imgLoader.LoadLogo(Channel->Name(), imageBGWidth - 4, imageBGHeight - 4);
        if( img ) {
            imageTop = Top + (imageBGHeight - img->Height()) / 2;
            imageLeft = Left + (imageBGWidth - img->Width()) / 2;
            menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
        } else {
            bool isRadioChannel = ((!Channel->Vpid())&&(Channel->Apid(0))) ? true : false;

            if( isRadioChannel ) {
                img = imgLoader.LoadIcon("radio", imageBGWidth - 10, imageBGHeight - 10);
                if( img ) {
                    imageTop = Top + (imageBGHeight - img->Height()) / 2;
                    imageLeft = Left + (imageBGWidth - img->Width()) / 2;
                    menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
                }
            } else if( Channel->GroupSep() ) {
                img = imgLoader.LoadIcon("changroup", imageBGWidth - 10, imageBGHeight - 10);
                if( img ) {
                    imageTop = Top + (imageBGHeight - img->Height()) / 2;
                    imageLeft = Left + (imageBGWidth - img->Width()) / 2;
                    menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
                }
            } else {
                img = imgLoader.LoadIcon("tv", imageBGWidth - 10, imageBGHeight - 10);
                if( img ) {
                    imageTop = Top + (imageBGHeight - img->Height()) / 2;
                    imageLeft = Left + (imageBGWidth - img->Width()) / 2;
                    menuIconsPixmap->DrawImage( cPoint(imageLeft, imageTop), *img );
                }
            }
        }
        Left += imageBGWidth + marginItem * 2;
        LeftSecond = Left;

        cString channame;
        w = menuItemWidth / 10 * 2;
        if( Config.MenuEventView == 2 || Config.MenuEventView == 3 ) {
            channame = Channel->Name();
            w = font->Width(channame);
        } else
            channame = Channel->ShortName(true);

        if( Channel->GroupSep() ) {
            int lineTop = Top + (fontHeight - 3) / 2;
            menuPixmap->DrawRectangle(cRect( Left, lineTop, menuItemWidth - Left, 3), ColorFg);
            Left += w / 2;
            cString groupname = cString::sprintf(" %s ", *channame);
            menuPixmap->DrawText(cPoint(Left, Top), groupname, ColorFg, ColorBg, font, 0, 0, taCenter);
        } else
            menuPixmap->DrawText(cPoint(Left, Top), channame, ColorFg, ColorBg, font, w);

        Left += w + marginItem * 2;

        if( Event ) {
            int PBWidth = menuItemWidth/20;
            time_t now = time(NULL);

            if( (now >= (Event->StartTime() - 2*60) ) ) {
                int total = Event->EndTime() - Event->StartTime();
                if( total >= 0 ) {
                    // calculate progress bar
                    double progress = (int)roundf( (float)(time(NULL) - Event->StartTime()) / (float) (Event->Duration()) * 100.0);
                    if(progress < 0)
                        progress = 0.;
                    else if(progress > 100)
                        progress = 100;
                    int PBTop =  y + (itemEventHeight - Config.MenuItemPadding)/2 - Config.decorProgressMenuItemSize/2 - Config.decorBorderMenuItemSize;
                    int PBLeft = Left;
                    int PBHeight = Config.decorProgressMenuItemSize;


                    if( (Config.MenuEventView == 2 || Config.MenuEventView == 3) ) {
                        PBTop =  y + fontHeight + fontSmlHeight + marginItem;
                        PBWidth = menuItemWidth - LeftSecond - scrollBarWidth - marginItem * 2;
                        if( isScrolling )
                            PBWidth += scrollBarWidth;

                        PBLeft = LeftSecond;
                        PBHeight = Config.decorProgressMenuItemSize / 2;
                    }

                    if( Current )
                        ProgressBarDrawRaw(menuPixmap, menuPixmap, cRect( PBLeft, PBTop, PBWidth, PBHeight),
                            cRect( PBLeft, PBTop, PBWidth, PBHeight), progress, 100,
                            Config.decorProgressMenuItemCurFg, Config.decorProgressMenuItemCurBarFg, Config.decorProgressMenuItemCurBg, Config.decorProgressMenuItemType, false);
                    else
                        ProgressBarDrawRaw(menuPixmap, menuPixmap, cRect( PBLeft, PBTop, PBWidth, PBHeight),
                            cRect( PBLeft, PBTop, PBWidth, PBHeight), progress, 100,
                            Config.decorProgressMenuItemFg, Config.decorProgressMenuItemBarFg, Config.decorProgressMenuItemBg, Config.decorProgressMenuItemType, false);
                }
            }
            Left += PBWidth + marginItem*2;
        }
    }

    if( WithDate && Event && Selectable ) {
        if( (Config.MenuEventView == 2 || Config.MenuEventView == 3) && Channel )
            w = fontSml->Width("XXX 99. ") + marginItem;
        else
            w = font->Width("XXX 99. ") + marginItem;

        struct tm tm_r;
        time_t Day = Event->StartTime();
        localtime_r(&Day, &tm_r);
        char buf[8];
        strftime(buf, sizeof(buf), "%2d", &tm_r);

        cString DateString = cString::sprintf("%s %s. ", *WeekDayName( (time_t)Event->StartTime()), buf );
        if( (Config.MenuEventView == 2 || Config.MenuEventView == 3) && Channel ) {
            menuPixmap->DrawText(cPoint(LeftSecond, Top + fontHeight), DateString, ColorFg, ColorBg, fontSml, w);
            LeftSecond += w + marginItem;
        } else
            menuPixmap->DrawText(cPoint(Left, Top), DateString, ColorFg, ColorBg, font, w, fontHeight, taRight);

        Left += w + marginItem;
    }

    int imageHeight = fontHeight;
    if( (Config.MenuEventView == 2 || Config.MenuEventView == 3) && Channel && Event && Selectable ) {
        Top += fontHeight;
        Left = LeftSecond;
        imageHeight = fontSmlHeight;
        menuPixmap->DrawText(cPoint(Left, Top), Event->GetTimeString(), ColorFg, ColorBg, fontSml);
        Left += fontSml->Width( Event->GetTimeString() ) + marginItem;
    } else if( (Config.MenuEventView == 2 || Config.MenuEventView == 3) && Event && Selectable ){
        imageHeight = fontHeight;
        menuPixmap->DrawText(cPoint(Left, Top), Event->GetTimeString(), ColorFg, ColorBg, font);
        Left += font->Width( Event->GetTimeString() ) + marginItem;
    } else if( Event && Selectable ){
        menuPixmap->DrawText(cPoint(Left, Top), Event->GetTimeString(), ColorFg, ColorBg, font);
        Left += font->Width( Event->GetTimeString() ) + marginItem;
    }

    if( TimerMatch == tmFull ) {
        img = imgLoader.LoadIcon("timer_full", imageHeight, imageHeight);
        if( img ) {
            imageTop = Top;
            menuIconsPixmap->DrawImage( cPoint(Left, imageTop), *img );
        }
    } else if( TimerMatch == tmPartial ) {
        img = imgLoader.LoadIcon("timer_partial", imageHeight, imageHeight);
        if( img ) {
            imageTop = Top;
            menuIconsPixmap->DrawImage( cPoint(Left, imageTop), *img );
        }
    }
    Left += imageHeight + marginItem;
    if( Event && Selectable ) {
        if( Event->Vps() && (Event->Vps() - Event->StartTime()) ) {
            img = imgLoader.LoadIcon("vps", imageHeight, imageHeight);
            if( img ) {
                imageTop = Top;
                menuIconsPixmap->DrawImage( cPoint(Left, imageTop), *img );
            }
        }
        Left += imageHeight + marginItem;

        if( (Config.MenuEventView == 2 || Config.MenuEventView == 3) && Channel ) {
            menuPixmap->DrawText(cPoint(Left, Top), Event->Title(), ColorFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);
            if( Event->ShortText() ) {
                Left += fontSml->Width( Event->Title() );
                cString ShortText = cString::sprintf("  %s", Event->ShortText());
                menuPixmap->DrawText(cPoint(Left, Top), ShortText, ColorExtraTextFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);
            }
        } else if( (Config.MenuEventView == 2 || Config.MenuEventView == 3) ) {
            menuPixmap->DrawText(cPoint(Left, Top), Event->Title(), ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
            if( Event->ShortText() ) {
                Top += fontHeight;
                menuPixmap->DrawText(cPoint(Left, Top), Event->ShortText(), ColorExtraTextFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);
            }
        } else {
            menuPixmap->DrawText(cPoint(Left, Top), Event->Title(), ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
            if( Event->ShortText() ) {
                Left += font->Width(Event->Title());
                cString ShortText = cString::sprintf("  %s", Event->ShortText());
                menuPixmap->DrawText(cPoint(Left, Top), ShortText, ColorExtraTextFg, ColorBg, font, menuItemWidth - Left - marginItem);
            }
        }
    } else if( Event ) {
        try {
            // extract date from Separator
            std::string sep = Event->Title();
            if( sep.size() > 12 ) {
                std::size_t found = sep.find(" -");
                if( found >= 10 ) {
                    std::string date = sep.substr(found - 10, 10);
                    int lineTop = Top + (fontHeight - 3) / 2;
                    menuPixmap->DrawRectangle(cRect( 0, lineTop, menuItemWidth, 3), ColorFg);
                    cString datespace = cString::sprintf(" %s ", date.c_str());
                    menuPixmap->DrawText(cPoint(LeftSecond + menuWidth / 10 * 2, Top), datespace, ColorFg, ColorBg, font, 0, 0, taCenter);
                } else
                    menuPixmap->DrawText(cPoint(Left, Top), Event->Title(), ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
            } else
                menuPixmap->DrawText(cPoint(Left, Top), Event->Title(), ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
        }
        catch( ... ) {
            menuPixmap->DrawText(cPoint(Left, Top), Event->Title(), ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
        }
    }

    sDecorBorder ib;
    ib.Left = Config.decorBorderMenuItemSize;
    ib.Top = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuItemSize + y;

    ib.Width = menuItemWidth - Config.decorBorderMenuItemSize*2;

    if( isScrolling ) {
        ib.Width -= scrollBarWidth;
    }

    ib.Width = menuItemWidth;

    ib.Height = Height;
    ib.Size = Config.decorBorderMenuItemSize;
    ib.Type = Config.decorBorderMenuItemType;

    if( Current ) {
        ib.ColorFg = Config.decorBorderMenuItemCurFg;
        ib.ColorBg = Config.decorBorderMenuItemCurBg;
    } else {
        if( Selectable ) {
            ib.ColorFg = Config.decorBorderMenuItemSelFg;
            ib.ColorBg = Config.decorBorderMenuItemSelBg;
        } else {
            ib.ColorFg = Config.decorBorderMenuItemFg;
            ib.ColorBg = Config.decorBorderMenuItemBg;
        }
    }

    DecorBorderDraw(ib.Left, ib.Top, ib.Width, ib.Height,
        ib.Size, ib.Type, ib.ColorFg, ib.ColorBg, BorderMenuItem);

    if( !isScrolling ) {
        ItemBorderInsertUnique(ib);
    }

    if( Config.MenuEventView == 3  && Current ) {
        DrawItemExtraEvent(Event, "");
    }

    return true;
}

bool cFlatDisplayMenu::SetItemRecording(const cRecording *Recording, int Index, bool Current, bool Selectable, int Level, int Total, int New) {
    if( Config.MenuRecordingView == 0 )
        return false;

    cString buffer;
    cString RecName = GetRecordingName(Recording, Level, Total == 0);

    int y = Index * itemRecordingHeight;

    int Height = fontHeight;
    if( Config.MenuRecordingView == 2 || Config.MenuRecordingView == 3 )
        Height = fontHeight + fontSmlHeight + marginItem;

    menuItemWidth = menuWidth - Config.decorBorderMenuItemSize*2;
    if( Config.MenuRecordingView == 2 || Config.MenuRecordingView == 3 )
        menuItemWidth *= 0.5;

    if( isScrolling )
        menuItemWidth -= scrollBarWidth;

    tColor ColorFg, ColorBg, ColorExtraTextFg;
    ColorExtraTextFg = Theme.Color(clrMenuItemExtraTextFont);
    if (Current) {
        ColorFg = Theme.Color(clrItemCurrentFont);
        ColorBg = Theme.Color(clrItemCurrentBg);
        ColorExtraTextFg = Theme.Color(clrMenuItemExtraTextCurrentFont);
    }
    else {
        if( Selectable ) {
            ColorFg = Theme.Color(clrItemSelableFont);
            ColorBg = Theme.Color(clrItemSelableBg);
        } else {
            ColorFg = Theme.Color(clrItemFont);
            ColorBg = Theme.Color(clrItemBg);
        }
    }

    menuPixmap->DrawRectangle(cRect(Config.decorBorderMenuItemSize, y, menuItemWidth, Height), ColorBg);
    cImage *img = NULL;
    cImage *imgRecNew = imgLoader.LoadIcon("recording_new", fontHeight, fontHeight);
    cImage *imgRecNewSml = imgLoader.LoadIcon("recording_new", fontSmlHeight, fontSmlHeight);
    cImage *imgRecCut = imgLoader.LoadIcon("recording_cutted", fontHeight, fontHeight);

    int Left, Top;
    Left = Config.decorBorderMenuItemSize + marginItem;
    Top = y;

    if( Config.MenuRecordingView == 1 ) {
        int LeftWidth = Left + fontHeight + imgRecNew->Width() + imgRecCut->Width() +
            marginItem * 3 + font->Width("99.99.99  99:99  99:99 ");

        if( Total == 0 ) {
            img = imgLoader.LoadIcon("recording", fontHeight, fontHeight);
            if( img ) {
                menuIconsPixmap->DrawImage( cPoint(Left, Top), *img );
                Left += fontHeight + marginItem;
            }

            //int Minutes = max(0, (Recording->LengthInSeconds() + 30) / 60);
            int Minutes = (Recording->LengthInSeconds() + 30) / 60;
            cString Length = cString::sprintf("%02d:%02d", Minutes / 60, Minutes % 60);
            buffer = cString::sprintf("%s  %s  %s ", *ShortDateString(Recording->Start()), *TimeString(Recording->Start()), *Length);

            menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);

            Left += font->Width( buffer );
            if( Recording->IsNew() ) {
                if( imgRecNew ) {
                    menuIconsPixmap->DrawImage( cPoint(Left, Top), *imgRecNew );
                }
            }
            Left += imgRecNew->Width() + marginItem;
            if (Recording->IsEdited()) {
                if( imgRecCut ) {
                    menuIconsPixmap->DrawImage( cPoint(Left, Top), *imgRecCut );
                }
            }
            Left += imgRecCut->Width() + marginItem;

            menuPixmap->DrawText(cPoint(Left, Top), RecName, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
        } else if( Total > 0 ) {
            img = imgLoader.LoadIcon("folder", fontHeight, fontHeight);
            if( img ) {
                menuIconsPixmap->DrawImage( cPoint(Left, Top), *img );
                Left += img->Width() + marginItem;
            }

            buffer = cString::sprintf("%d  ", Total);
            menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, font, font->Width("  999"), fontHeight, taLeft);
            Left += font->Width("  999 ");

            if( imgRecNew )
                menuIconsPixmap->DrawImage( cPoint(Left, Top), *imgRecNew );
            Left += imgRecNew->Width() + marginItem;
            buffer = cString::sprintf("%d", New);
            menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
            Left += font->Width(" 999 ");
            if( Config.MenuItemRecordingShowFolderDate != 0 ) {
                buffer = cString::sprintf("  (%s)", *ShortDateString(GetLastRecTimeFromFolder(Recording, Level)));
                menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorExtraTextFg, ColorBg, font, menuItemWidth - Left - marginItem);
            }

            menuPixmap->DrawText(cPoint(LeftWidth, Top), RecName, ColorFg, ColorBg, font, menuItemWidth - LeftWidth - marginItem);
            LeftWidth += font->Width(RecName) + marginItem*2;
        } else if( Total == -1 ) {
            img = imgLoader.LoadIcon("folder", fontHeight, fontHeight);
            if( img ) {
                menuIconsPixmap->DrawImage( cPoint(Left, Top), *img );
                Left += img->Width() + marginItem;
            }

            menuPixmap->DrawText(cPoint(Left, Top), Recording->FileName(), ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
        }
    } else {
        if( Total == 0 ) {
            img = imgLoader.LoadIcon("recording", fontHeight, fontHeight);
            if( img ) {
                menuIconsPixmap->DrawImage( cPoint(Left, Top), *img );
                Left += fontHeight + marginItem;
            }
            int ImagesWidth = imgRecNew->Width() + imgRecCut->Width() + marginItem*2 + scrollBarWidth;
            if( isScrolling )
                ImagesWidth -= scrollBarWidth;

            menuPixmap->DrawText(cPoint(Left, Top), RecName, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem - ImagesWidth);
            Top += fontHeight;

            int Minutes = (Recording->LengthInSeconds() + 30) / 60;
            cString Length = cString::sprintf("%02d:%02d", Minutes / 60, Minutes % 60);
            buffer = cString::sprintf("%s  %s  %s ", *ShortDateString(Recording->Start()), *TimeString(Recording->Start()), *Length);

            menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);

            Top -= fontHeight;
            Left = menuItemWidth - ImagesWidth;
            if( Recording->IsNew() ) {
                if( imgRecNew ) {
                    menuIconsPixmap->DrawImage( cPoint(Left, Top), *imgRecNew );
                }
            }
            Left += imgRecNew->Width() + marginItem;
            if (Recording->IsEdited()) {
                if( imgRecCut ) {
                    menuIconsPixmap->DrawImage( cPoint(Left, Top), *imgRecCut );
                }
            }
            Left += imgRecCut->Width() + marginItem;

        } else if( Total > 0 ) {
            img = imgLoader.LoadIcon("folder", fontHeight, fontHeight);
            if( img ) {
                menuIconsPixmap->DrawImage( cPoint(Left, Top), *img );
                Left += img->Width() + marginItem;
            }
            menuPixmap->DrawText(cPoint(Left, Top), RecName, ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);

            Top += fontHeight;
            buffer = cString::sprintf("  %d", Total);
            menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, fontSml, fontSml->Width("  999"), fontSmlHeight, taRight);
            Left += fontSml->Width("  999 ");

            if( imgRecNewSml )
                menuIconsPixmap->DrawImage( cPoint(Left, Top), *imgRecNewSml );
            Left += imgRecNewSml->Width() + marginItem;
            buffer = cString::sprintf("%d", New);
            menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);
            Left += fontSml->Width(" 999 ");

            if( Config.MenuItemRecordingShowFolderDate != 0 ) {
                buffer = cString::sprintf("  (%s)", *ShortDateString(GetLastRecTimeFromFolder(Recording, Level)));
                menuPixmap->DrawText(cPoint(Left, Top), buffer, ColorExtraTextFg, ColorBg, fontSml, menuItemWidth - Left - marginItem);
            }
        } else if( Total == -1 ) {
            img = imgLoader.LoadIcon("folder", fontHeight, fontHeight);
            if( img ) {
                menuIconsPixmap->DrawImage( cPoint(Left, Top), *img );
                Left += img->Width() + marginItem;
            }
            menuPixmap->DrawText(cPoint(Left, Top), Recording->FileName(), ColorFg, ColorBg, font, menuItemWidth - Left - marginItem);
        }
    }

    sDecorBorder ib;
    ib.Left = Config.decorBorderMenuItemSize;
    ib.Top = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuItemSize + y;

    ib.Width = menuItemWidth - Config.decorBorderMenuItemSize*2;

    if( isScrolling ) {
        ib.Width -= scrollBarWidth;
    }

    ib.Width = menuItemWidth;

    ib.Height = Height;
    ib.Size = Config.decorBorderMenuItemSize;
    ib.Type = Config.decorBorderMenuItemType;

    if( Current ) {
        ib.ColorFg = Config.decorBorderMenuItemCurFg;
        ib.ColorBg = Config.decorBorderMenuItemCurBg;
    } else {
        if( Selectable ) {
            ib.ColorFg = Config.decorBorderMenuItemSelFg;
            ib.ColorBg = Config.decorBorderMenuItemSelBg;
        } else {
            ib.ColorFg = Config.decorBorderMenuItemFg;
            ib.ColorBg = Config.decorBorderMenuItemBg;
        }
    }

    DecorBorderDraw(ib.Left, ib.Top, ib.Width, ib.Height,
        ib.Size, ib.Type, ib.ColorFg, ib.ColorBg, BorderMenuItem);

    if( !isScrolling ) {
        ItemBorderInsertUnique(ib);
    }

    if( Config.MenuRecordingView == 3 && Current ) {
        DrawItemExtraRecording(Recording, tr("no recording info"));
    }

    return true;
}

void cFlatDisplayMenu::SetEvent(const cEvent *Event) {
    if( !Event )
        return;

    #ifdef DEBUGEPGTIME
        uint32_t tick0 = GetMsTicks();
    #endif

    ShowEvent = true;
    ShowRecording = false;
    ShowText = false;
    ItemBorderClear();

    cLeft = Config.decorBorderMenuContentSize;
    cTop = chTop + marginItem*3 + fontHeight + fontSmlHeight*2 +
        Config.decorBorderMenuContentSize + Config.decorBorderMenuContentHeadSize;
    cWidth = menuWidth - Config.decorBorderMenuContentSize*2;
    cHeight = osdHeight - (topBarHeight + Config.decorBorderTopBarSize*2 +
        buttonsHeight + Config.decorBorderButtonSize*2 + marginItem*3 +
        chHeight + Config.decorBorderMenuContentHeadSize*2 + Config.decorBorderMenuContentSize*2);

    if( !ButtonsDrawn() )
        cHeight += buttonsHeight + Config.decorBorderButtonSize*2;

    menuItemWidth = cWidth;

    contentHeadPixmap->Fill(clrTransparent);
    contentHeadPixmap->DrawRectangle(cRect(0, 0, menuWidth, fontHeight + fontSmlHeight*2 + marginItem*2), Theme.Color(clrScrollbarBg));

    cString date = Event->GetDateString();
    cString startTime = Event->GetTimeString();
    cString endTime = Event->GetEndTimeString();

    cString timeString = cString::sprintf("%s %s - %s", *date, *startTime, *endTime);

    cString title = Event->Title();
    cString shortText = Event->ShortText();

    contentHeadPixmap->DrawText(cPoint(marginItem, marginItem), timeString, Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), fontSml, menuWidth - marginItem*2);
    contentHeadPixmap->DrawText(cPoint(marginItem, marginItem + fontSmlHeight), title, Theme.Color(clrMenuEventFontTitle), Theme.Color(clrMenuEventBg), font, menuWidth - marginItem*2);
    contentHeadPixmap->DrawText(cPoint(marginItem, marginItem + fontSmlHeight + fontHeight), shortText, Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), fontSml, menuWidth - marginItem*2);

    DecorBorderDraw(chLeft, chTop, chWidth, chHeight, Config.decorBorderMenuContentHeadSize, Config.decorBorderMenuContentHeadType,
        Config.decorBorderMenuContentHeadFg, Config.decorBorderMenuContentHeadBg);

    // Description
    ostringstream text, textAdditional;
    if( !isempty(Event->Description()) ) {
        text << Event->Description();
    }

    if( Config.EpgAdditionalInfoShow ) {
        const cComponents *Components = Event->Components();
        if (Components) {
            ostringstream audio;
            bool firstAudio = true;
            const char *audio_type = NULL;
            ostringstream subtitle;
            bool firstSubtitle = true;
            for (int i = 0; i < Components->NumComponents(); i++) {
                const tComponent *p = Components->Component(i);
                switch (p->stream) {
                    case sc_video_MPEG2:
                        if (p->description)
                            textAdditional << tr("Video") << ": " <<  p->description << " (MPEG2)";
                        else
                            textAdditional << tr("Video") << ": MPEG2";
                        break;
                    case sc_video_H264_AVC:
                        if (p->description)
                            textAdditional << tr("Video") << ": " <<  p->description << " (H.264)";
                        else
                            textAdditional << tr("Video") << ": H.264";
                        break;

                    case sc_audio_MP2:
                    case sc_audio_AC3:
                    case sc_audio_HEAAC:
                        if (firstAudio)
                            firstAudio = false;
                        else
                            audio << ", ";
                        switch (p->stream) {
                            case sc_audio_MP2:
                                // workaround for wrongfully used stream type X 02 05 for AC3
                                if (p->type == 5)
                                    audio_type = "AC3";
                                else
                                    audio_type = "MP2";
                                break;
                            case sc_audio_AC3:
                                audio_type = "AC3"; break;
                            case sc_audio_HEAAC:
                                audio_type = "HEAAC"; break;
                        }
                        if (p->description)
                            audio << p->description << " (" << audio_type << ", " << p->language << ")";
                        else
                            audio << p->language << " (" << audio_type << ")";
                        break;
                    case sc_subtitle:
                        if (firstSubtitle)
                            firstSubtitle = false;
                        else
                            subtitle << ", ";
                        if (p->description)
                            subtitle << p->description << " (" << p->language << ")";
                        else
                            subtitle << p->language;
                        break;
                }
            }
            if (audio.str().length() > 0) {
                if( textAdditional.str().length() > 0 )
                    textAdditional << endl;
                textAdditional << tr("Audio") << ": "<< audio.str();
            } if (subtitle.str().length() > 0) {
                if( textAdditional.str().length() > 0 )
                    textAdditional << endl;
                textAdditional << endl << tr("Subtitle") << ": "<< subtitle.str();
            }
        }
    }

    #ifdef DEBUGEPGTIME
        uint32_t tick1 = GetMsTicks();
        dsyslog("SetEvent info-text time: %d ms", tick1 - tick0);
    #endif

    std::ostringstream sstrReruns;
    if( Config.EpgRerunsShow ) {
        // lent from nopacity
        cPlugin *epgSearchPlugin = cPluginManager::GetPlugin("epgsearch");
        if (epgSearchPlugin && !isempty(Event->Title())) {
            Epgsearch_searchresults_v1_0 data;
            std::string strQuery = Event->Title();
            data.useSubTitle = false;

            data.query = (char *)strQuery.c_str();
            data.mode = 0;
            data.channelNr = 0;
            data.useTitle = true;
            data.useDescription = false;

            if (epgSearchPlugin->Service("Epgsearch-searchresults-v1.0", &data)) {
                cList<Epgsearch_searchresults_v1_0::cServiceSearchResult>* list = data.pResultList;
                if (list && (list->Count() > 1)) {
                    int i = 0;
                    for (Epgsearch_searchresults_v1_0::cServiceSearchResult *r = list->First(); r && i < 5; r = list->Next(r)) {
                        if ((Event->ChannelID() == r->event->ChannelID()) && (Event->StartTime() == r->event->StartTime()))
                            continue;
                        i++;
                        sstrReruns << *DayDateTime(r->event->StartTime());
                        cChannel *channel = Channels.GetByChannelID(r->event->ChannelID(), true, true);
                        if (channel) {
                            sstrReruns << ", " << channel->Number() << " -";
                            sstrReruns << " " << channel->ShortName(true);
                        }
                        sstrReruns << ":  " << r->event->Title();
                        //if (!isempty(r->event->ShortText()))
                        //    sstrReruns << "~" << r->event->ShortText();
                        sstrReruns << std::endl;
                    }
                    delete list;
                }
            }
        }
    }
    #ifdef DEBUGEPGTIME
        uint32_t tick2 = GetMsTicks();
        dsyslog("SetEvent reruns time: %d ms", tick2 - tick1);
    #endif

    bool Scrollable = false;
    bool FirstRun = true;

    do {
        if( Scrollable ) {
            FirstRun = false;
            cWidth -= scrollBarWidth;
        }

        ComplexContent.Clear();
        ComplexContent.SetOsd(osd);
        ComplexContent.SetPosition(cRect(cLeft, cTop, cWidth, cHeight));
        ComplexContent.SetBGColor(Theme.Color(clrMenuRecBg));
        ComplexContent.SetScrollSize(fontHeight);
        ComplexContent.SetScrollingActive(true);

        int ContentTop = marginItem;

        ostringstream series_info, movie_info;

        std::vector<std::string> actors_path;
        std::vector<std::string> actors_name;
        std::vector<std::string> actors_role;

        std::string mediaPath;
        int mediaWidth = 0;
        int mediaHeight = 0;

        #ifdef DEBUGEPGTIME
            uint32_t tick3 = GetMsTicks();
        #endif

        // first try scraper2vdr
        static cPlugin *pScraper = cPluginManager::GetPlugin("scraper2vdr");
        if( !pScraper ) // if it doesn't exit, try tvscraper
            pScraper = cPluginManager::GetPlugin("tvscraper");
        if( (Config.TVScraperEPGInfoShowPoster || Config.TVScraperEPGInfoShowActors) && pScraper ) {
            ScraperGetEventType call;
            call.event = Event;
            int seriesId = 0;
            int episodeId = 0;
            int movieId = 0;

            if (pScraper->Service("GetEventType", &call)) {
                seriesId = call.seriesId;
                episodeId = call.episodeId;
                movieId = call.movieId;
            }
            if( seriesId > 0 ) {
                cSeries series;
                series.seriesId = seriesId;
                series.episodeId = episodeId;
                if (pScraper->Service("GetSeries", &series)) {
                    if( series.banners.size() > 0 )
                        mediaPath = series.banners[0].path;
                    mediaWidth = cWidth/2 - marginItem*2;
                    mediaHeight = cHeight - marginItem*2 - fontHeight - 6;
                    if( Config.TVScraperEPGInfoShowActors ) {
                        for( unsigned int i = 0; i < series.actors.size(); i++ ) {
                            if( imgLoader.FileExits(series.actors[i].actorThumb.path) ) {
                                actors_path.push_back(series.actors[i].actorThumb.path);
                                actors_name.push_back(series.actors[i].name);
                                actors_role.push_back(series.actors[i].role);
                            }
                        }
                    }
                    if( series.name.length() > 0 )
                        series_info << tr("name: ") << series.name << endl;
                    if( series.firstAired.length() > 0 )
                        series_info << tr("first aired: ") << series.firstAired << endl;
                    if( series.network.length() > 0 )
                        series_info << tr("network: ") << series.network << endl;
                    if( series.genre.length() > 0 )
                        series_info << tr("genre: ") << series.genre << endl;
                    if( series.rating > 0 )
                        series_info << tr("rating: ") << series.rating << endl;
                    if( series.status.length() > 0 )
                        series_info << tr("status: ") << series.status << endl;
                    if( series.episode.season > 0 )
                        series_info << tr("season number: ") << series.episode.season << endl;
                    if( series.episode.number > 0 )
                        series_info << tr("episode number: ") << series.episode.number << endl;
                }
            } else if (movieId > 0) {
                cMovie movie;
                movie.movieId = movieId;
                if (pScraper->Service("GetMovie", &movie)) {
                    mediaPath = movie.poster.path;
                    mediaWidth = cWidth/2 - marginItem*3;
                    mediaHeight = cHeight - marginItem*2 - fontHeight - 6;
                    if( Config.TVScraperEPGInfoShowActors ) {
                        for( unsigned int i = 0; i < movie.actors.size(); i++ ) {
                            if( imgLoader.FileExits(movie.actors[i].actorThumb.path) ) {
                                actors_path.push_back(movie.actors[i].actorThumb.path);
                                actors_name.push_back(movie.actors[i].name);
                                actors_role.push_back(movie.actors[i].role);
                            }
                        }
                    }
                    if( movie.title.length() > 0 )
                        movie_info << tr("title: ") << movie.title << endl;
                    if( movie.originalTitle.length() > 0 )
                        movie_info << tr("original title: ") << movie.originalTitle << endl;
                    if( movie.collectionName.length() > 0 )
                        movie_info << tr("collection name: ") << movie.collectionName << endl;
                    if( movie.genres.length() > 0 )
                        movie_info << tr("genre: ") << movie.genres << endl;
                    if( movie.releaseDate.length() > 0 )
                        movie_info << tr("release date: ") << movie.releaseDate << endl;
                    if( movie.popularity > 0 )
                        movie_info << tr("popularity: ") << movie.popularity << endl;
                    if( movie.voteAverage > 0 )
                        movie_info << tr("vote average: ") << movie.voteAverage << endl;
                }
            }
        }

        #ifdef DEBUGEPGTIME
            uint32_t tick4 = GetMsTicks();
            dsyslog("SetEvent tvscraper time: %d ms", tick4 - tick3);
        #endif

        if( mediaPath.length() > 0 ) {
            cImage *img = imgLoader.LoadFile(mediaPath.c_str(), mediaWidth, mediaHeight);
            if( img ) {
                ComplexContent.AddText(tr("Description"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuEventFontTitle), Theme.Color(clrMenuEventBg), font);
                ContentTop += fontHeight;
                ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuEventTitleLine));
                ContentTop += 6;
                ComplexContent.AddImageWithFloatedText(img, CIP_Right, text.str().c_str(), cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                    Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), font);
            } else if( text.str().length() > 0 ) {
                ComplexContent.AddText(tr("Description"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuEventFontTitle), Theme.Color(clrMenuEventBg), font);
                ContentTop += fontHeight;
                ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuEventTitleLine));
                ContentTop += 6;
                ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                    Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), font);
            }
        } else if( text.str().length() > 0 ) {
            ComplexContent.AddText(tr("Description"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuEventFontTitle), Theme.Color(clrMenuEventBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuEventTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), font);
        }

        if( movie_info.str().length() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Movie information"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuEventFontTitle), Theme.Color(clrMenuEventBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuEventTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(movie_info.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), font);
        }

        if( series_info.str().length() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Series information"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuEventFontTitle), Theme.Color(clrMenuEventBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuEventTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(series_info.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), font);
        }
        #ifdef DEBUGEPGTIME
            uint32_t tick5 = GetMsTicks();
            dsyslog("SetEvent epg-text time: %d ms", tick5 - tick4);
        #endif

        if( Config.TVScraperEPGInfoShowActors && actors_path.size() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Actors"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuEventFontTitle), Theme.Color(clrMenuEventBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuEventTitleLine));
            ContentTop += 6;

            int actorsPerLine = 6;
            int numActors = actors_path.size();
            int actorWidth = cWidth / actorsPerLine - marginItem*4;
            int picsPerLine = (cWidth - marginItem*2) / actorWidth;
            int picLines = numActors / picsPerLine;
            if( numActors%picsPerLine != 0 )
                picLines++;
            int actorMargin = ((cWidth - marginItem*2) - actorWidth*actorsPerLine) / (actorsPerLine-1);
            int x = marginItem;
            int y = ContentTop;
            int actor = 0;
            for (int row = 0; row < picLines; row++) {
                for (int col = 0; col < picsPerLine; col++) {
                    if (actor == numActors)
                        break;
                    std::string path = actors_path[actor];
                    cImage *img = imgLoader.LoadFile(path.c_str(), actorWidth, 999);
                    if( img ) {
                        ComplexContent.AddImage(img, cRect(x, y, 0, 0));
                        std::string name = actors_name[actor];
                        std::stringstream sstrRole;
                        if( actors_role[actor].length() > 0 )
                            sstrRole << "\"" << actors_role[actor] << "\"";
                        std::string role = sstrRole.str();
                        ComplexContent.AddText(name.c_str(), false, cRect(x, y + img->Height() + marginItem, actorWidth, 0), Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), fontSml, actorWidth, fontSmlHeight, taCenter);
                        ComplexContent.AddText(role.c_str(), false, cRect(x, y + img->Height() + marginItem + fontSmlHeight, actorWidth, 0), Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), fontSml, actorWidth, fontSmlHeight, taCenter);
                    }
                    x += actorWidth + actorMargin;
                    actor++;
                }
                x = marginItem;
                y = ComplexContent.BottomContent() + fontHeight;
            }
        }
        #ifdef DEBUGEPGTIME
            uint32_t tick6 = GetMsTicks();
            dsyslog("SetEvent actor time: %d ms", tick6 - tick5);
        #endif

        if( sstrReruns.str().length() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Reruns"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuEventFontTitle), Theme.Color(clrMenuEventBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuEventTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(sstrReruns.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), font);
        }

        if( textAdditional.str().length() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Video information"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuEventFontTitle), Theme.Color(clrMenuEventBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuEventTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(textAdditional.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuEventFontInfo), Theme.Color(clrMenuEventBg), font);
        }
        Scrollable = ComplexContent.Scrollable(cHeight - marginItem*2);
    } while( Scrollable && FirstRun );

    if( Config.MenuContentFullSize || Scrollable ) {
        ComplexContent.CreatePixmaps(true);
    } else
        ComplexContent.CreatePixmaps(false);

    ComplexContent.Draw();

    if( Scrollable )
        DrawScrollbar(ComplexContent.ScrollTotal(), ComplexContent.ScrollOffset(), ComplexContent.ScrollShown(), ComplexContent.Top() - scrollBarTop, ComplexContent.Height(), ComplexContent.ScrollOffset() > 0, ComplexContent.ScrollOffset() + ComplexContent.ScrollShown() < ComplexContent.ScrollTotal(), true);

    if( Config.MenuContentFullSize || Scrollable )
        DecorBorderDraw(cLeft, cTop, cWidth, ComplexContent.ContentHeight(true), Config.decorBorderMenuContentSize, Config.decorBorderMenuContentType,
            Config.decorBorderMenuContentFg, Config.decorBorderMenuContentBg);
    else
        DecorBorderDraw(cLeft, cTop, cWidth, ComplexContent.ContentHeight(false), Config.decorBorderMenuContentSize, Config.decorBorderMenuContentType,
            Config.decorBorderMenuContentFg, Config.decorBorderMenuContentBg);

    #ifdef DEBUGEPGTIME
        uint32_t tick7 = GetMsTicks();
        dsyslog("SetEvent total time: %d ms", tick7 - tick0);
        //dsyslog("SetEvent time: %d", tick7);
    #endif
}

void cFlatDisplayMenu::DrawItemExtraRecording(const cRecording *Recording, cString EmptyText) {
    cLeft = menuItemWidth + Config.decorBorderMenuItemSize*2 + Config.decorBorderMenuContentSize + marginItem;
    if( isScrolling )
        cLeft += scrollBarWidth;
    cTop = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuContentSize;
    cWidth = menuWidth - cLeft - Config.decorBorderMenuContentSize;
    cHeight = osdHeight - (topBarHeight + Config.decorBorderTopBarSize*2 +
        buttonsHeight + Config.decorBorderButtonSize*2 + marginItem*3 + Config.decorBorderMenuContentSize*2);

    ostringstream text;
    if( Recording ) {
        const cRecordingInfo *recInfo = Recording->Info();
        text.imbue(std::locale(""));

        if (!isempty(recInfo->Description()))
            text << recInfo->Description() << endl << endl;

        // lent from skinelchi
        if( Config.RecordingAdditionalInfoShow ) {
            cChannel *channel = Channels.GetByChannelID(((cRecordingInfo *)recInfo)->ChannelID());
            if (channel)
                text << trVDR("Channel") << ": " << channel->Number() << " - " << channel->Name() << endl;

            cMarks marks;
            bool hasMarks = marks.Load(Recording->FileName(), Recording->FramesPerSecond(), Recording->IsPesRecording()) && marks.Count();
            cIndexFile *index = new cIndexFile(Recording->FileName(), false, Recording->IsPesRecording());

            int lastIndex = 0;

            int cuttedLength = 0;
            long cutinframe = 0;
            unsigned long long recsize = 0;
            unsigned long long recsizecutted = 0;
            unsigned long long cutinoffset = 0;
            unsigned long long filesize[100000];
            filesize[0] = 0;

            int i = 0;
            int imax = 999;
            struct stat filebuf;
            cString filename;
            int rc = 0;

            do {
                if (Recording->IsPesRecording())
                    filename = cString::sprintf("%s/%03d.vdr", Recording->FileName(), ++i);
                else {
                    filename = cString::sprintf("%s/%05d.ts", Recording->FileName(), ++i);
                    imax = 99999;
                }
                rc=stat(filename, &filebuf);
                if (rc == 0)
                    filesize[i] = filesize[i-1] + filebuf.st_size;
                else {
                    if (ENOENT != errno) {
                        esyslog ("skinflatplus: error determining file size of \"%s\" %d (%s)", (const char *)filename, errno, strerror(errno));
                        recsize = 0;
                    }
                }
            } while( i <= imax && !rc );
            recsize = filesize[i-1];

            if (hasMarks && index) {
                uint16_t FileNumber;
                off_t FileOffset;

                bool cutin = true;
                cMark *mark = marks.First();
                while (mark) {
                    long position = mark->Position();
                    index->Get(position, &FileNumber, &FileOffset);
                    if (cutin) {
                        cutinframe = position;
                        cutin = false;
                        cutinoffset = filesize[FileNumber-1] + FileOffset;
                    } else {
                        cuttedLength += position - cutinframe;
                        cutin = true;
                        recsizecutted += filesize[FileNumber-1] + FileOffset - cutinoffset;
                    }
                    cMark *nextmark = marks.Next(mark);
                    mark = nextmark;
                }
                if( !cutin ) {
                    cuttedLength += index->Last() - cutinframe;
                    index->Get(index->Last() - 1, &FileNumber, &FileOffset);
                    recsizecutted += filesize[FileNumber-1] + FileOffset - cutinoffset;
                }
            }
            if (index) {
                lastIndex = index->Last();
                text << tr("Length") << ": " << *IndexToHMSF(lastIndex, false, Recording->FramesPerSecond());
                if (hasMarks)
                    text << " (" << tr("cutted") << ": " << *IndexToHMSF(cuttedLength, false, Recording->FramesPerSecond()) << ")";
                text << endl;
            }
            delete index;

            if (recsize > MEGABYTE(1023))
                text << tr("Size") << ": " << fixed << setprecision(2) << (float)recsize / MEGABYTE(1024) << " GB";
            else
                text << tr("Size") << ": " << recsize / MEGABYTE(1) << " MB";
            if( hasMarks )
                if (recsize > MEGABYTE(1023))
                    text << " (" <<  tr("cutted") << ": " << fixed << setprecision(2) <<  (float)recsizecutted/MEGABYTE(1024) << " GB)";
                else
                    text << " (" << tr("cutted") << ": " <<  recsizecutted/MEGABYTE(1) << " MB)";

            text << endl << trVDR("Priority") << ": " << Recording->Priority() << ", " << trVDR("Lifetime") << ": " << Recording->Lifetime() << endl;

            if( lastIndex ) {
                text << tr("format") << ": " << (Recording->IsPesRecording() ? "PES" : "TS") << ", " << tr("bit rate") << ": ~ " << fixed << setprecision (2) << (float)recsize/lastIndex*Recording->FramesPerSecond()*8/MEGABYTE(1) << " MBit/s (Video + Audio)";
            }
            const cComponents *Components = recInfo->Components();
            if( Components ) {
                ostringstream audio;
                bool firstAudio = true;
                const char *audio_type = NULL;
                ostringstream subtitle;
                bool firstSubtitle = true;
                for (int i = 0; i < Components->NumComponents(); i++) {
                    const tComponent *p = Components->Component(i);

                    switch (p->stream) {
                        case sc_video_MPEG2:
                            text << endl << tr("Video") << ": " <<  p->description << " (MPEG2)";
                            break;
                        case sc_video_H264_AVC:
                            text << endl << tr("Video") << ": " <<  p->description << " (H.264)";
                            break;
                        case sc_audio_MP2:
                        case sc_audio_AC3:
                        case sc_audio_HEAAC:
                            if (firstAudio)
                                firstAudio = false;
                            else
                                audio << ", ";
                            switch (p->stream) {
                                case sc_audio_MP2:
                                    // workaround for wrongfully used stream type X 02 05 for AC3
                                    if (p->type == 5)
                                        audio_type = "AC3";
                                    else
                                        audio_type = "MP2";
                                    break;
                                case sc_audio_AC3:
                                    audio_type = "AC3"; break;
                                case sc_audio_HEAAC:
                                    audio_type = "HEAAC"; break;
                            }
                            if (p->description)
                                audio << p->description << " (" << audio_type << ", " << p->language << ")";
                            else
                                audio << p->language << " (" << audio_type << ")";
                            break;
                        case sc_subtitle:
                            if (firstSubtitle)
                                firstSubtitle = false;
                            else
                                subtitle << ", ";
                            if (p->description)
                                subtitle << p->description << " (" << p->language << ")";
                            else
                                subtitle << p->language;
                            break;
                    }
                }
                if (audio.str().length() > 0)
                    text << endl << tr("Audio") << ": "<< audio.str();
                if (subtitle.str().length() > 0)
                    text << endl << tr("Subtitle") << ": "<< subtitle.str();
            }
            if (recInfo->Aux()) {
                string str_epgsearch = xml_substring(recInfo->Aux(), "<epgsearch>", "</epgsearch>");
                string channel, searchtimer, pattern;

                if (!str_epgsearch.empty()) {
                    channel = xml_substring(str_epgsearch, "<channel>", "</channel>");
                    searchtimer = xml_substring(str_epgsearch, "<searchtimer>", "</searchtimer>");
                    if (searchtimer.empty())
                        searchtimer = xml_substring(str_epgsearch, "<Search timer>", "</Search timer>");
                }

                string str_vdradmin = xml_substring(recInfo->Aux(), "<vdradmin-am>", "</vdradmin-am>");
                if (!str_vdradmin.empty()) {
                    pattern = xml_substring(str_vdradmin, "<pattern>", "</pattern>");
                }

                if ((!channel.empty() && !searchtimer.empty()) || !pattern.empty())  {
                    text << endl << endl << tr("additional information") << ":" << endl;
                    if (!channel.empty() && !searchtimer.empty()) {
                        text << "EPGsearch: " << tr("channel") << ": " << channel << ", " << tr("search pattern") << ": " << searchtimer;
                    }
                    if (!pattern.empty()) {
                        text << "VDRadmin-AM: " << tr("search pattern") << ": " << pattern;
                    }
                }
            }
        }
    } else
        text << *EmptyText;

    ComplexContent.Clear();
    ComplexContent.SetScrollSize(fontSmlHeight);
    ComplexContent.SetScrollingActive(false);
    ComplexContent.SetOsd(osd);
    ComplexContent.SetPosition(cRect(cLeft, cTop, cWidth, cHeight));
    ComplexContent.SetBGColor(Theme.Color(clrMenuRecBg));

    std::string mediaPath;
    int mediaWidth = 0;
    int mediaHeight = 0;
    int mediaType = 0;

    // first try scraper2vdr
    static cPlugin *pScraper = cPluginManager::GetPlugin("scraper2vdr");
    if( !pScraper ) // if it doesn't exit, try tvscraper
        pScraper = cPluginManager::GetPlugin("tvscraper");
    if( Config.TVScraperRecInfoShowPoster && pScraper ) {
        ScraperGetEventType call;
        call.recording = Recording;
        int seriesId = 0;
        int episodeId = 0;
        int movieId = 0;

        if (pScraper->Service("GetEventType", &call)) {
            seriesId = call.seriesId;
            episodeId = call.episodeId;
            movieId = call.movieId;
        }
        if( seriesId > 0 ) {
            cSeries series;
            series.seriesId = seriesId;
            series.episodeId = episodeId;
            if (pScraper->Service("GetSeries", &series)) {
                if( series.banners.size() > 0 )
                    mediaPath = series.banners[0].path;
                mediaWidth = cWidth - marginItem*2;
                mediaHeight = 999;
                mediaType = 1;
            }
        } else if (movieId > 0) {
            cMovie movie;
            movie.movieId = movieId;
            if (pScraper->Service("GetMovie", &movie)) {
                mediaPath = movie.poster.path;
                mediaWidth = cWidth/2 - marginItem*3;
                mediaHeight = 999;
                mediaType = 2;
            }
        }
    }

    cString recPath = cString::sprintf("%s", Recording->FileName());
    cString recImage;
    if( imgLoader.SearchRecordingPoster(recPath, recImage) ) {
        mediaWidth = cWidth/2 - marginItem*3;
        mediaHeight = 999;
        mediaType = 2;
        mediaPath = recImage;
    }

    if( mediaPath.length() > 0 ) {
        cImage *img = imgLoader.LoadFile(mediaPath.c_str(), mediaWidth, mediaHeight);
        if( img && mediaType == 2 ) {
            ComplexContent.AddImageWithFloatedText(img, CIP_Right, text.str().c_str(), cRect(marginItem, marginItem, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), fontSml);
        } else if( img && mediaType == 1 ) {
            ComplexContent.AddImage(img, cRect(marginItem, marginItem, img->Width(), img->Height()) );
            ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, marginItem + img->Height(), cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), fontSml);
        } else {
            ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, marginItem, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), fontSml);
        }
    } else {
        ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, marginItem, cWidth - marginItem*2, cHeight - marginItem*2),
            Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), fontSml);
    }

    ComplexContent.CreatePixmaps(Config.MenuContentFullSize);
    ComplexContent.Draw();

    DecorBorderClearByFrom(BorderContent);
    if( Config.MenuContentFullSize )
        DecorBorderDraw(cLeft, cTop, cWidth, ComplexContent.ContentHeight(true), Config.decorBorderMenuContentSize, Config.decorBorderMenuContentType,
            Config.decorBorderMenuContentFg, Config.decorBorderMenuContentBg, BorderContent);
    else
        DecorBorderDraw(cLeft, cTop, cWidth, ComplexContent.ContentHeight(false), Config.decorBorderMenuContentSize, Config.decorBorderMenuContentType,
            Config.decorBorderMenuContentFg, Config.decorBorderMenuContentBg, BorderContent);
}

void cFlatDisplayMenu::SetRecording(const cRecording *Recording) {
    if( !Recording )
        return;

    #ifdef DEBUGEPGTIME
        uint32_t tick0 = GetMsTicks();
    #endif

    ShowEvent = false;
    ShowRecording = true;
    ShowText = false;
    ItemBorderClear();

    const cRecordingInfo *recInfo = Recording->Info();

    chLeft = Config.decorBorderMenuContentHeadSize;
    chTop = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuContentHeadSize;
    chWidth = menuWidth - Config.decorBorderMenuContentHeadSize*2;
    chHeight = fontHeight + fontSmlHeight*2 + marginItem*2;
    contentHeadPixmap = osd->CreatePixmap(1, cRect(chLeft, chTop, chWidth, chHeight));

    cLeft = Config.decorBorderMenuContentSize;
    cTop = chTop + marginItem*3 + fontHeight + fontSmlHeight*2 +
        Config.decorBorderMenuContentSize + Config.decorBorderMenuContentHeadSize;
    cWidth = menuWidth - Config.decorBorderMenuContentSize*2;
    cHeight = osdHeight - (topBarHeight + Config.decorBorderTopBarSize*2 +
        buttonsHeight + Config.decorBorderButtonSize*2 + marginItem*3 +
        chHeight + Config.decorBorderMenuContentHeadSize*2 + Config.decorBorderMenuContentSize*2);

    if( !ButtonsDrawn() )
        cHeight += buttonsHeight + Config.decorBorderButtonSize*2;

    menuItemWidth = cWidth;

    ostringstream text, textAdditional, recAdditional;
    text.imbue(std::locale(""));
    textAdditional.imbue(std::locale(""));
    recAdditional.imbue(std::locale(""));

    if (!isempty(recInfo->Description()))
        text << recInfo->Description() << endl << endl;

    // lent from skinelchi
    if( Config.RecordingAdditionalInfoShow ) {
        cChannel *channel = Channels.GetByChannelID(((cRecordingInfo *)recInfo)->ChannelID());
        if (channel)
            recAdditional << trVDR("Channel") << ": " << channel->Number() << " - " << channel->Name() << endl;

        cMarks marks;
        bool hasMarks = marks.Load(Recording->FileName(), Recording->FramesPerSecond(), Recording->IsPesRecording()) && marks.Count();
        cIndexFile *index = new cIndexFile(Recording->FileName(), false, Recording->IsPesRecording());

        int lastIndex = 0;

        int cuttedLength = 0;
        long cutinframe = 0;
        unsigned long long recsize = 0;
        unsigned long long recsizecutted = 0;
        unsigned long long cutinoffset = 0;
        unsigned long long filesize[100000];
        filesize[0] = 0;

        int i = 0;
        int imax = 999;
        struct stat filebuf;
        cString filename;
        int rc = 0;

        do {
            if (Recording->IsPesRecording())
                filename = cString::sprintf("%s/%03d.vdr", Recording->FileName(), ++i);
            else {
                filename = cString::sprintf("%s/%05d.ts", Recording->FileName(), ++i);
                imax = 99999;
            }
            rc=stat(filename, &filebuf);
            if (rc == 0)
                filesize[i] = filesize[i-1] + filebuf.st_size;
            else {
                if (ENOENT != errno) {
                    esyslog ("skinflatplus: error determining file size of \"%s\" %d (%s)", (const char *)filename, errno, strerror(errno));
                    recsize = 0;
                }
            }
        } while( i <= imax && !rc );
        recsize = filesize[i-1];

        if (hasMarks && index) {
            uint16_t FileNumber;
            off_t FileOffset;

            bool cutin = true;
            cMark *mark = marks.First();
            while (mark) {
                long position = mark->Position();
                index->Get(position, &FileNumber, &FileOffset);
                if (cutin) {
                    cutinframe = position;
                    cutin = false;
                    cutinoffset = filesize[FileNumber-1] + FileOffset;
                } else {
                    cuttedLength += position - cutinframe;
                    cutin = true;
                    recsizecutted += filesize[FileNumber-1] + FileOffset - cutinoffset;
                }
                cMark *nextmark = marks.Next(mark);
                mark = nextmark;
            }
            if( !cutin ) {
                cuttedLength += index->Last() - cutinframe;
                index->Get(index->Last() - 1, &FileNumber, &FileOffset);
                recsizecutted += filesize[FileNumber-1] + FileOffset - cutinoffset;
            }
        }
        if (index) {
            lastIndex = index->Last();
            recAdditional << tr("Length") << ": " << *IndexToHMSF(lastIndex, false, Recording->FramesPerSecond());
            if (hasMarks)
                recAdditional << " (" << tr("cutted") << ": " << *IndexToHMSF(cuttedLength, false, Recording->FramesPerSecond()) << ")";
            recAdditional << endl;
        }
        delete index;

        if (recsize > MEGABYTE(1023))
            recAdditional << tr("Size") << ": " << fixed << setprecision(2) << (float)recsize / MEGABYTE(1024) << " GB";
        else
            recAdditional << tr("Size") << ": " << recsize / MEGABYTE(1) << " MB";
        if( hasMarks )
            if (recsize > MEGABYTE(1023))
                recAdditional << " (" <<  tr("cutted") << ": " << fixed << setprecision(2) <<  (float)recsizecutted/MEGABYTE(1024) << " GB)";
            else
                recAdditional << " (" << tr("cutted") << ": " <<  recsizecutted/MEGABYTE(1) << " MB)";

        recAdditional << endl << trVDR("Priority") << ": " << Recording->Priority() << ", " << trVDR("Lifetime") << ": " << Recording->Lifetime() << endl;

        if( lastIndex ) {
            recAdditional << tr("format") << ": " << (Recording->IsPesRecording() ? "PES" : "TS") << ", " << tr("bit rate") << ": ~ " << fixed << setprecision (2) << (float)recsize/lastIndex*Recording->FramesPerSecond()*8/MEGABYTE(1) << " MBit/s (Video + Audio)";
        }
        const cComponents *Components = recInfo->Components();
        if( Components ) {
            ostringstream audio;
            bool firstAudio = true;
            const char *audio_type = NULL;
            ostringstream subtitle;
            bool firstSubtitle = true;
            for (int i = 0; i < Components->NumComponents(); i++) {
                const tComponent *p = Components->Component(i);

                switch (p->stream) {
                    case sc_video_MPEG2:
                        textAdditional << tr("Video") << ": " <<  p->description << " (MPEG2)";
                        break;
                    case sc_video_H264_AVC:
                        textAdditional << tr("Video") << ": " <<  p->description << " (H.264)";
                        break;
                    case sc_audio_MP2:
                    case sc_audio_AC3:
                    case sc_audio_HEAAC:
                        if (firstAudio)
                            firstAudio = false;
                        else
                            audio << ", ";
                        switch (p->stream) {
                            case sc_audio_MP2:
                                // workaround for wrongfully used stream type X 02 05 for AC3
                                if (p->type == 5)
                                    audio_type = "AC3";
                                else
                                    audio_type = "MP2";
                                break;
                            case sc_audio_AC3:
                                audio_type = "AC3"; break;
                            case sc_audio_HEAAC:
                                audio_type = "HEAAC"; break;
                        }
                        if (p->description)
                            audio << p->description << " (" << audio_type << ", " << p->language << ")";
                        else
                            audio << p->language << " (" << audio_type << ")";
                        break;
                    case sc_subtitle:
                        if (firstSubtitle)
                            firstSubtitle = false;
                        else
                            subtitle << ", ";
                        if (p->description)
                            subtitle << p->description << " (" << p->language << ")";
                        else
                            subtitle << p->language;
                        break;
                }
            }
            if (audio.str().length() > 0) {
                if( textAdditional.str().length() > 0 )
                    textAdditional << endl;
                textAdditional << tr("Audio") << ": "<< audio.str();
            } if (subtitle.str().length() > 0) {
                if( textAdditional.str().length() > 0 )
                    textAdditional << endl;
                textAdditional << tr("Subtitle") << ": "<< subtitle.str();
            }
        }
        if (recInfo->Aux()) {
            string str_epgsearch = xml_substring(recInfo->Aux(), "<epgsearch>", "</epgsearch>");
            string channel, searchtimer, pattern;

            if (!str_epgsearch.empty()) {
                channel = xml_substring(str_epgsearch, "<channel>", "</channel>");
                searchtimer = xml_substring(str_epgsearch, "<searchtimer>", "</searchtimer>");
                if (searchtimer.empty())
                    searchtimer = xml_substring(str_epgsearch, "<Search timer>", "</Search timer>");
            }

            string str_vdradmin = xml_substring(recInfo->Aux(), "<vdradmin-am>", "</vdradmin-am>");
            if (!str_vdradmin.empty()) {
                pattern = xml_substring(str_vdradmin, "<pattern>", "</pattern>");
            }

            if ((!channel.empty() && !searchtimer.empty()) || !pattern.empty())  {
                recAdditional << endl;
                if (!channel.empty() && !searchtimer.empty()) {
                    recAdditional << "EPGsearch: " << tr("channel") << ": " << channel << ", " << tr("search pattern") << ": " << searchtimer;
                }
                if (!pattern.empty()) {
                    recAdditional << "VDRadmin-AM: " << tr("search pattern") << ": " << pattern;
                }
            }
        }
    }

    #ifdef DEBUGEPGTIME
        uint32_t tick1 = GetMsTicks();
        dsyslog("SetRecording info-text time: %d ms", tick1 - tick0);
    #endif

    bool Scrollable = false;
    bool FirstRun = true;

    do {
        if( Scrollable ) {
            FirstRun = false;
            cWidth -= scrollBarWidth;
        }

        ComplexContent.Clear();
        ComplexContent.SetOsd(osd);
        ComplexContent.SetPosition(cRect(cLeft, cTop, cWidth, cHeight));
        ComplexContent.SetBGColor(Theme.Color(clrMenuRecBg));
        ComplexContent.SetScrollSize(fontHeight);
        ComplexContent.SetScrollingActive(true);

        int ContentTop = marginItem;

        ostringstream series_info, movie_info;

        std::vector<std::string> actors_path;
        std::vector<std::string> actors_name;
        std::vector<std::string> actors_role;

        std::string mediaPath;
        int mediaWidth = 0;
        int mediaHeight = 0;

        #ifdef DEBUGEPGTIME
            uint32_t tick2 = GetMsTicks();
        #endif

        // first try scraper2vdr
        static cPlugin *pScraper = cPluginManager::GetPlugin("scraper2vdr");
        if( !pScraper ) // if it doesn't exit, try tvscraper
            pScraper = cPluginManager::GetPlugin("tvscraper");
        if( (Config.TVScraperRecInfoShowPoster || Config.TVScraperRecInfoShowActors) && pScraper ) {
            ScraperGetEventType call;
            call.recording = Recording;
            int seriesId = 0;
            int episodeId = 0;
            int movieId = 0;

            if (pScraper->Service("GetEventType", &call)) {
                seriesId = call.seriesId;
                episodeId = call.episodeId;
                movieId = call.movieId;
            }
            if( seriesId > 0 ) {
                cSeries series;
                series.seriesId = seriesId;
                series.episodeId = episodeId;
                if (pScraper->Service("GetSeries", &series)) {
                    if( series.banners.size() > 0 )
                        mediaPath = series.banners[0].path;
                    mediaWidth = cWidth/2 - marginItem*2;
                    mediaHeight = cHeight - marginItem*2 - fontHeight - 6;
                    if( Config.TVScraperRecInfoShowActors ) {
                        for( unsigned int i = 0; i < series.actors.size(); i++ ) {
                            if( imgLoader.FileExits( series.actors[i].actorThumb.path ) ) {
                                actors_path.push_back(series.actors[i].actorThumb.path);
                                actors_name.push_back(series.actors[i].name);
                                actors_role.push_back(series.actors[i].role);
                            }
                        }
                    }
                    if( series.name.length() > 0 )
                        series_info << tr("name: ") << series.name << endl;
                    if( series.firstAired.length() > 0 )
                        series_info << tr("first aired: ") << series.firstAired << endl;
                    if( series.network.length() > 0 )
                        series_info << tr("network: ") << series.network << endl;
                    if( series.genre.length() > 0 )
                        series_info << tr("genre: ") << series.genre << endl;
                    if( series.rating > 0 )
                        series_info << tr("rating: ") << series.rating << endl;
                    if( series.status.length() > 0 )
                        series_info << tr("status: ") << series.status << endl;
                    if( series.episode.season > 0 )
                        series_info << tr("season number: ") << series.episode.season << endl;
                    if( series.episode.number > 0 )
                        series_info << tr("episode number: ") << series.episode.number << endl;
                }
            } else if (movieId > 0) {
                cMovie movie;
                movie.movieId = movieId;
                if (pScraper->Service("GetMovie", &movie)) {
                    mediaPath = movie.poster.path;
                    mediaWidth = cWidth/2 - marginItem*3;
                    mediaHeight = cHeight - marginItem*2 - fontHeight - 6;
                    if( Config.TVScraperRecInfoShowActors ) {
                        for( unsigned int i = 0; i < movie.actors.size(); i++ ) {
                            if( imgLoader.FileExits( movie.actors[i].actorThumb.path ) ) {
                                actors_path.push_back(movie.actors[i].actorThumb.path);
                                actors_name.push_back(movie.actors[i].name);
                                actors_role.push_back(movie.actors[i].role);
                            }
                        }
                    }
                    if( movie.title.length() > 0 )
                        movie_info << tr("title: ") << movie.title << endl;
                    if( movie.originalTitle.length() > 0 )
                        movie_info << tr("original title: ") << movie.originalTitle << endl;
                    if( movie.collectionName.length() > 0 )
                        movie_info << tr("collection name: ") << movie.collectionName << endl;
                    if( movie.genres.length() > 0 )
                        movie_info << tr("genre: ") << movie.genres << endl;
                    if( movie.releaseDate.length() > 0 )
                        movie_info << tr("release date: ") << movie.releaseDate << endl;
                    if( movie.popularity > 0 )
                        movie_info << tr("popularity: ") << movie.popularity << endl;
                    if( movie.voteAverage > 0 )
                        movie_info << tr("vote average: ") << movie.voteAverage << endl;
                }
            }
        }

        #ifdef DEBUGEPGTIME
            uint32_t tick3 = GetMsTicks();
            dsyslog("SetRecording tvscraper time: %d ms", tick3 - tick2);
        #endif

        cString recPath = cString::sprintf("%s", Recording->FileName());
        cString recImage;
        if( imgLoader.SearchRecordingPoster(recPath, recImage) ) {
            mediaWidth = cWidth/2 - marginItem*2;
            mediaHeight = cHeight - marginItem*2 - fontHeight - 6;
            mediaPath = recImage;
        }
        if( mediaPath.length() > 0 ) {
            cImage *img = imgLoader.LoadFile(mediaPath.c_str(), mediaWidth, mediaHeight);
            if( img ) {
                ComplexContent.AddText(tr("Description"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuRecFontTitle), Theme.Color(clrMenuRecBg), font);
                ContentTop += fontHeight;
                ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuRecTitleLine));
                ContentTop += 6;
                ComplexContent.AddImageWithFloatedText(img, CIP_Right, text.str().c_str(), cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                    Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), font);
            } else if( text.str().length() > 0 ) {
                ComplexContent.AddText(tr("Description"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuRecFontTitle), Theme.Color(clrMenuRecBg), font);
                ContentTop += fontHeight;
                ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuRecTitleLine));
                ContentTop += 6;
                ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                    Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), font);
            }
        } else if( text.str().length() > 0 ) {
            ComplexContent.AddText(tr("Description"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuRecFontTitle), Theme.Color(clrMenuRecBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuRecTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(text.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), font);
        }

        if( movie_info.str().length() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Movie information"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuRecFontTitle), Theme.Color(clrMenuRecBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuRecTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(movie_info.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), font);
        }

        if( series_info.str().length() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Series information"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuRecFontTitle), Theme.Color(clrMenuRecBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuRecTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(series_info.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), font);
        }
        #ifdef DEBUGEPGTIME
            uint32_t tick4 = GetMsTicks();
            dsyslog("SetRecording epg-text time: %d ms", tick4 - tick3);
        #endif

        if( Config.TVScraperRecInfoShowActors && actors_path.size() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Actors"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuRecFontTitle), Theme.Color(clrMenuRecBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuRecTitleLine));
            ContentTop += 6;

            int actorsPerLine = 6;
            int numActors = actors_path.size();
            int actorWidth = cWidth / actorsPerLine - marginItem*4;
            int picsPerLine = (cWidth - marginItem*2) / actorWidth;
            int picLines = numActors / picsPerLine;
            if( numActors%picsPerLine != 0 )
                picLines++;
            int actorMargin = ((cWidth - marginItem*2) - actorWidth*actorsPerLine) / (actorsPerLine-1);
            int x = marginItem;
            int y = ContentTop;
            int actor = 0;
            for (int row = 0; row < picLines; row++) {
                for (int col = 0; col < picsPerLine; col++) {
                    if (actor == numActors)
                        break;
                    std::string path = actors_path[actor];
                    cImage *img = imgLoader.LoadFile(path.c_str(), actorWidth, 999);
                    if( img ) {
                        ComplexContent.AddImage(img, cRect(x, y, 0, 0));
                        std::string name = actors_name[actor];
                        std::stringstream sstrRole;
                        sstrRole << "\"" << actors_role[actor] << "\"";
                        std::string role = sstrRole.str();
                        ComplexContent.AddText(name.c_str(), false, cRect(x, y + img->Height() + marginItem, actorWidth, 0), Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), fontSml, actorWidth, fontSmlHeight, taCenter);
                        ComplexContent.AddText(role.c_str(), false, cRect(x, y + img->Height() + marginItem + fontSmlHeight, actorWidth, 0), Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), fontSml, actorWidth, fontSmlHeight, taCenter);
                        }
                    x += actorWidth + actorMargin;
                    actor++;
                }
                x = marginItem;
                y = ComplexContent.BottomContent() + fontHeight;
            }
        }
        #ifdef DEBUGEPGTIME
            uint32_t tick5 = GetMsTicks();
            dsyslog("SetRecording actor time: %d ms", tick5 - tick4);
        #endif

        if( recAdditional.str().length() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Recording information"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuRecFontTitle), Theme.Color(clrMenuRecBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuRecTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(recAdditional.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), font);
        }

        if( textAdditional.str().length() > 0 ) {
            ContentTop = ComplexContent.BottomContent() + fontHeight;
            ComplexContent.AddText(tr("Video information"), false, cRect(marginItem*10, ContentTop, 0, 0), Theme.Color(clrMenuRecFontTitle), Theme.Color(clrMenuRecBg), font);
            ContentTop += fontHeight;
            ComplexContent.AddRect(cRect(0, ContentTop, cWidth, 3), Theme.Color(clrMenuRecTitleLine));
            ContentTop += 6;
            ComplexContent.AddText(textAdditional.str().c_str(), true, cRect(marginItem, ContentTop, cWidth - marginItem*2, cHeight - marginItem*2),
                Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), font);
        }

        Scrollable = ComplexContent.Scrollable(cHeight - marginItem*2);
    } while( Scrollable && FirstRun );

    if( Config.MenuContentFullSize || Scrollable ) {
        ComplexContent.CreatePixmaps(true);
    } else
        ComplexContent.CreatePixmaps(false);

    ComplexContent.Draw();

    contentHeadPixmap->Fill(clrTransparent);
    contentHeadPixmap->DrawRectangle(cRect(0, 0, menuWidth, fontHeight + fontSmlHeight*2 + marginItem*2), Theme.Color(clrScrollbarBg));

    cString timeString = cString::sprintf("%s  %s  %s", *DateString(Recording->Start()), *TimeString(Recording->Start()), recInfo->ChannelName() ? recInfo->ChannelName() : "");

    cString title = recInfo->Title();
    if( isempty(title) )
        title = Recording->Name();
    cString shortText = recInfo->ShortText();

    contentHeadPixmap->DrawText(cPoint(marginItem, marginItem), timeString, Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), fontSml, menuWidth - marginItem*2);
    contentHeadPixmap->DrawText(cPoint(marginItem, marginItem + fontSmlHeight), title, Theme.Color(clrMenuRecFontTitle), Theme.Color(clrMenuRecBg), font, menuWidth - marginItem*2);
    contentHeadPixmap->DrawText(cPoint(marginItem, marginItem + fontSmlHeight + fontHeight), shortText, Theme.Color(clrMenuRecFontInfo), Theme.Color(clrMenuRecBg), fontSml, menuWidth - marginItem*2);

    DecorBorderDraw(chLeft, chTop, chWidth, chHeight, Config.decorBorderMenuContentHeadSize, Config.decorBorderMenuContentHeadType,
        Config.decorBorderMenuContentHeadFg, Config.decorBorderMenuContentHeadBg, BorderSetRecording, false);

    if( Scrollable )
        DrawScrollbar(ComplexContent.ScrollTotal(), ComplexContent.ScrollOffset(), ComplexContent.ScrollShown(), ComplexContent.Top() - scrollBarTop, ComplexContent.Height(), ComplexContent.ScrollOffset() > 0, ComplexContent.ScrollOffset() + ComplexContent.ScrollShown() < ComplexContent.ScrollTotal(), true);

    RecordingBorder.Left = cLeft;
    RecordingBorder.Top = cTop;
    RecordingBorder.Width = cWidth;
    RecordingBorder.Height = ComplexContent.Height();
    RecordingBorder.Size = Config.decorBorderMenuContentSize;
    RecordingBorder.Type = Config.decorBorderMenuContentType;
    RecordingBorder.ColorFg = Config.decorBorderMenuContentFg;
    RecordingBorder.ColorBg = Config.decorBorderMenuContentBg;
    RecordingBorder.From = BorderMenuRecord;

    if( Config.MenuContentFullSize || Scrollable )
        DecorBorderDraw(RecordingBorder.Left, RecordingBorder.Top, RecordingBorder.Width, ComplexContent.ContentHeight(true),
            RecordingBorder.Size, RecordingBorder.Type,
            RecordingBorder.ColorFg, RecordingBorder.ColorBg, RecordingBorder.From, false);
    else
        DecorBorderDraw(RecordingBorder.Left, RecordingBorder.Top, RecordingBorder.Width, ComplexContent.ContentHeight(false),
            RecordingBorder.Size, RecordingBorder.Type,
            RecordingBorder.ColorFg, RecordingBorder.ColorBg, RecordingBorder.From, false);

    #ifdef DEBUGEPGTIME
        uint32_t tick6 = GetMsTicks();
        dsyslog("SetRecording total time: %d ms", tick6 - tick0);
    #endif
}

void cFlatDisplayMenu::SetText(const char *Text, bool FixedFont) {
    if( !Text )
        return;

    ShowEvent = false;
    ShowRecording = false;
    ShowText = true;
    ItemBorderClear();

    contentHeadPixmap->Fill(clrTransparent);

    int Left = Config.decorBorderMenuContentSize;
    int Top = topBarHeight + marginItem + Config.decorBorderTopBarSize*2 + Config.decorBorderMenuContentSize;
    int Width = menuWidth - Config.decorBorderMenuContentSize*2;
    int Height = osdHeight - (topBarHeight + Config.decorBorderTopBarSize*2 +
        buttonsHeight + Config.decorBorderButtonSize*2 + Config.decorBorderMenuContentSize*2 + marginItem);

    if( !ButtonsDrawn() )
        Height += buttonsHeight + Config.decorBorderButtonSize*2;

    ComplexContent.Clear();

    menuItemWidth = Width;
    bool Scrollable = false;
    if( FixedFont ) {
        ComplexContent.AddText(Text, true, cRect(marginItem, marginItem, Width - marginItem*2, Height - marginItem*2),
            Theme.Color(clrMenuTextFixedFont), Theme.Color(clrMenuTextBg), fontFixed);
        ComplexContent.SetScrollSize(fontFixedHeight);
    } else {
        ComplexContent.AddText(Text, true, cRect(marginItem, marginItem, Width - marginItem*2, Height - marginItem*2),
            Theme.Color(clrMenuTextFixedFont), Theme.Color(clrMenuTextBg), font);
        ComplexContent.SetScrollSize(fontHeight);
    }

    Scrollable = ComplexContent.Scrollable(Height - marginItem*2);
    if( Scrollable ) {
        Width -= scrollBarWidth;
        ComplexContent.Clear();
        if( FixedFont ) {
            ComplexContent.AddText(Text, true, cRect(marginItem, marginItem, Width - marginItem*2, Height - marginItem*2),
                Theme.Color(clrMenuTextFixedFont), Theme.Color(clrMenuTextBg), fontFixed);
            ComplexContent.SetScrollSize(fontFixedHeight);
        } else {
            ComplexContent.AddText(Text, true, cRect(marginItem, marginItem, Width - marginItem*2, Height - marginItem*2),
                Theme.Color(clrMenuTextFixedFont), Theme.Color(clrMenuTextBg), font);
            ComplexContent.SetScrollSize(fontHeight);
        }
    }

    ComplexContent.SetOsd(osd);
    ComplexContent.SetPosition(cRect(Left, Top, Width, Height));
    ComplexContent.SetBGColor(Theme.Color(clrMenuTextBg));
    ComplexContent.SetScrollingActive(true);

    if( Config.MenuContentFullSize || Scrollable ) {
        ComplexContent.CreatePixmaps(true);
    } else
        ComplexContent.CreatePixmaps(false);

    ComplexContent.Draw();

    if( Scrollable )
        DrawScrollbar(ComplexContent.ScrollTotal(), ComplexContent.ScrollOffset(), ComplexContent.ScrollShown(), ComplexContent.Top() - scrollBarTop, ComplexContent.Height(), ComplexContent.ScrollOffset() > 0, ComplexContent.ScrollOffset() + ComplexContent.ScrollShown() < ComplexContent.ScrollTotal(), true);

    if( Config.MenuContentFullSize || Scrollable )
        DecorBorderDraw(Left, Top, Width, ComplexContent.ContentHeight(true), Config.decorBorderMenuContentSize, Config.decorBorderMenuContentType,
            Config.decorBorderMenuContentFg, Config.decorBorderMenuContentBg);
    else
        DecorBorderDraw(Left, Top, Width, ComplexContent.ContentHeight(false), Config.decorBorderMenuContentSize, Config.decorBorderMenuContentType,
            Config.decorBorderMenuContentFg, Config.decorBorderMenuContentBg);
}

int cFlatDisplayMenu::GetTextAreaWidth(void) const {
    return menuWidth - (marginItem*2);
}

const cFont *cFlatDisplayMenu::GetTextAreaFont(bool FixedFont) const {
    const cFont *rfont = FixedFont ? fontFixed : font;
    return rfont;
}

void cFlatDisplayMenu::Flush(void) {
    TopBarUpdate();
    osd->Flush();
}

void cFlatDisplayMenu::ItemBorderInsertUnique(sDecorBorder ib) {
    std::list<sDecorBorder>::iterator it;
    for( it = ItemsBorder.begin(); it != ItemsBorder.end(); it++ ) {
        if( (*it).Left == ib.Left && (*it).Top == ib.Top ) {
            (*it).Left = ib.Left;
            (*it).Top = ib.Top;
            (*it).Width = ib.Width;
            (*it).Height = ib.Height;
            (*it).Size = ib.Size;
            (*it).Type = ib.Type;
            (*it).ColorFg = ib.ColorFg;
            (*it).ColorBg = ib.ColorBg;
            (*it).From = ib.From;
            return;
        }
    }

    ItemsBorder.push_back(ib);
}

void cFlatDisplayMenu::ItemBorderDrawAllWithScrollbar(void) {
    std::list<sDecorBorder>::iterator it;
    for( it = ItemsBorder.begin(); it != ItemsBorder.end(); it++ ) {
        DecorBorderDraw((*it).Left, (*it).Top, (*it).Width - scrollBarWidth, (*it).Height, (*it).Size, (*it).Type,
            (*it).ColorFg, (*it).ColorBg, BorderMenuItem);
    }
}

void cFlatDisplayMenu::ItemBorderDrawAllWithoutScrollbar(void) {
    std::list<sDecorBorder>::iterator it;
    for( it = ItemsBorder.begin(); it != ItemsBorder.end(); it++ ) {
        DecorBorderDraw((*it).Left, (*it).Top, (*it).Width + scrollBarWidth, (*it).Height, (*it).Size, (*it).Type,
            (*it).ColorFg, (*it).ColorBg, BorderMenuItem);
    }
}

void cFlatDisplayMenu::ItemBorderClear(void) {
    ItemsBorder.clear();
}

time_t cFlatDisplayMenu::GetLastRecTimeFromFolder(const cRecording *Recording, int Level) {
    std::string RecFolder = GetRecordingName(Recording, Level, true);
    time_t RecStart = Recording->Start();

    for(cRecording *rec = Recordings.First(); rec; rec = Recordings.Next(rec)) {
        std::string RecFolder2 = GetRecordingName(rec, Level, true);
        if( RecFolder == RecFolder2 ) { // recordings must be in the same folder
            time_t RecStart2 = rec->Start();
            if( Config.MenuItemRecordingShowFolderDate == 1) { // newest
                if( RecStart2 > RecStart )
                    RecStart = RecStart2;
            } else if( Config.MenuItemRecordingShowFolderDate == 2 ) // oldest
                if( RecStart2 < RecStart )
                    RecStart = RecStart2;
        }
    }

    return RecStart;
}

// returns the string between start and end or an empty string if not found
string cFlatDisplayMenu::xml_substring(string source, const char* str_start, const char* str_end) {
    size_t start = source.find(str_start);
    size_t end   = source.find(str_end);

    if (string::npos != start && string::npos != end) {
        return (source.substr(start + strlen(str_start), end - start - strlen(str_start)));
    }

    return string();
}

const char * cFlatDisplayMenu::GetRecordingName(const cRecording *Recording, int Level, bool isFolder) {
    if( !Recording )
        return "";
    std::string recNamePart;
    std::string recName = Recording->Name();
    try {
        std::vector<std::string> tokens;
        std::istringstream f(recName.c_str());
        std::string s;
        while (std::getline(f, s, FOLDERDELIMCHAR)) {
            tokens.push_back(s);
        }
        recNamePart = tokens.at(Level);
    } catch (...) {
        recNamePart = recName.c_str();
    }

    if( Config.MenuItemRecordingClearPercent && isFolder ) {
        if( recNamePart[0] == '%' ) {
            recNamePart.erase(0, 1);
        }
    }
    return recNamePart.c_str();
}

void cFlatDisplayMenu::PreLoadImages(void) {
    // menu icons
    cString Path = cString::sprintf("%s%s/menuIcons", *Config.iconPath, Setup.OSDTheme);
    cReadDir d(Path);
    struct dirent *e;
    while ((e = d.Next()) != NULL) {
        cString FileName = cString::sprintf("menuIcons/%s", GetFilenameWithoutext(e->d_name));
        imgLoader.LoadIcon(*FileName, fontHeight - marginItem*2, fontHeight - marginItem*2);
    }

    imgLoader.LoadIcon("menuIcons/blank", fontHeight - marginItem*2, fontHeight - marginItem*2);

    int imageHeight = fontHeight;
    int imageBGHeight = imageHeight;
    int imageBGWidth = imageHeight*1.34;
    cImage *imgBG = imgLoader.LoadIcon("logo_background", imageBGWidth, imageBGHeight);
    if( imgBG ) {
        imageBGHeight = imgBG->Height();
        imageBGWidth = imgBG->Width();
    }

    imgLoader.LoadIcon("radio", imageBGWidth - 10, imageBGHeight - 10);
    imgLoader.LoadIcon("changroup", imageBGWidth - 10, imageBGHeight - 10);
    imgLoader.LoadIcon("tv", imageBGWidth - 10, imageBGHeight - 10);
    imgLoader.LoadIcon("timerInactive", imageHeight, imageHeight);
    imgLoader.LoadIcon("timerRecording", imageHeight, imageHeight);
    imgLoader.LoadIcon("timerActive", imageHeight, imageHeight);

    int index = 0;
    cImage *img = NULL;
    for(cChannel *Channel = Channels.First(); Channel && index < LOGO_PRE_CACHE; Channel = Channels.Next(Channel))
    {
        img = imgLoader.LoadLogo(Channel->Name(), imageBGWidth - 4, imageBGHeight - 4);
        if( img )
            index++;
    }

    imgLoader.LoadIcon("radio", 999, topBarHeight - marginItem*2);
    imgLoader.LoadIcon("changroup", 999, topBarHeight - marginItem*2);
    imgLoader.LoadIcon("tv", 999, topBarHeight - marginItem*2);

    imgLoader.LoadIcon("timer_full", imageHeight, imageHeight);
    imgLoader.LoadIcon("timer_partial", imageHeight, imageHeight);
    imgLoader.LoadIcon("vps", imageHeight, imageHeight);

    imgLoader.LoadIcon("recording_new", fontHeight, fontHeight);
    imgLoader.LoadIcon("recording_new", fontSmlHeight, fontSmlHeight);
    imgLoader.LoadIcon("recording_cutted", fontHeight, fontHeight);
    imgLoader.LoadIcon("recording", fontHeight, fontHeight);
    imgLoader.LoadIcon("folder", fontHeight, fontHeight);
}
