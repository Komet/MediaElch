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

// clang-format off
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
    const int MovieDuplicatesProgressMessageId     = 90000;
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
// clang-format on

enum class MainActions
{
    Search,
    Save,
    SaveAll,
    FilterWidget,
    Rename,
    Export
};

enum class MainWidgets
{
    Movies,
    MovieSets,
    TvShows,
    Concerts,
    Music,
    Genres,
    Certifications,
    Downloads
};

/**
 * @brief The Actor struct
 */
struct Actor
{
    QString name;
    QString role;
    QString thumb;
    QByteArray image;
    bool imageHasChanged{false};
    QString id;
};
Q_DECLARE_METATYPE(Actor *)
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(QList<int>)

struct DiscographyAlbum
{
    QString title;
    QString year;
};
Q_DECLARE_METATYPE(DiscographyAlbum *)

/**
 * @brief The ScraperSearchResult struct
 */
struct ScraperSearchResult
{
    QString id;
    QString id2;
    QString name;
    QDate released;
};

struct TrailerResult
{
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
struct Poster
{
    QString id;
    QUrl originalUrl;
    QUrl thumbUrl;
    QSize originalSize;
    QString language;
    QString hint;
    int season{0};
};

enum TvShowType
{
    TypeTvShow,
    TypeEpisode,
    TypeSeason
};

enum MusicType
{
    TypeArtist,
    TypeAlbum
};

enum class ItemType
{
    Movie,
    TvShow,
    TvShowEpisode,
    Concert,
    Artist,
    Album
};

enum class DiscType
{
    Single,
    BluRay,
    Dvd
};

enum MovieSetArtworkType
{
    MovieSetArtworkSingleSetFolder,
    MovieSetArtworkSingleArtworkFolder
};

/**
 * @brief The SettingsDir struct
 */
struct SettingsDir
{
    QString path;
    bool separateFolders;
    bool autoReload;
};

enum SettingsDirType
{
    Movies,
    TvShows,
    Concerts,
    Downloads,
    Music
};

enum ComboDelegateType
{
    Genres,
    Studios,
    Countries,
    Writers,
    Directors
};

// clang-format off
enum class ImageType : int {
    None                 = -1,
    MoviePoster          = 1,
    MovieBackdrop        = 2,
    TvShowPoster         = 3,
    TvShowBackdrop       = 4,
    TvShowEpisodeThumb   = 5,
    TvShowBanner         = 7,
    ConcertPoster        = 8,
    ConcertBackdrop      = 9,
    MovieLogo            = 10,
    MovieClearArt        = 11,
    MovieCdArt           = 12,
    ConcertLogo          = 13,
    ConcertClearArt      = 14,
    ConcertCdArt         = 15,
    TvShowClearArt       = 16,
    TvShowLogos          = 17,
    TvShowCharacterArt   = 18,
    TvShowSeasonBackdrop = 19,
    TvShowSeasonBanner   = 20,
    MovieBanner          = 21,
    MovieThumb           = 22,
    TvShowThumb          = 23,
    TvShowSeasonThumb    = 24,
    TvShowSeasonPoster   = 25,
    Actor                = 26,
    MovieExtraFanart     = 27,
    MovieSetPoster       = 28,
    MovieSetBackdrop     = 29,
    ConcertExtraFanart   = 30,
    TvShowExtraFanart    = 31,
    ArtistThumb          = 32,
    ArtistFanart         = 33,
    ArtistLogo           = 34,
    AlbumThumb           = 35,
    AlbumCdArt           = 36,
    ArtistExtraFanart    = 37,
    AlbumBooklet         = 38
};

enum class MovieScraperInfos : int {
    Title         = 1,
    Tagline       = 2,
    Rating        = 3,
    Released      = 4,
    Runtime       = 5,
    Certification = 6,
    Trailer       = 7,
    Overview      = 8,
    Poster        = 9,
    Backdrop      = 10,
    Actors        = 11,
    Genres        = 12,
    Studios       = 13,
    Countries     = 14,
    Writer        = 15,
    Director      = 16,
    Tags          = 18,
    ExtraFanarts  = 19,
    Set           = 20,
    Logo          = 21,
    CdArt         = 22,
    ClearArt      = 23,
    Banner        = 24,
    Thumb         = 25,
    First         = 1,
    Last          = 25
};

enum class TvShowScraperInfos : int {
    Actors         = 1,
    Banner         = 2,
    Certification  = 3,
    Director       = 4,
    Fanart         = 5,
    FirstAired     = 6,
    Genres         = 7,
    Network        = 8,
    Overview       = 9,
    Poster         = 10,
    Rating         = 11,
    SeasonPoster   = 13,
    Thumbnail      = 14,
    Title          = 15,
    Writer         = 16,
    Tags           = 17,
    ExtraArts      = 18,
    SeasonBackdrop = 19,
    SeasonBanner   = 20,
    ExtraFanarts   = 21,
    Thumb          = 22,
    SeasonThumb    = 23,
    Runtime        = 24,
    Status         = 25
};

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
// clang-format on

enum SortBy
{
    SortByName,
    SortBySeen,
    SortByAdded,
    SortByYear,
    SortByNew
};

// clang-format off
enum class DataFileType : int {
    NoType               = -1,
    MovieNfo             = 1,
    MoviePoster          = 2,
    MovieBackdrop        = 3,
    MovieLogo            = 4,
    MovieClearArt        = 5,
    MovieCdArt           = 6,
    ConcertNfo           = 7,
    ConcertPoster        = 8,
    ConcertBackdrop      = 9,
    ConcertLogo          = 10,
    ConcertClearArt      = 11,
    ConcertCdArt         = 12,
    TvShowNfo            = 13,
    TvShowPoster         = 14,
    TvShowBackdrop       = 15,
    TvShowBanner         = 16,
    TvShowSeasonPoster   = 17,
    TvShowLogo           = 18,
    TvShowClearArt       = 19,
    TvShowCharacterArt   = 20,
    TvShowEpisodeNfo     = 21,
    TvShowEpisodeThumb   = 22,
    TvShowSeasonBackdrop = 23,
    TvShowSeasonBanner   = 24,
    MovieBanner          = 25,
    MovieThumb           = 26,
    TvShowSeasonThumb    = 27,
    TvShowThumb          = 28,
    MovieSetPoster       = 29,
    MovieSetBackdrop     = 30,
    ArtistNfo            = 31,
    AlbumNfo             = 32,
    ArtistThumb          = 33,
    ArtistFanart         = 34,
    ArtistLogo           = 35,
    AlbumThumb           = 36,
    AlbumCdArt           = 37
};
// clang-format on

enum TvShowUpdateType
{
    UpdateShow,
    UpdateShowAndAllEpisodes,
    UpdateShowAndNewEpisodes,
    UpdateNewEpisodes,
    UpdateAllEpisodes
};

enum ScraperData
{
    DataInfos,
    DataCasts,
    DataTrailers,
    DataImages,
    DataReleases
};

struct ExtraFanart
{
    QByteArray image;
    QString path;
};

enum MediaStatusColumns
{
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

// clang-format off
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
// clang-format on

struct MovieDuplicate
{
    bool title;
    bool imdbId;
    bool tmdbId;
};

#endif // GLOBALS_H
