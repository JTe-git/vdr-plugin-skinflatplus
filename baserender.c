#include "baserender.h"
#include "flat.h"
#include <vdr/menu.h>
#include "services/epgsearch.h"
#include "services/tvscraper.h"

cFlatBaseRender::cFlatBaseRender(void) {
    font = cFont::CreateFont(Setup.FontOsd, Setup.FontOsdSize );
    fontSml = cFont::CreateFont(Setup.FontSml, Setup.FontSmlSize);
    fontFixed = cFont::CreateFont(Setup.FontFix, Setup.FontFixSize);

    fontHeight = font->Height();
    fontSmlHeight = fontSml->Height();
    fontFixedHeight = fontFixed->Height();

    topBarTitle = "";
    tobBarTitleExtra1 = "";
    tobBarTitleExtra2 = "";
    topBarLastDate = "";
    topBarUpdateTitle = false;
    topBarHeight = 0;
    topBarExtraIconSet = false;
    topBarMenuIconSet = false;
    topBarMenuLogo = "";
    topBarMenuLogoSet = false;

    marginItem = 5;

    scrollBarWidth = 10;

    buttonsHeight = 0;
    buttonsDrawn = false;

    osd = NULL;
    topBarPixmap = NULL;
    buttonsPixmap = NULL;
    messagePixmap = NULL;
    contentPixmap = NULL;
    contentEpgImagePixmap = NULL;
    progressBarPixmap = NULL;
    progressBarPixmapBg = NULL;
    decorPixmap = NULL;

    Config.ThemeCheckAndInit();
    Config.DecorCheckAndInit();
}

cFlatBaseRender::~cFlatBaseRender(void) {
    delete font;
    delete fontSml;
    delete fontFixed;

    if( osd ) {
        if( topBarPixmap )
            osd->DestroyPixmap(topBarPixmap);
        if( buttonsPixmap )
            osd->DestroyPixmap(buttonsPixmap);
        if( messagePixmap )
            osd->DestroyPixmap(messagePixmap);
        if( contentPixmap )
            osd->DestroyPixmap(contentPixmap);
        if( progressBarPixmap )
            osd->DestroyPixmap(progressBarPixmap);
        if( progressBarPixmapBg )
            osd->DestroyPixmap(progressBarPixmapBg);
        if( decorPixmap )
            osd->DestroyPixmap(decorPixmap);
        if( topBarIconPixmap )
            osd->DestroyPixmap(topBarIconPixmap);
        if( topBarIconBGPixmap )
            osd->DestroyPixmap(topBarIconBGPixmap);
        if( contentEpgImagePixmap )
            osd->DestroyPixmap(contentEpgImagePixmap);

        delete osd;
    }
}

void cFlatBaseRender::CreateFullOsd(void) {
    CreateOsd(cOsd::OsdLeft() + Config.marginOsdHor, cOsd::OsdTop() + Config.marginOsdVer, cOsd::OsdWidth() - Config.marginOsdHor*2, cOsd::OsdHeight() - Config.marginOsdVer*2);
}

void cFlatBaseRender::CreateOsd(int left, int top, int width, int height) {
    osdLeft = left;
    osdTop = top;
    osdWidth = width;
    osdHeight = height;

    osd = cOsdProvider::NewOsd(left, top);
    if (osd) {
        tArea Area = { 0, 0, width, height,  32 };
        if (osd->SetAreas(&Area, 1) == oeOk) {
            //dsyslog("skinflatplus: create osd SUCCESS left: %d top: %d width: %d height: %d", left, top, width, height);
            return;
        }
    }
    esyslog("skinflatplus: create osd FAILED left: %d top: %d width: %d height: %d", left, top, width, height);
    return;
}

void cFlatBaseRender::TopBarCreate(void) {
    int fs = int(round(cOsd::OsdHeight() * Config.TopBarFontSize));
    topBarFont = cFont::CreateFont(Setup.FontOsd, fs);
    topBarFontSml = cFont::CreateFont(Setup.FontOsd, fs / 2);
    topBarFontHeight = topBarFont->Height();
    topBarFontSmlHeight = topBarFontSml->Height();

    if( topBarFontHeight > topBarFontSmlHeight*2 )
        topBarHeight = topBarFontHeight;
    else
        topBarHeight = topBarFontSmlHeight * 2;

    topBarPixmap = osd->CreatePixmap(1, cRect(Config.decorBorderTopBarSize, Config.decorBorderTopBarSize, osdWidth - Config.decorBorderTopBarSize*2, topBarHeight));
    topBarIconBGPixmap = osd->CreatePixmap(2, cRect(Config.decorBorderTopBarSize, Config.decorBorderTopBarSize, osdWidth - Config.decorBorderTopBarSize*2, topBarHeight));
    topBarIconPixmap = osd->CreatePixmap(3, cRect(Config.decorBorderTopBarSize, Config.decorBorderTopBarSize, osdWidth - Config.decorBorderTopBarSize*2, topBarHeight));
    topBarPixmap->Fill(clrTransparent);
    topBarIconBGPixmap->Fill(clrTransparent);
    topBarIconPixmap->Fill(clrTransparent);

    if( Config.DiskUsageShow == 3)
        TopBarEnableDiskUsage();
}

void cFlatBaseRender::TopBarSetTitle(cString title) {
    topBarTitle = title;
    tobBarTitleExtra1 = "";
    tobBarTitleExtra2 = "";
    topBarExtraIcon = "";
    topBarMenuIcon = "";
    topBarUpdateTitle = true;
    topBarExtraIconSet = false;
    topBarMenuIconSet = false;
    topBarMenuLogo = "";
    topBarMenuLogoSet = false;
    if( Config.DiskUsageShow == 3)
        TopBarEnableDiskUsage();
}

void cFlatBaseRender::TopBarSetTitleExtra(cString extra1, cString extra2) {
    tobBarTitleExtra1 = extra1;
    tobBarTitleExtra2 = extra2;
    topBarUpdateTitle = true;
}

void cFlatBaseRender::TopBarSetExtraIcon(cString icon) {
    if( !strcmp(*icon, "") )
        return;
    topBarExtraIcon = icon;
    topBarExtraIconSet = true;
    topBarUpdateTitle = true;
}

void cFlatBaseRender::TopBarSetMenuIcon(cString icon) {
    if( !strcmp(*icon, "") )
        return;
    topBarMenuIcon = icon;
    topBarMenuIconSet = true;
    topBarUpdateTitle = true;
}

void cFlatBaseRender::TopBarSetMenuLogo(cString icon) {
    if( !strcmp(*icon, "") )
        return;
    topBarMenuLogo = icon;
    topBarMenuLogoSet = true;
    topBarUpdateTitle = true;
}

