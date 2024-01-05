/*
 * Skin flatPlus: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */
#pragma once

#define X_DISPLAY_MISSING

#include <vdr/osd.h>
#include <vdr/skins.h>

#include <Magick++.h>
#include "./imagemagickwrapper.h"

using namespace Magick;

class cImageLoader : public cImageMagickWrapper {
 public:
    cImageLoader();
    ~cImageLoader();

    cImage* LoadLogo(const char *logo, int width, int height);
    cImage* LoadIcon(const char *cIcon, int width, int height);
    cImage* LoadFile(const char *cFile, int width, int height);
    bool FileExits(const std::string &name);
    bool SearchRecordingPoster(cString RecPath, cString &found);  // NOLINT
 private:
    // int epgImageWidthLarge, epgImageHeightLarge;  // Unused?
    // int epgImageWidth, epgImageHeight;
    cString m_LogoExtension{"png"};

    void ToLowerCase(std::string &str);
};
