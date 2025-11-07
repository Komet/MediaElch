#pragma once

#include "data/ImdbId.h"
#include "data/tv_show/EpisodeNumber.h"
#include "data/tv_show/SeasonNumber.h"

#include <QObject>
#include <QSet>
#include <QString>
#include <memory>

class TvShowEpisode;

namespace mediaelch {
namespace scraper {

class ImdbApi;

class ImdbTvSeasonParser
{
public:
    ImdbTvSeasonParser() = default;

    /// \brief Returns a list of available seasons which is parsed from the
    ///        episode overview page of a TV show.
    /// \param html HTML from https://www.imdb.com/title/tt<id>/episodes
    static QSet<SeasonNumber> parseSeasonNumbersFromEpisodesPage(const QString& html);

    /// \brief Parses episode IDs from the HTML.
    /// \param html IMDb website HTML for a season page.
    /// \param forSeason Only parse episode IDs for this season.
    static QMap<EpisodeNumber, ImdbId> parseEpisodeIds(const QString& html, int forSeason);
};

} // namespace scraper
} // namespace mediaelch
