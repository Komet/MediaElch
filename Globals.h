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
}

namespace TvShowRoles {
    const int Type = Qt::UserRole+1;
    const int ParentId = Qt::UserRole+2;
    const int Id = Qt::UserRole+3;
    const int EpisodeCount = Qt::UserRole+4;
    const int HasChanged = Qt::UserRole+5;
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
}

enum MainActions {
    ActionSearch, ActionSave, ActionRefresh, ActionExport
};

enum MainWidgets {
    WidgetMovies, WidgetTvShows
};

struct Actor {
    QString name;
    QString role;
    QString thumb;
    QImage image;
};
Q_DECLARE_METATYPE(Actor*);
Q_DECLARE_METATYPE(QString*);

struct ScraperSearchResult {
    QString id;
    QString name;
    QDate released;
};

struct Poster {
    QString id;
    QUrl originalUrl;
    QUrl thumbUrl;
};

enum ImageType {
    TypePoster, TypeBackdrop, TypeActor, TypeSeasonPoster, TypeShowThumbnail
};

enum TvShowType {
    TypeTvShow, TypeEpisode
};

/*
struct DownloadManagerElement {
    ImageType imageType;
    QUrl url;
    QImage image;
    qint64 bytesReceived;
    qint64 bytesTotal;
    Actor *actor;
    TvShowEpisode *episode;
    Movie *movie;
    int season;
};
*/
struct SettingsDir {
    QString path;
    QString mediaCenterPath;
};

#endif // GLOBALS_H
