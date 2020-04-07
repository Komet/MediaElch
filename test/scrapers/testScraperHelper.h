#pragma once

#include "scrapers/movie/MovieScraper.h"

namespace test {

QVector<mediaelch::scraper::MovieSearchJob::Result> searchMovieSync(mediaelch::scraper::MovieScraper& scraper,
    const mediaelch::scraper::MovieSearchJob::Config& config);


void loadMovieSync(mediaelch::scraper::MovieScraper& scraper,
    Movie& movie,
    QString identifier,
    QString locale,
    mediaelch::scraper::MovieScraper::LoadDetails details);

} // namespace test
