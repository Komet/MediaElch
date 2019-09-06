#pragma once

#include "tv_shows/SeasonNumber.h"

#include <QSize>
#include <QString>
#include <QUrl>

struct Poster
{
    QString id;
    QUrl originalUrl;
    QUrl thumbUrl;
    QSize originalSize;
    QString language;
    QString hint;
    QString aspect; // "aspect" attribute in Kodi NFO files
    SeasonNumber season = SeasonNumber::NoSeason;
};
