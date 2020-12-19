#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>

// clang-format: off

enum class MovieScraperInfo : int
{
    Invalid = 0, // Only used to check that serialization works
    Title = 1,
    Tagline = 2,
    Rating = 3,
    Released = 4,
    Runtime = 5,
    Certification = 6,
    Trailer = 7,
    Overview = 8,
    Poster = 9,
    Backdrop = 10,
    Actors = 11,
    Genres = 12,
    Studios = 13,
    Countries = 14,
    Writer = 15,
    Director = 16,
    Tags = 18,
    ExtraFanarts = 19,
    Set = 20,
    Logo = 21,
    CdArt = 22,
    ClearArt = 23,
    Banner = 24,
    Thumb = 25,
    First = 1,
    Last = 25
};

namespace mediaelch {
namespace scraper {
QSet<MovieScraperInfo> allMovieScraperInfos();
} // namespace scraper
} // namespace mediaelch

inline uint qHash(const MovieScraperInfo& key, uint seed)
{
    return qHash(static_cast<int>(key), seed);
}

enum class ShowScraperInfo : int
{
    Invalid = 0, // Only used to check that serialization works
    Actors = 1,
    Banner = 2,
    Certification = 3,
    // Only episodes: Director = 4,
    Fanart = 5,
    FirstAired = 6,
    Genres = 7,
    Network = 8,
    Overview = 9,
    Poster = 10,
    Rating = 11,
    SeasonPoster = 13,
    // Only episodes: Thumbnail = 14,
    Title = 15,
    // Only episodes: Writer = 16,
    Tags = 17,
    ExtraArts = 18,
    SeasonBackdrop = 19,
    SeasonBanner = 20,
    ExtraFanarts = 21,
    Thumb = 22,
    SeasonThumb = 23,
    Runtime = 24,
    Status = 25
};

namespace mediaelch {
QString scraperInfoToTranslatedString(ShowScraperInfo info);
QSet<ShowScraperInfo> allShowScraperInfos();
} // namespace mediaelch

inline uint qHash(const ShowScraperInfo& key, uint seed)
{
    return qHash(static_cast<int>(key), seed);
}

enum class EpisodeScraperInfo : int
{
    Invalid = 0, // Only used to check that serialization works
    Actors = 1,
    // Only Shows: Banner = 2,
    Certification = 3,
    Director = 4,
    // Only Shows: Fanart = 5,
    FirstAired = 6,
    // Only Shows: Genres = 7,
    Network = 8,
    Overview = 9,
    // Only Shows: Poster = 10,
    Rating = 11,
    // Only Shows: SeasonPoster = 13,
    Thumbnail = 14,
    Title = 15,
    Writer = 16,
    // Only Shows: Tags = 17,
    // Only Shows: ExtraArts = 18,
    // Only Shows: SeasonBackdrop = 19,
    // Only Shows: SeasonBanner = 20,
    // Only Shows: ExtraFanarts = 21,
    // Not used: Thumb = 22,
    // Only Shows: SeasonThumb = 23,
    // Only Shows: Runtime = 24,
    // Only Shows: Status = 25
    Tags = 26
};

namespace mediaelch {
QString scraperInfoToTranslatedString(EpisodeScraperInfo info);
QSet<EpisodeScraperInfo> allEpisodeScraperInfos();
} // namespace mediaelch

inline uint qHash(const EpisodeScraperInfo& key, uint seed)
{
    return qHash(static_cast<int>(key), seed);
}

enum class ConcertScraperInfo : int
{
    Invalid = 0, // Only used to check that serialization works
    Title = 1,
    Tagline = 2,
    Rating = 3,
    Released = 4,
    Runtime = 5,
    Certification = 6,
    Trailer = 7,
    Overview = 8,
    Poster = 9,
    Backdrop = 10,
    Genres = 11,
    ExtraArts = 12,
    Tags = 13,
    ExtraFanarts = 14
};

inline uint qHash(const ConcertScraperInfo& key, uint seed)
{
    return qHash(static_cast<int>(key), seed);
}

enum class MusicScraperInfo : int
{
    Invalid = 0, // Only used to check that serialization works
    Name = 1,
    Genres = 2,
    Styles = 3,
    Moods = 4,
    YearsActive = 5,
    Formed = 6,
    Born = 7,
    Died = 8,
    Disbanded = 9,
    Biography = 10,
    Thumb = 11,
    Fanart = 12,
    Logo = 13,
    Title = 14,
    Artist = 15,
    Review = 16,
    ReleaseDate = 17,
    Label = 18,
    Rating = 19,
    Year = 20,
    CdArt = 21,
    Cover = 22,
    ExtraFanarts = 23,
    Discography = 24
};

// clang-format: on

inline uint qHash(const MusicScraperInfo& key, uint seed)
{
    return qHash(static_cast<int>(key), seed);
}

// Just for translations
class ScraperInfoTranslation : public QObject
{
    Q_OBJECT
public:
    ScraperInfoTranslation(QObject* parent = nullptr) : QObject(parent) {}
    ~ScraperInfoTranslation() override;
    QString toString(ShowScraperInfo info);
    QString toString(EpisodeScraperInfo info);
};
