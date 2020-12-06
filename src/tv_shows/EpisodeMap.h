#pragma once

#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"

#include <QMap>
#include <QPair>

class TvShowEpisode;

namespace mediaelch {

/// \brief Map of season/episode number and corresponding episode object.
/// \details Used by TV scrapers when scraping multiple seasons.
using EpisodeMap = QMap<QPair<SeasonNumber, EpisodeNumber>, TvShowEpisode*>;

} // namespace mediaelch