void cFlatBaseRender::TopBarEnableDiskUsage(void) {
    cVideoDiskUsage::HasChanged(VideoDiskUsageState);
    int DiskUsage = cVideoDiskUsage::UsedPercent();
    double FreeGB = cVideoDiskUsage::FreeMB() / 1024.0;
    int FreeMinutes = cVideoDiskUsage::FreeMinutes();
    cString extra1 = cString::sprintf("%s: %d%%", tr("disk usage"), DiskUsage);
    cString extra2 = cString::sprintf("%s: %.1f GB ~ %02d:%02d", tr("free"), FreeGB, FreeMinutes / 60, FreeMinutes % 60);

    cString iconName("chart1");
    if( DiskUsage > 14 )
        iconName = "chart2";
    if( DiskUsage > 28 )
        iconName = "chart3";
    if( DiskUsage > 42 )
        iconName = "chart4";
    if( DiskUsage > 56 )
        iconName = "chart5";
    if( DiskUsage > 70 )
        iconName = "chart6";
    if( DiskUsage > 84 )
        iconName = "chart7";

    TopBarSetTitleExtra(extra1, extra2);
    TopBarSetExtraIcon(iconName);
}
// sollte bei jedum "Flush" aufgerufen werden!
void cFlatBaseRender::TopBarUpdate(void) {
    cString curDate = DayDateTime();
    int TopBarWidth = osdWidth - Config.decorBorderTopBarSize*2;
    int MenuIconWidth = 0;

    if ( strcmp(curDate, topBarLastDate) || topBarUpdateTitle ) {
        topBarUpdateTitle = false;
        topBarLastDate = curDate;

        int TitleWidthLeft = TopBarWidth;

        int fontTop = (topBarHeight - topBarFontHeight) / 2;
        int fontSmlTop = (topBarHeight - topBarFontSmlHeight*2) / 2;

        int RecLeft = TopBarWidth / 2;

        topBarPixmap->Fill(Theme.Color(clrTopBarBg));
        topBarIconPixmap->Fill(clrTransparent);
        topBarIconBGPixmap->Fill(clrTransparent);

        int TitleWidth = topBarFont->Width(topBarTitle);
        time_t t;
        time(&t);

        cString time = TimeString(t);
        cString time2 = cString::sprintf("%s %s", *time, tr("clock"));

        int timeWidth = topBarFont->Width(*time2) + marginItem*2;
        topBarPixmap->DrawText(cPoint(TopBarWidth - timeWidth, fontTop), time2, Theme.Color(clrTopBarTimeFont), Theme.Color(clrTopBarBg), topBarFont);

        TitleWidthLeft -= timeWidth;

        cString weekday = WeekDayNameFull(t);
        int weekdayWidth = topBarFontSml->Width(*weekday);

        cString date = ShortDateString(t);
        int dateWidth = topBarFontSml->Width(*date);

        int fullWidth = max(weekdayWidth, dateWidth);
        TitleWidthLeft -= fullWidth;
        int DateRight = TopBarWidth - timeWidth - fullWidth - marginItem*2;

        if( topBarMenuIconSet && Config.TopBarMenuIconShow ) {
            int IconLeft = marginItem;
            cImage *img = imgLoader.LoadIcon(*topBarMenuIcon, 999, topBarHeight - marginItem*2);
            if( img ) {
                int iconTop = (topBarHeight / 2 - img->Height()/2);
                topBarIconPixmap->DrawImage(cPoint(IconLeft, iconTop), *img);
                MenuIconWidth = img->Width()+marginItem*2;
                TitleWidthLeft -= MenuIconWidth + marginItem*3;
            }
        }

        if( topBarMenuLogoSet && Config.TopBarMenuIconShow ) {
            topBarIconPixmap->Fill(clrTransparent);
            int IconLeft = marginItem;
            int imageBGHeight = topBarHeight - marginItem*2;
            int imageBGWidth = imageBGHeight*1.34;
            int iconTop = 0;

            cImage *imgBG = imgLoader.LoadIcon("logo_background", imageBGWidth, imageBGHeight);
            if( imgBG ) {
                imageBGHeight = imgBG->Height();
                imageBGWidth = imgBG->Width();
                iconTop = (topBarHeight / 2 - imgBG->Height()/2);
                topBarIconBGPixmap->DrawImage( cPoint(IconLeft, iconTop), *imgBG );
            }

            cImage *img = imgLoader.LoadLogo(*topBarMenuLogo, imageBGWidth - 4, imageBGHeight - 4);
            if( img ) {
                iconTop += (imageBGHeight - img->Height())/2;
                IconLeft += (imageBGWidth - img->Width())/2;
                topBarIconPixmap->DrawImage(cPoint(IconLeft, iconTop), *img);
            }
            MenuIconWidth = imageBGWidth+marginItem*2;
            TitleWidthLeft -= MenuIconWidth + marginItem*3;
        }

        int extra1Width = topBarFontSml->Width(tobBarTitleExtra1);
        int extra2Width = topBarFontSml->Width(tobBarTitleExtra2);
        int extraMaxWidth = max(extra1Width, extra2Width);

        int extraLeft = TopBarWidth/2 - extraMaxWidth/2;
        extraLeft = max(MenuIconWidth + marginItem*4 + TitleWidth, extraLeft);

        topBarPixmap->DrawText(cPoint(extraLeft, fontSmlTop), tobBarTitleExtra1, Theme.Color(clrTopBarDateFont), Theme.Color(clrTopBarBg), topBarFontSml, extraMaxWidth, 0, taRight);
        topBarPixmap->DrawText(cPoint(extraLeft, fontSmlTop + topBarFontSmlHeight), tobBarTitleExtra2, Theme.Color(clrTopBarDateFont), Theme.Color(clrTopBarBg), topBarFontSml, extraMaxWidth, 0, taRight);

        RecLeft = extraLeft + extraMaxWidth + marginItem;

        if( topBarExtraIconSet ) {
            int extraIconLeft = extraLeft + extraMaxWidth + marginItem;
            cImage *img = imgLoader.LoadIcon(*topBarExtraIcon, 999, topBarHeight);
            if( img ) {
                if( (extraIconLeft + img->Width() + marginItem) < DateRight ) {
                    int iconTop = 0;
                    topBarIconPixmap->DrawImage(cPoint(extraIconLeft, iconTop), *img);

                    RecLeft += topBarHeight + marginItem;
                }
            }
        }

        topBarPixmap->DrawText(cPoint(TopBarWidth - timeWidth - fullWidth - marginItem*2, fontSmlTop), weekday, Theme.Color(clrTopBarDateFont), Theme.Color(clrTopBarBg), topBarFontSml, fullWidth, 0, taRight);
        topBarPixmap->DrawText(cPoint(TopBarWidth - timeWidth - fullWidth - marginItem*2, fontSmlTop + topBarFontSmlHeight), date, Theme.Color(clrTopBarDateFont), Theme.Color(clrTopBarBg), topBarFontSml, fullWidth, 0, taRight);

        DecorBorderDraw(Config.decorBorderTopBarSize, Config.decorBorderTopBarSize, osdWidth - Config.decorBorderTopBarSize*2, topBarHeight, Config.decorBorderTopBarSize, Config.decorBorderTopBarType, Config.decorBorderTopBarFg, Config.decorBorderTopBarBg);

        int RecW = 0;
        int numRec = 0;
        cImage *imgRec = imgLoader.LoadIcon("topbar_timer", topBarFontHeight - marginItem*2, topBarFontHeight - marginItem*2);
        if( imgRec )
            RecW = imgRec->Width();
        if( Config.TopBarRecordingShow ) {
            // look for timers
            for(cTimer *ti = Timers.First(); ti; ti = Timers.Next(ti)) {
                if( ti->Matches(t) ) {
                    numRec++;
                }
            }
            if( numRec ) {
                cString Rec = cString::sprintf("%d", numRec);
                RecW += topBarFont->Width(*Rec);
                TitleWidthLeft -= RecW + marginItem*2;
            }
        }

        int numConflicts = 0, ConW = 0;
        if( Config.TopBarRecConflictsShow ) {
            cPlugin *p = cPluginManager::GetPlugin("epgsearch");
            if (p) {
                Epgsearch_lastconflictinfo_v1_0 *serviceData = new Epgsearch_lastconflictinfo_v1_0;
                if (serviceData) {
                    serviceData->nextConflict = 0;
                    serviceData->relevantConflicts = 0;
                    serviceData->totalConflicts = 0;
                    p->Service("Epgsearch-lastconflictinfo-v1.0", serviceData);
                    if (serviceData->relevantConflicts > 0) {
                        numConflicts = serviceData->relevantConflicts;
                    }
                    delete serviceData;
                }
            }
        }
        cImage *imgCon = NULL;
        if( numConflicts < Config.TopBarRecConflictsHigh )
            imgCon = imgLoader.LoadIcon("topbar_timerconflict_low", topBarFontHeight - marginItem*2, topBarFontHeight - marginItem*2);
        else
            imgCon = imgLoader.LoadIcon("topbar_timerconflict_high", topBarFontHeight - marginItem*2, topBarFontHeight - marginItem*2);

        if( imgCon )
            ConW = imgCon->Width();
        if( numConflicts ) {
            cString Con = cString::sprintf("%d", numRec);
            ConW += topBarFont->Width(*Con);
            TitleWidthLeft -= ConW + marginItem*2;
        }

        if( TitleWidth > TitleWidthLeft ) {
            int dotsWidth = topBarFont->Width("... ");
            cTextWrapper TitleWrapper(topBarTitle, topBarFont, TitleWidthLeft - dotsWidth);
            topBarTitle = TitleWrapper.GetLine(0);
            topBarTitle = cString::sprintf("%s...", *topBarTitle);
            TitleWidth = topBarFont->Width(topBarTitle);
        }

        topBarPixmap->DrawText(cPoint(MenuIconWidth + marginItem*2, fontTop), topBarTitle, Theme.Color(clrTopBarFont), Theme.Color(clrTopBarBg), topBarFont);

        if( TitleWidth > RecLeft )
            RecLeft = TitleWidth + marginItem*2;
        if( numRec > 0 && Config.TopBarRecordingShow && (RecLeft + RecW) < DateRight ) {
            if( imgRec ) {
                int iconTop = (topBarFontHeight - imgRec->Height()) / 2;
                topBarIconPixmap->DrawImage( cPoint(RecLeft, iconTop), *imgRec );
                RecW = imgRec->Width() + marginItem;
            }
            cString RecNum = cString::sprintf("%d", numRec);
            topBarPixmap->DrawText(cPoint(RecLeft + RecW, fontSmlTop), RecNum, Theme.Color(clrTopBarRecordingActiveFg), Theme.Color(clrTopBarRecordingActiveBg), topBarFontSml);
            RecLeft += RecW + marginItem*2 + topBarFontSml->Width(RecNum);
        }

        int ConLeft = RecLeft + marginItem;
        if( numConflicts > 0 && Config.TopBarRecConflictsShow && (ConLeft + ConW) < DateRight ) {
            if( imgCon ) {
                int iconTop = (topBarFontHeight - imgCon->Height()) / 2;
                topBarIconPixmap->DrawImage( cPoint(RecLeft, iconTop), *imgCon );
                ConW = imgCon->Width();
            }

            cString ConNum = cString::sprintf("%d", numConflicts);
            if( numConflicts < Config.TopBarRecConflictsHigh )
                topBarPixmap->DrawText(cPoint(ConLeft + ConW + marginItem, fontSmlTop), ConNum, Theme.Color(clrTopBarConflictLowFg), Theme.Color(clrTopBarConflictLowBg), topBarFontSml);
            else
                topBarPixmap->DrawText(cPoint(ConLeft + ConW + marginItem, fontSmlTop), ConNum, Theme.Color(clrTopBarConflictHighFg), Theme.Color(clrTopBarConflictHighBg), topBarFontSml);
        }

    }
}

