#pragma once

#include "tv_shows/TvShowEpisode.h"

#include <QJsonObject>
#include <QString>

namespace mediaelch {
namespace scraper {

class TheTvDbEpisodeParser
{
public:
    /// \brief Initialize an episode parser for the given episode.
    /// \param episode Episode into which details are stored.
    /// \param order Which order to use. TheTvDb sends both numbers for requests.
    TheTvDbEpisodeParser(TvShowEpisode& episode, SeasonOrder order) : m_episode{episode}, m_order{order} {}

    void parseInfos(const QJsonDocument& json);
    void parseInfos(const QJsonObject& episodeObj);
    void parseIdFromSeason(const QJsonDocument& json);

private:
    TvShowEpisode& m_episode;
    SeasonOrder m_order;
};

} // namespace scraper
} // namespace mediaelch
