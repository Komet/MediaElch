#pragma once

#include "globals/ScraperInfos.h"

#include <QSet>

class Movie;

namespace mediaelch {
namespace scraper {

void copyDetailsToMovie(Movie& target, const Movie& source, const QSet<MovieScraperInfo>& details);

} // namespace scraper
} // namespace mediaelch
