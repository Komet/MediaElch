#pragma once

#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/tv_show/TvScraper.h"

#include <QPair>


QPair<QVector<mediaelch::scraper::ConcertSearchJob::Result>, mediaelch::ScraperError>
searchConcertScraperSync(mediaelch::scraper::ConcertSearchJob* searchJob, bool mayError = false);

QPair<QVector<mediaelch::scraper::ShowSearchJob::Result>, mediaelch::ScraperError>
searchTvScraperSync(mediaelch::scraper::ShowSearchJob* searchJob, bool mayError = false);

QPair<QVector<mediaelch::scraper::MovieSearchJob::Result>, mediaelch::ScraperError>
searchMovieScraperSync(mediaelch::scraper::MovieSearchJob* searchJob, bool mayError = false);

void scrapeMovieScraperSync(mediaelch::scraper::MovieScrapeJob* scrapeJob, bool mayError = false);

void scrapeTvScraperSync(mediaelch::scraper::ShowScrapeJob* scrapeJob, bool mayError = false);
