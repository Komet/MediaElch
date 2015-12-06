#ifndef GLOBALS_H
#define GLOBALS_H

#include <QDate>
#include <QDebug>
#include <QImage>
#include <QMap>
#include <QMetaType>
#include <QString>
#include <QUrl>
#include <QVariant>

namespace Constants {
    const int MovieFileSearcherProgressMessageId   = 10000;
    const int MovieWidgetProgressMessageId         = 10001;
    const int TvShowSearcherProgressMessageId      = 10002;
    const int TvShowWidgetProgressMessageId        = 10003;
    const int TvShowWidgetSaveProgressMessageId    = 10004;
    const int ConcertFileSearcherProgressMessageId = 10005;
    const int TvShowUpdaterProgressMessageId       = 10006;
    const int MusicFileSearcherProgressMessageId   = 10007;
    const int MovieProgressMessageId               = 20000;
    const int TvShowProgressMessageId              = 40000;
    const int EpisodeProgressMessageId             = 60000;
    const int ConcertProgressMessageId             = 80000;
    const int MusicArtistProgressMessageId         = 100000;
    const int MusicAlbumProgressMessageId          = 200000;
    const int MusicWidgetSaveProgressMessageId     = 300000;
}

namespace TvShowRoles {
    const int Type = Qt::UserRole+1;
    const int ParentId = Qt::UserRole+2;
    const int Id = Qt::UserRole+3;
    const int EpisodeCount = Qt::UserRole+4;
    const int HasChanged = Qt::UserRole+5;
    const int IsNew = Qt::UserRole+6;
    const int SyncNeeded = Qt::UserRole+7;
    const int HasBanner = Qt::UserRole+8;
    const int HasPoster = Qt::UserRole+9;
    const int HasFanart = Qt::UserRole+10;
    const int HasExtraFanart = Qt::UserRole+11;
    const int HasLogo = Qt::UserRole+12;
    const int HasThumb = Qt::UserRole+13;
    const int HasClearArt = Qt::UserRole+14;
    const int HasCharacterArt = Qt::UserRole+15;
    const int MissingEpisodes = Qt::UserRole+16;
    const int LogoPath = Qt::UserRole+17;
    const int FilePath = Qt::UserRole+18;
    const int SelectionForeground = Qt::UserRole+19;
    const int HasDummyEpisodes = Qt::UserRole+20;
}

namespace MusicRoles {
    const int Type = Qt::UserRole+1;
    const int IsNew = Qt::UserRole+2;
    const int HasChanged = Qt::UserRole+3;
    const int NumOfAlbums = Qt::UserRole+4;
    const int SelectionForeground = Qt::UserRole+5;
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
    const int EditConcertRating              = 5;
    const int EditConcertTagline             = 6;
    const int EditConcertCertification       = 7;
    const int EditConcertTrailer             = 8;
    const int EditConcertWatched             = 9;
}

enum MainActions {
    ActionSearch, ActionSave, ActionSaveAll, ActionFilterWidget, ActionRename, ActionExport
};

enum MainWidgets {
    WidgetMovies, WidgetMovieSets, WidgetTvShows, WidgetConcerts, WidgetMusic, WidgetGenres, WidgetCertifications, WidgetDownloads
};

/**
 * @brief The Actor struct
 */
struct Actor {
    QString name;
    QString role;
    QString thumb;
    QByteArray image;
    bool imageHasChanged;
    QString id;
};
Q_DECLARE_METATYPE(Actor*)
Q_DECLARE_METATYPE(QString*)
Q_DECLARE_METATYPE(QList<int>)

struct DiscographyAlbum {
    QString title;
    QString year;
};
Q_DECLARE_METATYPE(DiscographyAlbum*)

/**
 * @brief The ScraperSearchResult struct
 */
struct ScraperSearchResult {
    QString id;
    QString id2;
    QString name;
    QDate released;
};

struct TrailerResult {
    QUrl preview;
    QString name;
    QString language;
    QUrl trailerUrl;
    QImage previewImage;
    bool previewImageLoaded;
};

/**
 * @brief The Poster struct
 */
struct Poster {
    QString id;
    QUrl originalUrl;
    QUrl thumbUrl;
    QSize originalSize;
    QString language;
    QString hint;
    int season;
};

enum TvShowType {
    TypeTvShow, TypeEpisode, TypeSeason
};

enum MusicType {
    TypeArtist, TypeAlbum
};

