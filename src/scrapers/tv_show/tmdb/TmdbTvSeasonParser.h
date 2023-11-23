#pragma once

#include "data/Actor.h"

#include <QJsonDocument>
#include <QObject>
#include <QVector>
#include <functional>
#include <memory>

class TvShowEpisode;

namespace mediaelch {
namespace scraper {

class TmdbApi;

class TmdbTvSeasonParser
{
public:
    TmdbTvSeasonParser() = default;

    /// \brief Parses episodes from the json and passes new objects to the callback, see
    ///        https://developers.themoviedb.org/3/tv-seasons/get-tv-season-details
    /// \param json TmdbTv API JSON response (season page)
    /// \param parentForEpisodes QObject parent parameter for new TvShowEpisodes.
    /// \param episodeCallback Called when an episode is parsed. Can be used to
    ///                        delete the generated episode and/or store the
    ///                        pointer.
    static void parseEpisodes(TmdbApi& api,
        const QJsonDocument& json,
        QObject* parentForEpisodes,
        std::function<void(TvShowEpisode*)> episodeCallback);

    static QVector<Actor> parseSeasonActors(TmdbApi& api, const QJsonDocument& data);
};

} // namespace scraper
} // namespace mediaelch
