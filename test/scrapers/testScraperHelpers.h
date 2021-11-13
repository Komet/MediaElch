#pragma once

#include "third_party/catch2/catch.hpp"

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

void scrapeTvScraperSync(mediaelch::scraper::ShowScrapeJob* scrapeJob, bool mayError = false);

std::ostream& operator<<(std::ostream& os, const QVector<Actor*>& value);

struct HasActorMatcher : Catch::MatcherBase<QVector<Actor*>>
{
    HasActorMatcher(const QString& name, const QString& role) : m_name{name}, m_role{role} {}
    bool match(const QVector<Actor*>& source) const override;
    std::string describe() const override;

private:
    QString m_name;
    QString m_role;
};

HasActorMatcher HasActor(const QString& name, const QString& role);