enum ItemType {
    ItemMovie, ItemTvShow, ItemTvShowEpisode, ItemConcert, ItemArtist, ItemAlbum
};

enum DiscType {
    DiscSingle, DiscBluRay, DiscDvd
};

enum MovieSetArtworkType {
    MovieSetArtworkSingleSetFolder, MovieSetArtworkSingleArtworkFolder
};

/**
 * @brief The SettingsDir struct
 */
struct SettingsDir {
    QString path;
    bool separateFolders;
    bool autoReload;
};

enum SettingsDirType {
    DirTypeMovies, DirTypeTvShows, DirTypeConcerts, DirTypeDownloads, DirTypeMusic
};

enum ComboDelegateType {
    ComboDelegateGenres, ComboDelegateStudios, ComboDelegateCountries, ComboDelegateWriters, ComboDelegateDirectors
};

namespace ImageType {
    const int MoviePoster          = 1;
    const int MovieBackdrop        = 2;
    const int TvShowPoster         = 3;
    const int TvShowBackdrop       = 4;
    const int TvShowEpisodeThumb   = 5;
    const int TvShowBanner         = 7;
    const int ConcertPoster        = 8;
    const int ConcertBackdrop      = 9;
    const int MovieLogo            = 10;
    const int MovieClearArt        = 11;
    const int MovieCdArt           = 12;
    const int ConcertLogo          = 13;
    const int ConcertClearArt      = 14;
    const int ConcertCdArt         = 15;
    const int TvShowClearArt       = 16;
    const int TvShowLogos          = 17;
    const int TvShowCharacterArt   = 18;
    const int TvShowSeasonBackdrop = 19;
    const int TvShowSeasonBanner   = 20;
    const int MovieBanner          = 21;
    const int MovieThumb           = 22;
    const int TvShowThumb          = 23;
    const int TvShowSeasonThumb    = 24;
    const int TvShowSeasonPoster   = 25;
    const int Actor                = 26;
    const int MovieExtraFanart     = 27;
    const int MovieSetPoster       = 28;
    const int MovieSetBackdrop     = 29;
    const int ConcertExtraFanart   = 30;
    const int TvShowExtraFanart    = 31;
    const int ArtistThumb          = 32;
    const int ArtistFanart         = 33;
    const int ArtistLogo           = 34;
    const int AlbumThumb           = 35;
    const int AlbumCdArt           = 36;
    const int ArtistExtraFanart    = 37;
    const int AlbumBooklet         = 38;
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
    const int Writer        = 15;
    const int Director      = 16;
    const int Tags          = 18;
    const int ExtraFanarts  = 19;
    const int Set           = 20;
    const int Logo          = 21;
    const int CdArt         = 22;
    const int ClearArt      = 23;
    const int Banner        = 24;
    const int Thumb         = 25;

    const int First         = 1;
    const int Last          = 25;
}

namespace TvShowScraperInfos {
    const int Actors         = 1;
    const int Banner         = 2;
    const int Certification  = 3;
    const int Director       = 4;
    const int Fanart         = 5;
    const int FirstAired     = 6;
    const int Genres         = 7;
    const int Network        = 8;
    const int Overview       = 9;
    const int Poster         = 10;
    const int Rating         = 11;
    const int SeasonPoster   = 13;
    const int Thumbnail      = 14;
    const int Title          = 15;
    const int Writer         = 16;
    const int Tags           = 17;
    const int ExtraArts      = 18;
    const int SeasonBackdrop = 19;
    const int SeasonBanner   = 20;
    const int ExtraFanarts   = 21;
    const int Thumb          = 22;
    const int SeasonThumb    = 23;
    const int Runtime        = 24;
    const int Status         = 25;
}

namespace ConcertScraperInfos {
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
    const int Genres        = 11;
    const int ExtraArts     = 12;
    const int Tags          = 13;
    const int ExtraFanarts  = 14;
}

namespace MusicScraperInfos {
    const int Name         = 1;
    const int Genres       = 2;
    const int Styles       = 3;
    const int Moods        = 4;
    const int YearsActive  = 5;
    const int Formed       = 6;
    const int Born         = 7;
    const int Died         = 8;
    const int Disbanded    = 9;
    const int Biography    = 10;
    const int Thumb        = 11;
    const int Fanart       = 12;
    const int Logo         = 13;
    const int Title        = 14;
    const int Artist       = 15;
    const int Review       = 16;
    const int ReleaseDate  = 17;
    const int Label        = 18;
    const int Rating       = 19;
    const int Year         = 20;
    const int CdArt        = 21;
    const int Cover        = 22;
    const int ExtraFanarts = 23;
    const int Discography  = 24;
}

