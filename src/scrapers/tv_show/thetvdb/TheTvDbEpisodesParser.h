#pragma once

#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"

#include <QJsonDocument>
#include <memory>

class TvShowEpisode;

namespace mediaelch {
namespace scraper {

class TheTvDbEpisodesParser
{
public:
    TheTvDbEpisodesParser() {}

    /// \brief Parses episodes from the JSON and passes new objects to the callback.
    /// \param json TheTvDb api JSON response (episode page)
    /// \param parentForEpisodes QObject parent parameter for new TvShowEpisodes.
    /// \param episodeCallback Called when an episode is parsed. Can be used to
    ///                        delete the generated episode and/or store the
    ///                        pointer.
    static TheTvDbApi::Paginate parseEpisodes(const QJsonDocument& json,
        SeasonOrder seasonOrder,
        QObject* parentForEpisodes,
        std::function<void(TvShowEpisode*)> episodeCallback);
};

} // namespace scraper
} // namespace mediaelch