void cFlatBaseRender::ButtonsCreate(void) {
    marginButtonColor = 10;
    buttonColorHeight = 8;
    buttonsHeight = fontHeight + marginButtonColor + buttonColorHeight;
    buttonsWidth = osdWidth;
    buttonsTop = osdHeight - buttonsHeight - Config.decorBorderButtonSize;

    buttonsPixmap = osd->CreatePixmap(1, cRect(Config.decorBorderButtonSize,
        buttonsTop, buttonsWidth - Config.decorBorderButtonSize*2, buttonsHeight));
    buttonsPixmap->Fill(clrTransparent);
}

void cFlatBaseRender::ButtonsSet(const char *Red, const char *Green, const char *Yellow, const char *Blue) {
    int WidthMargin = buttonsWidth - marginItem*3;
    int buttonWidth = (WidthMargin / 4) - Config.decorBorderButtonSize*2;

    buttonsPixmap->Fill(clrTransparent);
    DecorBorderClearByFrom(BorderButton);

    buttonsDrawn = false;

    int x = 0;
    if( !(!Config.ButtonsShowEmpty && !Red) ) {
        buttonsPixmap->DrawText(cPoint(x, 0), Red, Theme.Color(clrButtonFont), Theme.Color(clrButtonBg), font, buttonWidth, fontHeight + marginButtonColor, taCenter);
        buttonsPixmap->DrawRectangle(cRect(x, fontHeight + marginButtonColor, buttonWidth, buttonColorHeight), Theme.Color(clrButtonRed));
        DecorBorderDraw(x + Config.decorBorderButtonSize, buttonsTop, buttonWidth, buttonsHeight, Config.decorBorderButtonSize, Config.decorBorderButtonType,
            Config.decorBorderButtonFg, Config.decorBorderButtonBg, BorderButton);
        buttonsDrawn = true;
    }

    x += buttonWidth + marginItem + Config.decorBorderButtonSize*2;
    if( !(!Config.ButtonsShowEmpty && !Green) ) {
        buttonsPixmap->DrawText(cPoint(x, 0), Green, Theme.Color(clrButtonFont), Theme.Color(clrButtonBg), font, buttonWidth, fontHeight + marginButtonColor, taCenter);
        buttonsPixmap->DrawRectangle(cRect(x, fontHeight + marginButtonColor, buttonWidth, buttonColorHeight), Theme.Color(clrButtonGreen));
        DecorBorderDraw(x + Config.decorBorderButtonSize, buttonsTop, buttonWidth, buttonsHeight, Config.decorBorderButtonSize, Config.decorBorderButtonType,
            Config.decorBorderButtonFg, Config.decorBorderButtonBg, BorderButton);
        buttonsDrawn = true;
    }

    x += buttonWidth + marginItem + Config.decorBorderButtonSize*2;
    if( !(!Config.ButtonsShowEmpty && !Yellow) ) {
        buttonsPixmap->DrawText(cPoint(x, 0), Yellow, Theme.Color(clrButtonFont), Theme.Color(clrButtonBg), font, buttonWidth, fontHeight + marginButtonColor, taCenter);
        buttonsPixmap->DrawRectangle(cRect(x, fontHeight + marginButtonColor, buttonWidth, buttonColorHeight), Theme.Color(clrButtonYellow));
        DecorBorderDraw(x + Config.decorBorderButtonSize, buttonsTop, buttonWidth, buttonsHeight, Config.decorBorderButtonSize, Config.decorBorderButtonType,
            Config.decorBorderButtonFg, Config.decorBorderButtonBg, BorderButton);
        buttonsDrawn = true;
    }

    x += buttonWidth + marginItem + Config.decorBorderButtonSize*2;
    if( x + buttonWidth + Config.decorBorderButtonSize*2 < buttonsWidth )
        buttonWidth += buttonsWidth - (x + buttonWidth + Config.decorBorderButtonSize*2);
    if( !(!Config.ButtonsShowEmpty && !Blue) ) {
        buttonsPixmap->DrawText(cPoint(x, 0), Blue, Theme.Color(clrButtonFont), Theme.Color(clrButtonBg), font, buttonWidth, fontHeight + marginButtonColor, taCenter);
        buttonsPixmap->DrawRectangle(cRect(x, fontHeight + marginButtonColor, buttonWidth, buttonColorHeight), Theme.Color(clrButtonBlue));
        DecorBorderDraw(x + Config.decorBorderButtonSize, buttonsTop, buttonWidth, buttonsHeight, Config.decorBorderButtonSize, Config.decorBorderButtonType,
            Config.decorBorderButtonFg, Config.decorBorderButtonBg, BorderButton);
        buttonsDrawn = true;
    }
}

