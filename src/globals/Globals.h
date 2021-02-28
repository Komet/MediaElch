#pragma once

#include "globals/Meta.h"

#include <QDate>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QMap>
#include <QMetaType>
#include <QString>
#include <QUrl>
#include <QVariant>

// Required for smoother upgrade to Qt 6 while still working with Qt 5
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
using ElchSplitBehavior = QString::SplitBehavior;
#else
using ElchSplitBehavior = Qt::SplitBehaviorFlags;
#endif

// clang-format off

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
    Downloads,
    Duplicates
};

struct DiscographyAlbum
{
    QString title;
    QString year;
};
Q_DECLARE_METATYPE(DiscographyAlbum*)

struct TrailerResult
{
    QUrl preview;
    QString name;
    QString language;
    QUrl trailerUrl;
    QImage previewImage;
    bool previewImageLoaded = false;
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

struct SettingsDir
{
    QDir path;
    bool separateFolders = false;
    bool autoReload = false;
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

inline uint qHash(const ImageType& type, uint seed)
{
    return qHash(static_cast<int>(type), seed);
}

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
    VideoCodec          = 35,
    TmdbId              = 36,
    OriginalTitle       = 37
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

bool isShowUpdateType(TvShowUpdateType type);
bool isEpisodeUpdateType(TvShowUpdateType type);
bool isNewEpisodeUpdateType(TvShowUpdateType type);
bool isAllEpisodeUpdateType(TvShowUpdateType type);

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

enum class MediaStatusColumn
{
    Id,
    StreamDetails,
    Trailer,
    LocalTrailer,
    Poster,
    Fanart,
    ExtraArts,
    ExtraFanarts,
    Actors,
    Unknown,

    First = Id,
    Last = Actors
};

enum class ColorLabel : int
{
    NoLabel = 0,
    Red = 1,
    Orange = 2,
    Yellow = 3,
    Green = 4,
    Blue = 5,
    Purple = 6,
    Grey = 7
};

struct MovieDuplicate
{
    bool title = false;
    bool imdbId = false;
    bool tmdbId = false;
};
