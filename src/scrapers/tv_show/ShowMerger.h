#pragma once

#include "globals/ScraperInfos.h"
#include "tv_shows/EpisodeMap.h"
#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"

#include <QMap>
#include <QPair>
#include <QSet>

class TvShow;
class TvShowEpisode;

namespace mediaelch {
namespace scraper {

/// \brief Copy the given details from the source show to target show.
/// \details Because TV show scrapers have their own show, we need to copy
/// details from one to another object.  Scrapers may set more elements than
/// were initially requested so this function becomes necessary.
///
/// Note that for actors and images, details are *appended* and do not replace
/// existing actors or images.
void copyDetailsToShow(TvShow& target, TvShow& source, const QSet<ShowScraperInfo>& details);

/// \brief Copy the given details from the source episode to target episode.
/// \details Because episode scrapers have their own episode, we need to copy
/// details from one to another object.  Scrapers may set more elements than
/// were initially requested so this function becomes necessary.
///
/// Note that for actors and images, details are *appended* and do not replace
/// existing actors or images.
void copyDetailsToEpisode(TvShowEpisode& target, const TvShowEpisode& source, const QSet<EpisodeScraperInfo>& details);

/// \brief Copy the given details from the source episodes to target show episodes.
/// \details Because season scrapers have their own episodes, we need to copy
/// details from one to another object.  The set TV show must have all
/// episodes() available and they must have proper season- and episode numbers.
///
/// Note that for actors and images, details are *appended* and do not replace
/// existing actors or images.
void copyDetailsToShowEpisodes(TvShow& target,
    const EpisodeMap& source,
    bool onlyCopyNew,
    const QSet<EpisodeScraperInfo>& details);

/// \brief Merges episodes from source into target according to given details.
/// \details New episodes from the source are copied into the target with the given parent.
///          Existing ones are updated according to the given details set.
///          Note: Ownership of TvShowEpisode* are not handled!
void copyDetailsToEpisodeMap(EpisodeMap& target,
    const EpisodeMap& source,
    const QSet<EpisodeScraperInfo>& details,
    QObject* parentForNewEpisodes);

} // namespace scraper
} // namespace mediaelch
