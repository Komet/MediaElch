#pragma once

#include "data/TmdbId.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/tmdb/TmdbMovieScrapeJob.h"
#include "scrapers/movie/tmdb/TmdbMovieSearch.h"

#include <QObject>
#include <QVector>

namespace mediaelch {
namespace scraper {

class TmdbMovieScrapeJob;

class TmdbMovie : public MovieScraper
{
    Q_OBJECT

public:
    explicit TmdbMovie(QObject* parent = nullptr);
    ~TmdbMovie() override = default;

    const ScraperInfo& info() const override;

    void initialize() override;
    bool isInitialized() const override;

    TmdbMovieSearchJob* search(MovieSearchJob::Config config) override;

    TmdbMovieScrapeJob* scrape(Movie& movie, MovieScrapeJob::Config config) override;

private slots:
    void handleConfigurationResponse();

private:
    ScraperInfo m_info;
    TmdbApi m_api;
    bool m_configLoaded = false;
};

} // namespace scraper
} // namespace mediaelch
