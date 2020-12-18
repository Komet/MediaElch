#pragma once

#include <QJsonDocument>
#include <QJsonObject>

class TvShowEpisode;

namespace mediaelch {
namespace scraper {

class TvMazeEpisodeParser
{
public:
    /// \brief Extract the episode's details from the given JSON document.
    static void parseEpisode(TvShowEpisode& episode, const QJsonDocument& json);

    /// \brief Extract the episode's details from the given JSON object.
    static void parseEpisode(TvShowEpisode& episode, const QJsonObject& json);

    /// \brief Extracts and parses the requested episode (season/episode) number from the JSON array.
    /// \details TVmaze provides an overview over all episodes of a show.  We
    ///          may only want a single episode. This function searches for the
    ///          episode that we want and parses it, i.e. stores its details in
    ///          the provided episode.
    static void parseEpisodeFromOverview(TvShowEpisode& episode, const QJsonDocument& json);
};

} // namespace scraper
} // namespace mediaelch