// The filter numbers have to unique for MovieFilters, TvShowFilters and ConcertFilters
namespace MovieFilters {
    const int Released            = 1;
    const int Certification       = 2;
    const int Trailer             = 3;
    const int Poster              = 4;
    const int Backdrop            = 5;
    const int Watched             = 6;
    const int Genres              = 7;
    const int Title               = 8;
    const int Logo                = 9;
    const int ClearArt            = 10;
    const int CdArt               = 11;
    const int StreamDetails       = 12;
    const int ExtraFanarts        = 13;
    const int Actors              = 14;
    const int LocalTrailer        = 15;
    const int Country             = 16;
    const int Studio              = 17;
    const int Path                = 18;
    const int Director            = 19;
    const int Tags                = 20;
    const int Quality             = 21;
    const int Banner              = 22;
    const int Thumb               = 23;
    const int ImdbId              = 24;
    const int Set                 = 25;
    const int Rating              = 28;
    const int Label               = 29;
    const int AudioChannels       = 30;
    const int AudioQuality        = 31;
    const int HasSubtitle         = 33;
    const int HasExternalSubtitle = 34;
}

namespace TvShowFilters {
    const int Title         = 26;
}

namespace ConcertFilters {
    const int Title         = 27;
}

namespace MusicFilters {
    const int Title         = 32;
}

enum SortBy {
    SortByName, SortBySeen, SortByAdded, SortByYear, SortByNew
};

namespace DataFileType {
    const int MovieNfo             = 1;
    const int MoviePoster          = 2;
    const int MovieBackdrop        = 3;
    const int MovieLogo            = 4;
    const int MovieClearArt        = 5;
    const int MovieCdArt           = 6;
    const int ConcertNfo           = 7;
    const int ConcertPoster        = 8;
    const int ConcertBackdrop      = 9;
    const int ConcertLogo          = 10;
    const int ConcertClearArt      = 11;
    const int ConcertCdArt         = 12;
    const int TvShowNfo            = 13;
    const int TvShowPoster         = 14;
    const int TvShowBackdrop       = 15;
    const int TvShowBanner         = 16;
    const int TvShowSeasonPoster   = 17;
    const int TvShowLogo           = 18;
    const int TvShowClearArt       = 19;
    const int TvShowCharacterArt   = 20;
    const int TvShowEpisodeNfo     = 21;
    const int TvShowEpisodeThumb   = 22;
    const int TvShowSeasonBackdrop = 23;
    const int TvShowSeasonBanner   = 24;
    const int MovieBanner          = 25;
    const int MovieThumb           = 26;
    const int TvShowSeasonThumb    = 27;
    const int TvShowThumb          = 28;
    const int MovieSetPoster       = 29;
    const int MovieSetBackdrop     = 30;
    const int ArtistNfo            = 31;
    const int AlbumNfo             = 32;
    const int ArtistThumb          = 33;
    const int ArtistFanart         = 34;
    const int ArtistLogo           = 35;
    const int AlbumThumb           = 36;
    const int AlbumCdArt           = 37;
}

enum TvShowUpdateType {
    UpdateShow, UpdateShowAndAllEpisodes, UpdateShowAndNewEpisodes, UpdateNewEpisodes, UpdateAllEpisodes
};

enum ScraperData {
    DataInfos, DataCasts, DataTrailers, DataImages, DataReleases
};

struct ExtraFanart {
    QByteArray image;
    QString path;
};

enum MediaStatusColumns {
    MediaStatusId,
    MediaStatusStreamDetails,
    MediaStatusTrailer,
    MediaStatusLocalTrailer,
    MediaStatusPoster,
    MediaStatusFanart,
    MediaStatusExtraArts,
    MediaStatusExtraFanarts,
    MediaStatusActors,
    MediaStatusUnknown,

    MediaStatusFirst = MediaStatusId,
    MediaStatusLast = MediaStatusActors
};

namespace Labels {
    const int NO_LABEL = 0;
    const int RED    = 1;
    const int ORANGE = 2;
    const int YELLOW = 3;
    const int GREEN  = 4;
    const int BLUE   = 5;
    const int PURPLE = 6;
    const int GREY   = 7;
}

#endif // GLOBALS_H
