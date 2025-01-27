/*
 * Skin flatPlus: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */
#pragma once

#include <ctype.h>
#include <vdr/menu.h>
#include <vdr/tools.h>

#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <string>

#include "./baserender.h"
#include "./complexcontent.h"

class cFlatDisplayMenu : public cFlatBaseRender, public cSkinDisplayMenu {
 private:
        cPixmap *MenuPixmap {nullptr};
        cPixmap *MenuIconsPixmap {nullptr};
        cPixmap *MenuIconsBgPixmap {nullptr};   // Background for icons/logos
        cPixmap *MenuIconsOvlPixmap {nullptr};  // Overlay for icons/logos

        int m_MenuTop {0}, m_MenuWidth {0};
        int m_MenuItemWidth {0};
        int m_MenuItemLastHeight {0};
        bool m_MenuFullOsdIsDrawn = false;

        eMenuCategory m_MenuCategory;

        int m_FontAscender {0};  // Top of capital letter
        // int m_VideoDiskUsageState;  // Also in cFlatBaseRender

        int m_LastTimerCount {0}, m_LastTimerActiveCount {0};
        cString m_LastTitle{""};

        int m_chLeft {0}, m_chTop {0}, m_chWidth {0}, m_chHeight {0};
        cPixmap *ContentHeadPixmap {nullptr};
        cPixmap *ContentHeadIconsPixmap {nullptr};

        int m_cLeft {0}, m_cTop {0}, m_cWidth {0}, m_cHeight {0};

        cPixmap *ScrollbarPixmap {nullptr};
        int m_ScrollBarTop {0};
        int m_ScrollBarWidth {0}, m_ScrollBarHeight {0};  //? 'm_ScrollBarWidth' also in cFlatBaseRender

        int m_ItemHeight {0}, m_ItemChannelHeight {0}, m_ItemTimerHeight {0};
        int m_ItemEventHeight {0}, m_ItemRecordingHeight {0};

        std::list<sDecorBorder> ItemsBorder;
        sDecorBorder EventBorder, RecordingBorder, TextBorder;

        bool m_IsScrolling = false;
        bool m_IsGroup = false;
        bool m_ShowEvent = false;
        bool m_ShowRecording = false;
        bool m_ShowText = false;

        cComplexContent ComplexContent;

        // Content for Widgets
        cComplexContent ContentWidget;

        // TextScroller
        cTextScrollers MenuItemScroller;

        cString m_ItemEventLastChannelName{""};

        std::string m_RecFolder{""}, m_LastRecFolder{""};
        int m_LastItemRecordingLevel {0};

        // Icons
        cImage *IconTimerFull {nullptr};
        // cImage *IconTimerPartial;
        cImage *IconArrowTurn {nullptr};
        cImage *IconRec {nullptr};
        // cImage *iconVps;
        // cImage *iconNew;
        // Icons

        void ItemBorderInsertUnique(sDecorBorder ib);
        void ItemBorderDrawAllWithScrollbar(void);
        void ItemBorderDrawAllWithoutScrollbar(void);
        void ItemBorderClear(void);

        //! Fix Static/global string variables are not permitted.  cpplint(warning:runtime/string)
        // static std::string items[16];
        const std::string items[16] = {"Schedule", "Channels",      "Timers",  "Recordings", "Setup", "Commands",
                                       "OSD",      "EPG",           "DVB",     "LNB",        "CAM",   "Recording",
                                       "Replay",   "Miscellaneous", "Plugins", "Restart"};
        std::string MainMenuText(std::string Text);
        cString GetIconName(std::string element);

        std::string GetRecordingName(const cRecording *Recording, int Level, bool IsFolder);
        // std::string XmlSubstring(std::string source, const char* StrStart, const char* StrEnd);  // Moved to flat.h

        bool IsRecordingOld(const cRecording *Recording, int Level);

        const char *GetGenreIcon(uchar genre);
        void InsertGenreInfo(const cEvent *Event, cString &Text);  // NOLINT
        void InsertGenreInfo(const cEvent *Event, cString &Text, std::list<std::string> &GenreIcons);  // NOLINT

