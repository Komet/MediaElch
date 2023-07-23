#pragma once

#include "third_party/catch2/catch.hpp"

#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/music/MusicScraper.h"
#include "scrapers/tv_show/TvScraper.h"

#include <QPair>

namespace test {

// TODO: We can streamline this: All search jobs have a similar interface. Use a template here.

QPair<QVector<mediaelch::scraper::ConcertSearchJob::Result>, mediaelch::ScraperError>
searchConcertScraperSync(mediaelch::scraper::ConcertSearchJob* searchJob, bool mayError = false);

QPair<QVector<mediaelch::scraper::ShowSearchJob::Result>, mediaelch::ScraperError>
searchTvScraperSync(mediaelch::scraper::ShowSearchJob* searchJob, bool mayError = false);

QPair<QVector<mediaelch::scraper::MovieSearchJob::Result>, mediaelch::ScraperError>
searchMovieScraperSync(mediaelch::scraper::MovieSearchJob* searchJob, bool mayError = false);

QPair<QVector<mediaelch::scraper::ArtistSearchJob::Result>, mediaelch::ScraperError>
searchArtistScraperSync(mediaelch::scraper::ArtistSearchJob* searchJob, bool mayError = false);
QPair<QVector<mediaelch::scraper::AlbumSearchJob::Result>, mediaelch::ScraperError>
searchAlbumScraperSync(mediaelch::scraper::AlbumSearchJob* searchJob, bool mayError = false);

void scrapeMovieScraperSync(mediaelch::scraper::MovieScrapeJob* scrapeJob, bool mayError = false);

void scrapeJobSync(mediaelch::worker::Job* job, bool mayError = false);
void scrapeTvScraperSync(mediaelch::scraper::ShowScrapeJob* scrapeJob, bool mayError = false);
void scrapeEpisodeSync(mediaelch::scraper::EpisodeScrapeJob* scrapeJob, bool mayError = false);
void scrapeSeasonSync(mediaelch::scraper::SeasonScrapeJob* scrapeJob, bool mayError = false);

} // namespace test
