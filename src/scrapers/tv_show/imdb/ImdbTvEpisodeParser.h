#pragma once

#include "data/ImdbId.h"
#include "data/Locale.h"

#include <QString>

class TvShowEpisode;

namespace mediaelch {
namespace scraper {

class ImdbTvEpisodeParser
{
public:
    /// \brief Parse the given HTML string and assign the details to the given episode.
    /// \param episode Where to store the episode details into.
    /// \param html HTML string from imdb.com
    /// \param preferredLocale Use this locale if values exist in multiple languages.
    static void parseInfos(TvShowEpisode& episode, const QString& html, const Locale& preferredLocale);
    /// \brief Parses the IMDb id from the IMDb season HTML code for the given episode
    ///        by using its season/episode number.
    /// \param episode Where to store the episode ID into.
    /// \param html Season HTML string from imdb.com
    static void parseIdFromSeason(TvShowEpisode& episode, const QString& html);
};

} // namespace scraper
} // namespace mediaelch
