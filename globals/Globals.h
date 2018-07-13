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

enum class TvShowType : int
{
    None,
    TvShow,
    Episode,
    Season
};

enum class MusicType : int
{
    None,
    Artist,
    Album
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

enum class MovieSetArtworkType : int
{
    SingleSetFolder,
    SingleArtworkFolder
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

enum class SettingsDirType : int
{
    Movies,
    TvShows,
    Concerts,
    Downloads,
    Music
};

enum class ComboDelegateType : int
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

enum class ConcertScraperInfos : int {
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
    Genres        = 11,
    ExtraArts     = 12,
    Tags          = 13,
    ExtraFanarts  = 14
};

enum class MusicScraperInfos : int {
    Name         = 1,
    Genres       = 2,
    Styles       = 3,
    Moods        = 4,
    YearsActive  = 5,
    Formed       = 6,
    Born         = 7,
    Died         = 8,
    Disbanded    = 9,
    Biography    = 10,
    Thumb        = 11,
    Fanart       = 12,
    Logo         = 13,
    Title        = 14,
    Artist       = 15,
    Review       = 16,
    ReleaseDate  = 17,
    Label        = 18,
    Rating       = 19,
    Year         = 20,
    CdArt        = 21,
    Cover        = 22,
    ExtraFanarts = 23,
    Discography  = 24
};

// The filter numbers have to be unique for MovieFilters, TvShowFilters and ConcertFilters
enum class MovieFilters : int
{
    Released            = 1,
    Certification       = 2,
    Trailer             = 3,
    Poster              = 4,
    Backdrop            = 5,
    Watched             = 6,
    Genres              = 7,
    Title               = 8,
    Logo                = 9,
    ClearArt            = 10,
    CdArt               = 11,
    StreamDetails       = 12,
    ExtraFanarts        = 13,
    Actors              = 14,
    LocalTrailer        = 15,
    Country             = 16,
    Studio              = 17,
    Path                = 18,
    Director            = 19,
    Tags                = 20,
    Quality             = 21,
    Banner              = 22,
    Thumb               = 23,
    ImdbId              = 24,
    Set                 = 25,
    Rating              = 28,
    Label               = 29,
    AudioChannels       = 30,
    AudioQuality        = 31,
    HasSubtitle         = 33,
    HasExternalSubtitle = 34,
    VideoCodec          = 35
};
// clang-format on

enum class TvShowFilters : int
{
    Title = 26
};

enum class ConcertFilters : int
{
    Title = 27
};

enum class MusicFilters : int
{
    Title = 32
};

enum class SortBy
{
    Name,
    Seen,
    Added,
    Year,
    New
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

enum class TvShowUpdateType : int
{
    Show,
    ShowAndAllEpisodes,
    ShowAndNewEpisodes,
    NewEpisodes,
    AllEpisodes
};

enum class ScraperData : int
{
    Infos,
    Casts,
    Trailers,
    Images,
    Releases
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