        time_t GetLastRecTimeFromFolder(const cRecording *Recording, int Level);

        void DrawScrollbar(int Total, int Offset, int Shown, int Top, int Height, bool CanScrollUp,
                           bool CanScrollDown, bool IsContent = false);
        int ItemsHeight(void);
        bool CheckProgressBar(const char *text);
        void DrawProgressBarFromText(cRect rec, cRect recBg, const char *bar, tColor ColorFg,
                                     tColor ColorBarFg, tColor ColorBg);

        static cBitmap bmCNew, bmCRec, bmCArrowTurn, bmCHD, bmCVPS;
        void DrawItemExtraEvent(const cEvent *Event, cString EmptyText);
        void DrawItemExtraRecording(const cRecording *Recording, cString EmptyText);
        void AddActors(cComplexContent &ComplexContent, std::vector<cString> &ActorsPath,   // NOLINT
                       std::vector<cString> &ActorsName, std::vector<cString> &ActorsRole,  // NOLINT
                       int NumActors);  // Add Actors to complexcontent
        void DrawMainMenuWidgets(void);
        int DrawMainMenuWidgetDVBDevices(int wLeft, int wWidth, int ContentTop);
        int DrawMainMenuWidgetActiveTimers(int wLeft, int wWidth, int ContentTop);
        int DrawMainMenuWidgetLastRecordings(int wLeft, int wWidth, int ContentTop);
        int DrawMainMenuWidgetTimerConflicts(int wLeft, int wWidth, int ContentTop);
        int DrawMainMenuWidgetSystemInformation(int wLeft, int wWidth, int ContentTop);
        int DrawMainMenuWidgetSystemUpdates(int wLeft, int wWidth, int ContentTop);
        int DrawMainMenuWidgetTemperatures(int wLeft, int wWidth, int ContentTop);
        int DrawMainMenuWidgetCommand(int wLeft, int wWidth, int ContentTop);
        int DrawMainMenuWidgetWeather(int wLeft, int wWidth, int ContentTop);

 public:
#ifdef DEPRECATED_SKIN_SETITEMEVENT
    using cSkinDisplayMenu::SetItemEvent;
#endif
        cFlatDisplayMenu(void);
        virtual ~cFlatDisplayMenu();
        virtual void Scroll(bool Up, bool Page);
        virtual int MaxItems(void);
        virtual void Clear(void);

        virtual void SetMenuCategory(eMenuCategory MenuCategory);
        // virtual void SetTabs(int Tab1, int Tab2 = 0, int Tab3 = 0, int Tab4 = 0, int Tab5 = 0);

        virtual void SetTitle(const char *Title);
        virtual void SetButtons(const char *Red, const char *Green = NULL,
                                const char *Yellow = NULL, const char *Blue = NULL);
        virtual void SetMessage(eMessageType Type, const char *Text);
        virtual void SetItem(const char *Text, int Index, bool Current, bool Selectable);

        virtual bool SetItemEvent(const cEvent *Event, int Index, bool Current, bool Selectable,
                                  const cChannel *Channel, bool WithDate, eTimerMatch TimerMatch,
                                  bool TimerActive);
        virtual bool SetItemTimer(const cTimer *Timer, int Index, bool Current, bool Selectable);
        virtual bool SetItemChannel(const cChannel *Channel, int Index, bool Current, bool Selectable,
                                    bool WithProvider);
        virtual bool SetItemRecording(const cRecording *Recording, int Index, bool Current, bool Selectable,
                                      int Level, int Total, int New);

        virtual void SetMenuSortMode(eMenuSortMode MenuSortMode);

        virtual void SetScrollbar(int Total, int Offset);
        virtual void SetEvent(const cEvent *Event);
        virtual void SetRecording(const cRecording *Recording);
        virtual void SetText(const char *Text, bool FixedFont);
        virtual int GetTextAreaWidth(void) const;
        virtual const cFont *GetTextAreaFont(bool FixedFont) const;
        virtual void Flush(void);

        void PreLoadImages(void);
};
