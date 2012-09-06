#ifndef GLOBALS_H
#define GLOBALS_H

#include <QDate>
#include <QImage>
#include <QMetaType>
#include <QString>
#include <QUrl>
#include <QVariant>

class Movie;
class TvShowEpisode;

namespace Constants {
    const int MovieFileSearcherProgressMessageId = 10000;
    const int MovieWidgetProgressMessageId       = 10001;
    const int TvShowSearcherProgressMessageId    = 10002;
    const int TvShowWidgetProgressMessageId      = 10003;
    const int TvShowWidgetSaveProgressMessageId  = 10004;
    const int MovieProgressMessageId             = 20000;
    const int TvShowProgressMessageId            = 40000;
    const int EpisodeProgressMessageId           = 60000;
}

namespace TvShowRoles {
    const int Type = Qt::UserRole+1;
    const int ParentId = Qt::UserRole+2;
    const int Id = Qt::UserRole+3;
    const int EpisodeCount = Qt::UserRole+4;
    const int HasChanged = Qt::UserRole+5;
    const int IsNew = Qt::UserRole+6;
}

namespace MediaCenterInterfaces {
    const int XbmcXml    = 1;
    const int XbmcMysql  = 2;
    const int XbmcSqlite = 3;
}

namespace MediaCenterFeatures {
    const int EditTvShowEpisodeCertification = 1;
    const int EditTvShowEpisodeShowTitle     = 2;
    const int EditTvShowEpisodeNetwork       = 3;
    const int HandleMovieSetImages           = 4;
}

enum MainActions {
    ActionSearch, ActionSave, ActionExport
};

enum MainWidgets {
    WidgetMovies, WidgetMovieSets, WidgetTvShows
};

/**
 * @brief The Actor struct
 */
struct Actor {
    QString name;
    QString role;
    QString thumb;
    QImage image;
    bool imageHasChanged;
};
Q_DECLARE_METATYPE(Actor*);
Q_DECLARE_METATYPE(Movie*);
Q_DECLARE_METATYPE(QString*);

/**
 * @brief The ScraperSearchResult struct
 */
struct ScraperSearchResult {
    QString id;
    QString name;
    QDate released;
};

/**
 * @brief The Poster struct
 */
struct Poster {
    QString id;
    QUrl originalUrl;
    QUrl thumbUrl;
    QSize originalSize;
};

enum ImageType {
    TypePoster, TypeBackdrop, TypeBanner, TypeActor, TypeSeasonPoster, TypeShowThumbnail
};

enum TvShowType {
    TypeTvShow, TypeEpisode, TypeSeason
};

/**
 * @brief The SettingsDir struct
 */
struct SettingsDir {
    QString path;
    QString mediaCenterPath;
    bool separateFolders;
};

namespace MovieImageDialogType {
    const int MoviePoster    = 1;
    const int MovieBackdrop  = 2;
    const int TvShowPoster   = 3;
    const int TvShowBackdrop = 4;
    const int TvShowThumb    = 5;
    const int TvShowSeason   = 6;
    const int TvShowBanner   = 7;
}

namespace MovieScraperInfos {
    const int Title         = 1;
    const int Tagline       = 2;
    const int Rating        = 3;
    const int Released      = 4;
    const int Runtime       = 5;
    const int Certification = 6;
    const int Trailer       = 7;
    const int Overview      = 8;
    const int Poster        = 9;
    const int Backdrop      = 10;
    const int Actors        = 11;
    const int Genres        = 12;
    const int Studios       = 13;
    const int Countries     = 14;
}

#endif // GLOBALS_H
