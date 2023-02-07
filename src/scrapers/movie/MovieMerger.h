#pragma once

#include "scrapers/ScraperInfos.h"

#include <QSet>

class Movie;

namespace mediaelch {
namespace scraper {

void copyDetailsToMovie(Movie& target,
    const Movie& source,
    const QSet<MovieScraperInfo>& details,
    bool usePlotForOutline);

} // namespace scraper
} // namespace mediaelch
