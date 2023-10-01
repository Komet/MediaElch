#include "Rating.h"

#include <QMap>

Rating::Rating() = default;

QString Rating::sourceToName(const QString& source)
{
    static QMap<QString, QString> r({
        {"thetvdb", "TheTvDb"},
        {"tvdb", "TheTvDb"},
        {"themoviedb", "TMDB"},
        {"tmdb", "TMDB"},
        {"imdb", "IMDb"},
        {"metacritic", "Metacritic"},
        {"ofdb", "OFDb"}, // Removed in MediaElch, kept for backward compatibility
        {"default", "Default"},
        {"thetvdb", "TheTvDb"} //
    });

    if (r.contains(source)) {
        return r[source];
    }
    return source;
}

QStringList Rating::commonSources()
{
    // See https://kodi.wiki/view/NFO_files/Movies
    return {"themoviedb", "imdb", "tvdb", "metacritic", "default"};
}

void Ratings::setOrAddRating(const Rating& rating)
{
    auto it = std::find_if(
        m_ratings.begin(), m_ratings.end(), [&rating](const Rating& r) { return r.source == rating.source; });

    if (it == m_ratings.end()) {
        m_ratings.push_back(rating);
    } else {
        *it = rating;
    }
}

void Ratings::addRating(const Rating& rating)
{
    m_ratings.push_back(rating);
}

bool Ratings::hasSource(const QString& source) const
{
    auto it =
        std::find_if(m_ratings.begin(), m_ratings.end(), [&source](const Rating& r) { return r.source == source; });

    return it != m_ratings.end();
}

void Ratings::merge(const Ratings& ratings)
{
    for (const Rating& rating : ratings) {
        setOrAddRating(rating);
    }
}
