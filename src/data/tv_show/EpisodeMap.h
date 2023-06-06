#pragma once

#include "data/tv_show/EpisodeNumber.h"
#include "data/tv_show/SeasonNumber.h"

#include <QMap>
#include <QPair>

class TvShowEpisode;

namespace mediaelch {

/// \brief Map of season/episode number and corresponding episode object.
/// \details Used by TV scrapers when scraping multiple seasons.
using EpisodeMap = QMap<QPair<SeasonNumber, EpisodeNumber>, TvShowEpisode*>;
using EpisodeMapIterator = QMapIterator<QPair<SeasonNumber, EpisodeNumber>, TvShowEpisode*>;

} // namespace mediaelch