bool cFlatBaseRender::ButtonsDrawn(void) {
    return buttonsDrawn;
}

void cFlatBaseRender::MessageCreate(void) {
    messageHeight = fontHeight + marginItem*2;
    int top = osdHeight - Config.MessageOffset - messageHeight - Config.decorBorderMessageSize;
    messagePixmap = osd->CreatePixmap(5, cRect(Config.decorBorderMessageSize, top, osdWidth - Config.decorBorderMessageSize*2, messageHeight));
    messagePixmap->Fill(clrTransparent);
}

void cFlatBaseRender::MessageSet(eMessageType Type, const char *Text) {
    tColor col = Theme.Color(clrMessageStatus);
    switch (Type) {
        case mtStatus:
            col = Theme.Color(clrMessageStatus);
            break;
        case mtInfo:
            col = Theme.Color(clrMessageInfo);
            break;
        case mtWarning:
            col = Theme.Color(clrMessageWarning);
            break;
        case mtError:
            col = Theme.Color(clrMessageError);
            break;
    }
    messagePixmap->Fill(Theme.Color(clrMessageBg));

    messagePixmap->DrawRectangle(cRect( 0, 0, messageHeight, messageHeight), col);
    messagePixmap->DrawRectangle(cRect( osdWidth - messageHeight - Config.decorBorderMessageSize*2, 0, messageHeight, messageHeight), col);

    int textWidth = font->Width(Text);

    if( Config.MenuItemParseTilde ) {
        std::string tilde = Text;
        size_t found = tilde.find(" ~ ");
        size_t found2 = tilde.find("~");
        if( found != std::string::npos ) {
            std::string first = tilde.substr(0, found);
            std::string second = tilde.substr(found +2, tilde.length() );

            messagePixmap->DrawText(cPoint((osdWidth - textWidth) / 2, marginItem), first.c_str(), Theme.Color(clrMessageFont), Theme.Color(clrMessageBg), font);
            int l = font->Width( first.c_str() );
            messagePixmap->DrawText(cPoint((osdWidth - textWidth) / 2 + l, marginItem), second.c_str(), Theme.Color(clrMenuItemExtraTextFont), Theme.Color(clrMessageBg), font);
        } else if ( found2 != std::string::npos ) {
            std::string first = tilde.substr(0, found2);
            std::string second = tilde.substr(found2 +1, tilde.length() );

            messagePixmap->DrawText(cPoint((osdWidth - textWidth) / 2, marginItem), first.c_str(), Theme.Color(clrMessageFont), Theme.Color(clrMessageBg), font);
            int l = font->Width( first.c_str() );
            l += font->Width("X");
            messagePixmap->DrawText(cPoint((osdWidth - textWidth) / 2 + l, marginItem), second.c_str(), Theme.Color(clrMenuItemExtraTextFont), Theme.Color(clrMessageBg), font);
        } else
            messagePixmap->DrawText(cPoint((osdWidth - textWidth) / 2, marginItem), Text, Theme.Color(clrMessageFont), Theme.Color(clrMessageBg), font);
    } else
        messagePixmap->DrawText(cPoint((osdWidth - textWidth) / 2, marginItem), Text, Theme.Color(clrMessageFont), Theme.Color(clrMessageBg), font);


    int top = osdHeight - Config.MessageOffset - messageHeight - Config.decorBorderMessageSize;
    DecorBorderDraw(Config.decorBorderMessageSize, top, osdWidth - Config.decorBorderMessageSize*2, messageHeight, Config.decorBorderMessageSize, Config.decorBorderMessageType, Config.decorBorderMessageFg, Config.decorBorderMessageBg, BorderMessage);
}

void cFlatBaseRender::MessageClear(void) {
    messagePixmap->Fill(clrTransparent);
    DecorBorderClearByFrom(BorderMessage);
    DecorBorderRedrawAll();
}

void cFlatBaseRender::ProgressBarCreate(int Left, int Top, int Width, int Height, int MarginHor, int MarginVer, tColor ColorFg, tColor ColorBarFg, tColor ColorBg, int Type, bool SetBackground, bool isSignal) {
    progressBarTop = Top;
    progressBarWidth = Width;
    progressBarHeight = Height;
    ProgressType = Type;
    progressBarMarginHor = MarginHor;
    progressBarMarginVer = MarginVer;

    progressBarColorFg = ColorFg;
    progressBarColorBarFg = ColorBarFg;
    progressBarColorBg = ColorBg;

    progressBarSetBackground = SetBackground;
    progressBarIsSignal = isSignal;

    progressBarColorBarCurFg = Theme.Color(clrReplayProgressBarCurFg);

    progressBarPixmap = osd->CreatePixmap(3, cRect(Left, Top, Width, progressBarHeight));
    progressBarPixmapBg = osd->CreatePixmap(2, cRect(Left - progressBarMarginVer, Top - progressBarMarginHor, Width + progressBarMarginVer*2, progressBarHeight + progressBarMarginHor*2));
    progressBarPixmap->Fill(clrTransparent);
    progressBarPixmapBg->Fill(clrTransparent);
}

void cFlatBaseRender::ProgressBarDraw(int Current, int Total) {
    ProgressBarDrawRaw(progressBarPixmap, progressBarPixmapBg, cRect(0, 0, progressBarWidth, progressBarHeight),
        cRect(0, 0, progressBarWidth+progressBarMarginVer*2, progressBarHeight+progressBarMarginHor*2),
        Current, Total, progressBarColorFg, progressBarColorBarFg, progressBarColorBg, ProgressType, progressBarSetBackground, progressBarIsSignal);
}

void cFlatBaseRender::ProgressBarDrawBgColor(void) {
    progressBarPixmapBg->Fill(progressBarColorBg);
}

