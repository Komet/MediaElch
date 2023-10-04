#pragma once

#include "scrapers/ScraperInfos.h"

#include <QSet>

class Movie;

namespace mediaelch {
namespace scraper {

/// Copy the requested details from source to target, while blocking signals of target.
/// TODO: No multiple boolean arguments
void copyDetailsToMovie(Movie& target,
    const Movie& source,
    const QSet<MovieScraperInfo>& details,
    bool usePlotForOutline,
    bool ignoreDuplicateOriginalTitle);

} // namespace scraper
} // namespace mediaelch
