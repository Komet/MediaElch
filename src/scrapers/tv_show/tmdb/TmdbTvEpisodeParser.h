#pragma once

#include "scrapers/tmdb/TmdbApi.h"

#include <QJsonObject>

class TvShowEpisode;

namespace mediaelch {
namespace scraper {

class TmdbTvEpisodeParser
{
public:
    /// \brief Parse the given JSON document and assign the details to the given episode.
    /// \param episode Where to store the episode details into.
    /// \param data JSON document from TMDb
    static void parseInfos(const TmdbApi& api, TvShowEpisode& episode, const QJsonObject& data);
};

} // namespace scraper
} // namespace mediaelch