void cFlatBaseRender::ProgressBarDrawRaw(cPixmap *Pixmap, cPixmap *PixmapBg, cRect rect, cRect rectBg, int Current, int Total, tColor ColorFg, tColor ColorBarFg, tColor ColorBg, int Type, bool SetBackground, bool isSignal) {
    int Middle = rect.Height()/2;

    double percentLeft = ((double)Current) / (double)Total;

    if( PixmapBg && SetBackground )
        PixmapBg->DrawRectangle(cRect( rectBg.Left(), rectBg.Top(), rectBg.Width(), rectBg.Height()), ColorBg);

    if( SetBackground ) {
        if( PixmapBg == Pixmap )
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top(), rect.Width(), rect.Height()), ColorBg);
        else
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top(), rect.Width(), rect.Height()), clrTransparent);
    }

    switch( Type ) {
        case 0: // small line + big line
        {
            int sml = rect.Height() / 10 * 2;
            if( sml <= 1 )
                sml = 2;
            int big = rect.Height();

            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + Middle - (sml/2), rect.Width(), sml), ColorFg);

            if (Current > 0)
                Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + Middle - (big/2), (rect.Width() * percentLeft), big), ColorBarFg);
            break;
        }
        case 1: // big line
        {
            int big = rect.Height();

            if (Current > 0)
                Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + Middle - (big/2), (rect.Width() * percentLeft), big), ColorBarFg);
            break;
        }
        case 2: // big line + outline
        {
            int big = rect.Height();
            int out = 1;
            if( rect.Height() > 10 )
                out = 2;
            // outline
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top(), rect.Width(), out), ColorFg);
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + rect.Height() - out, rect.Width(), out), ColorFg);

            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top(), out, rect.Height()), ColorFg);
            Pixmap->DrawRectangle(cRect( rect.Left() + rect.Width() - out, rect.Top(), out, rect.Height()), ColorFg);

            if (Current > 0) {
                if( isSignal ) {
                    double perc = 100.0 / (double) Total * (double) Current / 100.0;
                    if( perc > 0.666 ) {
                        Pixmap->DrawRectangle(cRect( rect.Left() + out, rect.Top() + Middle - (big/2) + out, (rect.Width() * percentLeft) - out*2, big - out*2), Theme.Color(clrButtonGreen));
                        Pixmap->DrawRectangle(cRect( rect.Left() + out, rect.Top() + Middle - (big/2) + out, (rect.Width() * 0.666) - out*2, big - out*2), Theme.Color(clrButtonYellow));
                        Pixmap->DrawRectangle(cRect( rect.Left() + out, rect.Top() + Middle - (big/2) + out, (rect.Width() * 0.333) - out*2, big - out*2), Theme.Color(clrButtonRed));
                    } else if( perc > 0.333 ) {
                        Pixmap->DrawRectangle(cRect( rect.Left() + out, rect.Top() + Middle - (big/2) + out, (rect.Width() * percentLeft) - out*2, big - out*2), Theme.Color(clrButtonYellow));
                        Pixmap->DrawRectangle(cRect( rect.Left() + out, rect.Top() + Middle - (big/2) + out, (rect.Width() * 0.333) - out*2, big - out*2), Theme.Color(clrButtonRed));
                    } else
                        Pixmap->DrawRectangle(cRect( rect.Left() + out, rect.Top() + Middle - (big/2) + out, (rect.Width() * percentLeft) - out*2, big - out*2), Theme.Color(clrButtonRed));

                } else
                    Pixmap->DrawRectangle(cRect( rect.Left() + out, rect.Top() + Middle - (big/2) + out, (rect.Width() * percentLeft) - out*2, big - out*2), ColorBarFg);
            }
            break;
        }
        case 3: // small line + big line + dot
        {
            int sml = rect.Height() / 10 * 2;
            if( sml <= 1 )
                sml = 2;
            int big = rect.Height();

            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + Middle - (sml/2), rect.Width(), sml), ColorFg);

            if (Current > 0) {
                Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + Middle - (big/2), (rect.Width() * percentLeft), big), ColorBarFg);
                // dot
                Pixmap->DrawEllipse(cRect( rect.Left() + (rect.Width() * percentLeft) - (big/2), rect.Top() + Middle - (big/2), big, big), ColorBarFg, 0);
            }
            break;
        }
        case 4: // big line + dot
        {
            int big = rect.Height();

            if (Current > 0) {
                Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + Middle - (big/2), (rect.Width() * percentLeft), big), ColorBarFg);
                // dot
                Pixmap->DrawEllipse(cRect( rect.Left() + (rect.Width() * percentLeft) - (big/2), rect.Top() + Middle - (big/2), big, big), ColorBarFg, 0);
            }
            break;
        }
        case 5: // big line + outline + dot
        {
            int big = rect.Height();
            int out = 1;
            if( rect.Height() > 10 )
                out = 2;
            // outline
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top(), rect.Width(), out), ColorFg);
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + rect.Height() - out, rect.Width(), out), ColorFg);
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top(), out, rect.Height()), ColorFg);
            Pixmap->DrawRectangle(cRect( rect.Left() + rect.Width() - out, rect.Top(), out, rect.Height()), ColorFg);

            if (Current > 0) {
                Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + Middle - (big/2), (rect.Width() * percentLeft), big), ColorBarFg);
                // dot
                Pixmap->DrawEllipse(cRect( rect.Left() + (rect.Width() * percentLeft) - (big/2), rect.Top() + Middle - (big/2), big, big), ColorBarFg, 0);
            }
            break;
        }
        case 6: // small line + dot
        {
            int sml = rect.Height() / 10 * 2;
            if( sml <= 1 )
                sml = 2;
            int big = rect.Height();

            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + Middle - (sml/2), rect.Width(), sml), ColorFg);

            if (Current > 0) {
                // dot
                Pixmap->DrawEllipse(cRect( rect.Left() + (rect.Width() * percentLeft) - (big/2), rect.Top() + Middle - (big/2), big, big), ColorBarFg, 0);
            }
            break;
        }
        case 7: // outline + dot
        {
            int big = rect.Height();
            int out = 1;
            if( rect.Height() > 10 )
                out = 2;
            // outline
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top(), rect.Width(), out), ColorFg);
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + rect.Height() - out, rect.Width(), out), ColorFg);
            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top(), out, rect.Height()), ColorFg);
            Pixmap->DrawRectangle(cRect( rect.Left() + rect.Width() - out, rect.Top(), out, rect.Height()), ColorFg);

            if (Current > 0) {
                // dot
                Pixmap->DrawEllipse(cRect( rect.Left() + (rect.Width() * percentLeft) - (big/2), rect.Top() + Middle - (big/2), big, big), ColorBarFg, 0);
            }
            break;
        }
        case 8: // small line + big line + alpha blend
        {
            int sml = rect.Height() / 10 * 2;
            if( sml <= 1 )
                sml = 2;
            int big = rect.Height()/2 - sml/2;

            Pixmap->DrawRectangle(cRect( rect.Left(), rect.Top() + Middle - (sml/2), rect.Width(), sml), ColorFg);

            if (Current > 0) {
                DecorDrawGlowRectHor(Pixmap, rect.Left(), rect.Top(), (rect.Width() * percentLeft), big, ColorBarFg);
                DecorDrawGlowRectHor(Pixmap, rect.Left(), rect.Top() + Middle + sml/2, (rect.Width() * percentLeft), big*-1, ColorBarFg);
            }
            break;
        }
        case 9: // big line + alpha blend
        {
            int big = rect.Height();

            if (Current > 0) {
                DecorDrawGlowRectHor(Pixmap, rect.Left(), rect.Top() + Middle - big/2, (rect.Width() * percentLeft), big/2, ColorBarFg);
                DecorDrawGlowRectHor(Pixmap, rect.Left(), rect.Top() + Middle, (rect.Width() * percentLeft), big/-2, ColorBarFg);
            }
            break;
        }
    }
}

