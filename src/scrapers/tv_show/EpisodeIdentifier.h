#pragma once

#include "data/ImdbId.h"
#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/SeasonOrder.h"
#include "tv_shows/TvDbId.h"

#include <QDebug>
#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief This class uniquely identifies an episode for scrapers.
/// \details An episode is either identified by an unique string, e.g. TheTvDb
///          ID or an URL or it is identified by a season number, episode
///          number and season order and of course its TvShow identifier.
class EpisodeIdentifier
{
public:
    explicit EpisodeIdentifier(QString _episodeIdentifier) : episodeIdentifier{std::move(_episodeIdentifier)} {}
    explicit EpisodeIdentifier(TvDbId _episodeIdentifier) : episodeIdentifier{_episodeIdentifier.toString()} {}
    explicit EpisodeIdentifier(ImdbId _episodeIdentifier) : episodeIdentifier{_episodeIdentifier.toString()} {}

    EpisodeIdentifier(QString _showIdentifier, SeasonNumber season, EpisodeNumber episode, SeasonOrder order) :
        showIdentifier{std::move(_showIdentifier)},
        seasonNumber{std::move(season)},
        episodeNumber{std::move(episode)},
        seasonOrder{order}
    {
    }
    ~EpisodeIdentifier() = default;

    /// \brief Whether the episode can be uniquely identified by its
    ///        identifier string.
    bool hasEpisodeIdentifier() const { return !episodeIdentifier.isEmpty(); }

public:
    QString episodeIdentifier;

    QString showIdentifier;
    SeasonNumber seasonNumber;
    EpisodeNumber episodeNumber;
    SeasonOrder seasonOrder = SeasonOrder::Aired;
};

QDebug operator<<(QDebug debug, const EpisodeIdentifier& id);

} // namespace scraper
} // namespace mediaelch