void cFlatBaseRender::ProgressBarDrawMarks(int Current, int Total, const cMarks *Marks, tColor Color, tColor ColorCurrent) {
    progressBarColorMark = Color;
    progressBarColorMarkCurrent = ColorCurrent;
    int posMark = 0, posMarkLast = 0, posCurrent = 0;

    int top = progressBarHeight / 2;
    if( progressBarPixmapBg )
        progressBarPixmapBg->DrawRectangle(cRect( 0, progressBarMarginHor + progressBarHeight, progressBarWidth, progressBarMarginHor), progressBarColorBg);

    progressBarPixmap->Fill( progressBarColorBg );

    int sml = Config.decorProgressReplaySize / 10 * 2;
    if( sml <= 4 )
        sml = 4;
    int big = Config.decorProgressReplaySize - sml*2 - 2;

    if( !Marks ) {
        progressBarColorFg = progressBarColorBarCurFg;
        progressBarColorBarFg = progressBarColorBarCurFg;

        ProgressBarDraw(Current, Total);
        return;
    }
    if( !Marks->First() ) {
        progressBarColorFg = progressBarColorBarCurFg;
        progressBarColorBarFg = progressBarColorBarCurFg;

        ProgressBarDraw(Current, Total);
        return;
    }

    // the small line
    progressBarPixmap->DrawRectangle(cRect( 0, top - sml/2, progressBarWidth, sml), progressBarColorFg);

    bool Start = true;

    for( const cMark *m = Marks->First(); m; m = Marks->Next(m) ) {
        posMark = ProgressBarMarkPos( m->Position(), Total );
        posCurrent = ProgressBarMarkPos( Current, Total );

        ProgressBarDrawMark(posMark, posMarkLast, posCurrent, Start, m->Position() == Current);
        posMarkLast = posMark;
        Start = !Start;
    }

    // draw last marker vertical line
    if( posCurrent == posMark )
        progressBarPixmap->DrawRectangle(cRect( posMark - sml, 0, sml*2, progressBarHeight), progressBarColorMarkCurrent);
    else
        progressBarPixmap->DrawRectangle(cRect( posMark - sml/2, 0, sml, progressBarHeight), progressBarColorMark);

    if( !Start ) {
        //progressBarPixmap->DrawRectangle(cRect( posMarkLast + sml/2, top - big/2, progressBarWidth - posMarkLast, big), progressBarColorBarFg);
        if( posCurrent > posMarkLast )
            progressBarPixmap->DrawRectangle(cRect( posMarkLast + sml/2, top - big/2, posCurrent - posMarkLast, big), progressBarColorBarCurFg);
    } else {
        // marker
        progressBarPixmap->DrawRectangle(cRect( posMarkLast, top - sml/2, posCurrent - posMarkLast, sml), progressBarColorBarCurFg);
        progressBarPixmap->DrawRectangle(cRect( posCurrent - big/2, top - big/2, big, big), progressBarColorBarCurFg);

        if( posCurrent > posMarkLast + sml/2 )
            progressBarPixmap->DrawRectangle(cRect( posMarkLast - sml/2, 0, sml, progressBarHeight), progressBarColorMark);
    }
}

int cFlatBaseRender::ProgressBarMarkPos(int P, int Total) {
    return (int64_t)P * progressBarWidth / Total;
}

void cFlatBaseRender::ProgressBarDrawMark(int posMark, int posMarkLast, int posCurrent, bool Start, bool isCurrent)
{
    int top = progressBarHeight / 2;
    int sml = Config.decorProgressReplaySize / 10 * 2;
    if( sml <= 4 )
        sml = 4;
    int big = Config.decorProgressReplaySize - sml*2 - 2;

    int mbig = Config.decorProgressReplaySize*2;
    if( Config.decorProgressReplaySize > 15 )
        mbig = Config.decorProgressReplaySize;

    // marker vertical line
    if( posCurrent == posMark )
        progressBarPixmap->DrawRectangle(cRect( posMark - sml, 0, sml*2, progressBarHeight), progressBarColorMarkCurrent);
    else
        progressBarPixmap->DrawRectangle(cRect( posMark - sml/2, 0, sml, progressBarHeight), progressBarColorMark);

    if( Start ) {
        if( posCurrent > posMark )
            progressBarPixmap->DrawRectangle(cRect( posMarkLast, top - sml/2, posMark - posMarkLast, sml), progressBarColorBarCurFg);
        else {
            // marker
            progressBarPixmap->DrawRectangle(cRect( posCurrent - big/2, top - big/2, big, big), progressBarColorBarCurFg);
            if( posCurrent > posMarkLast )
                progressBarPixmap->DrawRectangle(cRect( posMarkLast, top - sml/2, posCurrent - posMarkLast, sml), progressBarColorBarCurFg);
        }
        // marker top
        if( isCurrent )
            progressBarPixmap->DrawRectangle(cRect( posMark - mbig/2, 0, mbig, sml), progressBarColorMarkCurrent);
        else
            progressBarPixmap->DrawRectangle(cRect( posMark - mbig/2, 0, mbig, sml), progressBarColorMark);
    } else {
        // big line
        if( posCurrent > posMark ) {
            progressBarPixmap->DrawRectangle(cRect( posMarkLast, top - big/2, posMark - posMarkLast, big), progressBarColorBarCurFg);
            // draw last marker top
            progressBarPixmap->DrawRectangle(cRect( posMarkLast - mbig/2, 0, mbig, marginItem/2), progressBarColorMark);
        } else {
            progressBarPixmap->DrawRectangle(cRect( posMarkLast, top - big/2, posMark - posMarkLast, big), progressBarColorBarFg);
            if( posCurrent > posMarkLast ) {
                progressBarPixmap->DrawRectangle(cRect( posMarkLast, top - big/2, posCurrent - posMarkLast, big), progressBarColorBarCurFg);
                // draw last marker top
                progressBarPixmap->DrawRectangle(cRect( posMarkLast - mbig/2, 0, mbig, marginItem/2), progressBarColorMark);
            }
        }
        // marker bottom
        if( isCurrent )
            progressBarPixmap->DrawRectangle(cRect( posMark - mbig/2, progressBarHeight - sml, mbig, sml), progressBarColorMarkCurrent);
        else
            progressBarPixmap->DrawRectangle(cRect( posMark - mbig/2, progressBarHeight - sml, mbig, sml), progressBarColorMark);
    }

    if( posCurrent == posMarkLast && posMarkLast != 0 )
        progressBarPixmap->DrawRectangle(cRect( posMarkLast - sml, 0, sml*2, progressBarHeight), progressBarColorMarkCurrent);
    else if( posMarkLast != 0 )
        progressBarPixmap->DrawRectangle(cRect( posMarkLast - sml/2, 0, sml, progressBarHeight), progressBarColorMark);

}

void cFlatBaseRender::ScrollbarDraw(cPixmap *Pixmap, int Left, int Top, int Height, int Total, int Offset, int Shown, bool CanScrollUp, bool CanScrollDown) {
    if( !Pixmap )
        return;

    if (Total > 0 && Total > Shown) {
        int scrollHeight = max(int((Height) * double(Shown) / Total + 0.5), 5);
        int scrollTop = min(int(Top + (Height) * double(Offset) / Total + 0.5), Top + Height - scrollHeight);

        Pixmap->Fill(clrTransparent);
        Pixmap->DrawRectangle(cRect(Left, Top, scrollBarWidth, Height), Theme.Color(clrScrollbarBg));

        if( scrollBarWidth <= 10 )
            Pixmap->DrawRectangle(cRect(Left, Top, 2, Height), Theme.Color(clrScrollbarFg));
        else if( scrollBarWidth <= 20 )
            Pixmap->DrawRectangle(cRect(Left, Top, 4, Height), Theme.Color(clrScrollbarFg));
        else
            Pixmap->DrawRectangle(cRect(Left, Top, 6, Height), Theme.Color(clrScrollbarFg));
        Pixmap->DrawRectangle(cRect(Left, scrollTop, scrollBarWidth, scrollHeight), Theme.Color(clrScrollbarBarFg));
    }
}

int cFlatBaseRender::ScrollBarWidth(void) {
    return scrollBarWidth;
}

void cFlatBaseRender::DecorBorderClear(int Left, int Top, int Width, int Height, int Size) {
    int LeftDecor = Left - Size;
    int TopDecor = Top - Size;
    int WidthDecor = Width + Size*2;
    int HeightDecor = Height + Size*2;
    int BottomDecor = Height + Size;

    if( decorPixmap ) {
        // top
        decorPixmap->DrawRectangle(cRect( LeftDecor, TopDecor, WidthDecor, Size), clrTransparent);
        // right
        decorPixmap->DrawRectangle(cRect( LeftDecor + Size + Width, TopDecor, Size, HeightDecor), clrTransparent);
        // bottom
        decorPixmap->DrawRectangle(cRect( LeftDecor, TopDecor + BottomDecor, WidthDecor, Size), clrTransparent);
        // left
        decorPixmap->DrawRectangle(cRect( LeftDecor, TopDecor, Size, HeightDecor), clrTransparent);
    }
}

void cFlatBaseRender::DecorBorderClearByFrom(int From) {
    std::list<sDecorBorder>::iterator it;
    for( it = Borders.begin(); it != Borders.end(); ) {
        if( (*it).From == From ) {
            DecorBorderClear((*it).Left, (*it).Top, (*it).Width, (*it).Height, (*it).Size);
            it = Borders.erase(it);
        } else
            ++it;
    }
}

void cFlatBaseRender::DecorBorderRedrawAll(void) {
    std::list<sDecorBorder>::iterator it;
    for( it = Borders.begin(); it != Borders.end(); it++) {
        DecorBorderDraw((*it).Left, (*it).Top, (*it).Width, (*it).Height, (*it).Size, (*it).Type, (*it).ColorFg, (*it).ColorBg, (*it).From, false);
    }
}

void cFlatBaseRender::DecorBorderClearAll(void) {
    if( decorPixmap )
        decorPixmap->Fill(clrTransparent);
}

void cFlatBaseRender::DecorBorderDraw(int Left, int Top, int Width, int Height, int Size, int Type, tColor ColorFg, tColor ColorBg, int From, bool Store) {
    if( Size == 0 || Type <= 0 )
        return;

    if( Store ) {
        sDecorBorder f;
        f.Left = Left;
        f.Top = Top;
        f.Width = Width;
        f.Height = Height;
        f.Size = Size;
        f.Type = Type;
        f.ColorFg = ColorFg;
        f.ColorBg = ColorBg;
        f.From = From;

        Borders.push_back(f);
    }

    int LeftDecor = Left - Size;
    int TopDecor = Top - Size;
    int WidthDecor = Width + Size*2;
    int HeightDecor = Height + Size*2;
    int BottomDecor = Height + Size;

    if( !decorPixmap ) {
        decorPixmap = osd->CreatePixmap(4, cRect(0, 0, cOsd::OsdWidth(), cOsd::OsdHeight()));
        decorPixmap->Fill(clrTransparent);
    }

    switch( Type ) {
        case 1: // rect
            // top
            decorPixmap->DrawRectangle(cRect( LeftDecor, TopDecor, WidthDecor, Size), ColorBg);
            // right
            decorPixmap->DrawRectangle(cRect( LeftDecor + Size + Width, TopDecor, Size, HeightDecor), ColorBg);
            // bottom
            decorPixmap->DrawRectangle(cRect( LeftDecor, TopDecor + BottomDecor, WidthDecor, Size), ColorBg);
            // left
            decorPixmap->DrawRectangle(cRect( LeftDecor, TopDecor, Size, HeightDecor), ColorBg);
            break;
        case 2: // round
            // top
            decorPixmap->DrawRectangle(cRect( LeftDecor + Size, TopDecor, Width, Size), ColorBg);
            // right
            decorPixmap->DrawRectangle(cRect( LeftDecor + Size + Width, TopDecor + Size, Size, Height), ColorBg);
            // bottom
            decorPixmap->DrawRectangle(cRect( LeftDecor + Size, TopDecor + BottomDecor, Width, Size), ColorBg);
            // left
            decorPixmap->DrawRectangle(cRect( LeftDecor, TopDecor + Size, Size, Height), ColorBg);

            // top,left corner
            decorPixmap->DrawEllipse(cRect( LeftDecor, TopDecor, Size, Size), ColorBg, 2);
            // top,right corner
            decorPixmap->DrawEllipse(cRect( LeftDecor + Size + Width, TopDecor, Size, Size), ColorBg, 1);
            // bottom,left corner
            decorPixmap->DrawEllipse(cRect( LeftDecor, TopDecor + BottomDecor, Size, Size), ColorBg, 3);
            // bottom,right corner
            decorPixmap->DrawEllipse(cRect( LeftDecor + Size + Width, TopDecor + BottomDecor, Size, Size), ColorBg, 4);
            break;
        case 3: // invert round
            // top
            decorPixmap->DrawRectangle(cRect( LeftDecor + Size, TopDecor, Width, Size), ColorBg);
            // right
            decorPixmap->DrawRectangle(cRect( LeftDecor+ Size + Width, TopDecor + Size, Size, Height), ColorBg);
            // bottom
            decorPixmap->DrawRectangle(cRect( LeftDecor + Size, TopDecor + BottomDecor, Width, Size), ColorBg);
            // left
            decorPixmap->DrawRectangle(cRect( LeftDecor, TopDecor + Size, Size, Height), ColorBg);

            // top,left corner
            decorPixmap->DrawEllipse(cRect( LeftDecor, TopDecor, Size, Size), ColorBg, -4);
            // top,right corner
            decorPixmap->DrawEllipse(cRect( LeftDecor + Size + Width, TopDecor, Size, Size), ColorBg, -3);
            // bottom,left corner
            decorPixmap->DrawEllipse(cRect( LeftDecor, TopDecor + BottomDecor, Size, Size), ColorBg, -1);
            // bottom,right corner
            decorPixmap->DrawEllipse(cRect( LeftDecor + Size + Width, TopDecor + BottomDecor, Size, Size), ColorBg, -2);
            break;
        case 4: // rect + alpha blend
            // top
            DecorDrawGlowRectHor(decorPixmap, LeftDecor + Size, TopDecor, WidthDecor - Size*2, Size, ColorBg);
            // bottom
            DecorDrawGlowRectHor(decorPixmap, LeftDecor + Size, TopDecor + BottomDecor, WidthDecor - Size*2, -1*Size, ColorBg);
            // left
            DecorDrawGlowRectVer(decorPixmap, LeftDecor, TopDecor + Size, Size, HeightDecor - Size*2, ColorBg);
            // right
            DecorDrawGlowRectVer(decorPixmap, LeftDecor + Size + Width, TopDecor + Size, -1*Size, HeightDecor - Size*2, ColorBg);

            DecorDrawGlowRectTL(decorPixmap, LeftDecor, TopDecor, Size, Size, ColorBg);
            DecorDrawGlowRectTR(decorPixmap, LeftDecor + Size + Width, TopDecor, Size, Size, ColorBg);
            DecorDrawGlowRectBL(decorPixmap, LeftDecor, TopDecor + Size + Height, Size, Size, ColorBg);
            DecorDrawGlowRectBR(decorPixmap, LeftDecor + Size + Width, TopDecor + Size + Height, Size, Size, ColorBg);
            break;
        case 5: // round + alpha blend
            // top
            DecorDrawGlowRectHor(decorPixmap, LeftDecor + Size, TopDecor, WidthDecor - Size*2, Size, ColorBg);
            // bottom
            DecorDrawGlowRectHor(decorPixmap, LeftDecor + Size, TopDecor + BottomDecor, WidthDecor - Size*2, -1*Size, ColorBg);
            // left
            DecorDrawGlowRectVer(decorPixmap, LeftDecor, TopDecor + Size, Size, HeightDecor - Size*2, ColorBg);
            // right
            DecorDrawGlowRectVer(decorPixmap, LeftDecor + Size + Width, TopDecor + Size, -1*Size, HeightDecor - Size*2, ColorBg);

            DecorDrawGlowEllipseTL(decorPixmap, LeftDecor, TopDecor, Size, Size, ColorBg, 2);
            DecorDrawGlowEllipseTR(decorPixmap, LeftDecor + Size + Width, TopDecor, Size, Size, ColorBg, 1);
            DecorDrawGlowEllipseBL(decorPixmap, LeftDecor, TopDecor + Size + Height, Size, Size, ColorBg, 3);
            DecorDrawGlowEllipseBR(decorPixmap, LeftDecor + Size + Width, TopDecor + Size + Height, Size, Size, ColorBg, 4);
            break;
        case 6: // invert round + alpha blend
            // top
            DecorDrawGlowRectHor(decorPixmap, LeftDecor + Size, TopDecor, WidthDecor - Size*2, Size, ColorBg);
            // bottom
            DecorDrawGlowRectHor(decorPixmap, LeftDecor + Size, TopDecor + BottomDecor, WidthDecor - Size*2, -1*Size, ColorBg);
            // left
            DecorDrawGlowRectVer(decorPixmap, LeftDecor, TopDecor + Size, Size, HeightDecor - Size*2, ColorBg);
            // right
            DecorDrawGlowRectVer(decorPixmap, LeftDecor + Size + Width, TopDecor + Size, -1*Size, HeightDecor - Size*2, ColorBg);

            DecorDrawGlowEllipseTL(decorPixmap, LeftDecor, TopDecor, Size, Size, ColorBg, -4);
            DecorDrawGlowEllipseTR(decorPixmap, LeftDecor + Size + Width, TopDecor, Size, Size, ColorBg, -3);
            DecorDrawGlowEllipseBL(decorPixmap, LeftDecor, TopDecor + Size + Height, Size, Size, ColorBg, -1);
            DecorDrawGlowEllipseBR(decorPixmap, LeftDecor + Size + Width, TopDecor + Size + Height, Size, Size, ColorBg, -2);
            break;
    }
}

/*
tColor cFlatBaseRender::Multiply(tColor Color, uint8_t Alpha)
{
  tColor RB = (Color & 0x00FF00FF) * Alpha;
  RB = ((RB + ((RB >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
  tColor AG = ((Color >> 8) & 0x00FF00FF) * Alpha;
  AG = ((AG + ((AG >> 8) & 0x00FF00FF) + 0x00800080)) & 0xFF00FF00;
  return AG | RB;
}
*/

tColor cFlatBaseRender::SetAlpha(tColor Color, double am)
{
    uint8_t A = (Color & 0xFF000000) >> 24;
    uint8_t R = (Color & 0x00FF0000) >> 16;
    uint8_t G = (Color & 0x0000FF00) >> 8;
    uint8_t B = (Color & 0x000000FF);

    A = A * am;
    return ArgbToColor(A, R, G, B);
}


void cFlatBaseRender::DecorDrawGlowRectHor(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg) {
    double Alpha;
    if( Height < 0 ) {
        Height *= -1;
        for(int i = Height, j = 0; i >= 0; i--, j++) {
            Alpha = 255.0 / Height * j;
            tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
            pixmap->DrawRectangle(cRect( Left, Top + i, Width, 1), col);
        }
    } else {
        for(int i = 0; i < Height; i++) {
            Alpha = 255.0 / Height * i;
            tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
            pixmap->DrawRectangle(cRect( Left, Top + i, Width, 1), col);
        }
    }
}

void cFlatBaseRender::DecorDrawGlowRectVer(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg) {
    double Alpha;
    if( Width < 0 ) {
        Width *= -1;
        for(int i = Width, j = 0; i >= 0; i--, j++) {
            Alpha = 255.0 / Width * j;
            tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
            pixmap->DrawRectangle(cRect( Left + i, Top, 1, Height), col);
        }
    } else {
        for(int i = 0; i < Width; i++) {
            Alpha = 255.0 / Width * i;
            tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
            pixmap->DrawRectangle(cRect( Left + i, Top, 1, Height), col);
        }
    }
}

void cFlatBaseRender::DecorDrawGlowRectTL(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg) {
    double Alpha;

    for(int i = 0; i < Width; i++) {
        Alpha = 255.0 / Width * i;
        tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
        pixmap->DrawRectangle(cRect( Left + i, Top + i, Width-i, Height-i), col);
    }
}

void cFlatBaseRender::DecorDrawGlowRectTR(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg) {
    double Alpha;

    for(int i = 0, j = Width; i < Width; i++, j--) {
        Alpha = 255.0 / Width * i;
        tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
        pixmap->DrawRectangle(cRect( Left, Top + Height-j, j, j), col);
    }
}

void cFlatBaseRender::DecorDrawGlowRectBL(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg) {
    double Alpha;

    for(int i = 0, j = Width; i < Width; i++, j--) {
        Alpha = 255.0 / Width * i;
        tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
        pixmap->DrawRectangle(cRect( Left + Width - j, Top, j, j), col);
    }
}

void cFlatBaseRender::DecorDrawGlowRectBR(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg) {
    double Alpha;

    for(int i = 0, j = Width; i < Width; i++, j--) {
        Alpha = 255 / Width * i;
        tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
        pixmap->DrawRectangle(cRect( Left, Top, j, j), col);
    }
}

void cFlatBaseRender::DecorDrawGlowEllipseTL(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg, int type) {
    double Alpha;

    for(int i = 0, j = Width; i < Width; i++, j--) {
        if( VDRVERSNUM < 20002 && j == 1 ) // in VDR Version < 2.0.2 osd breaks if width & height == 1
            continue;
        Alpha = 255 / Width * i;
        tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
        pixmap->DrawEllipse(cRect( Left + i, Top + i, j, j), col, type);
    }
}

void cFlatBaseRender::DecorDrawGlowEllipseTR(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg, int type) {
    double Alpha;

    for(int i = 0, j = Width; i < Width; i++, j--) {
        if( VDRVERSNUM < 20002 && j == 1 ) // in VDR Version < 2.0.2 osd breaks if width & height == 1
            continue;
        Alpha = 255 / Width * i;
        tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
        pixmap->DrawEllipse(cRect( Left, Top + Height-j, j, j), col, type);
    }
}

void cFlatBaseRender::DecorDrawGlowEllipseBL(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg, int type) {
    double Alpha;

    for(int i = 0, j = Width; i < Width; i++, j--) {
        if( VDRVERSNUM < 20002 && j == 1 ) // in VDR Version < 2.0.2 osd breaks if width & height == 1
            continue;
        Alpha = 255 / Width * i;
        tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
        pixmap->DrawEllipse(cRect( Left + Width - j, Top, j, j), col, type);
    }
}

void cFlatBaseRender::DecorDrawGlowEllipseBR(cPixmap *pixmap, int Left, int Top, int Width, int Height, tColor ColorBg, int type) {
    double Alpha;

    for(int i = 0, j = Width; i < Width; i++, j--) {
        if( VDRVERSNUM < 20002 && j == 1 ) // in VDR Version < 2.0.2 osd breaks if width & height == 1
            continue;
        Alpha = 255 / Width * i;
        tColor col = SetAlpha(ColorBg, 100.0/255.0*Alpha/100.0);
        pixmap->DrawEllipse(cRect( Left, Top, j, j), col, type);
    }
}

