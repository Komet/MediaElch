#pragma once

#include "data/movie/Movie.h"
#include "scrapers/movie/MovieScrapeJob.h"
#include "scrapers/movie/MovieScraper.h"

namespace test {

/// \brief Stub used to test the custom movie scraper's implementation.
class StubMovieScraper : public mediaelch::scraper::MovieScraper
{
    Q_OBJECT

public:
    explicit StubMovieScraper(const QString& id, QObject* parent = nullptr);
    ~StubMovieScraper() override = default;
    const ScraperMeta& meta() const override { return m_meta; }

    void initialize() override {}
    ELCH_NODISCARD bool isInitialized() const override { return true; }
    ELCH_NODISCARD mediaelch::scraper::MovieSearchJob* search(
        mediaelch::scraper::MovieSearchJob::Config config) override;
    ELCH_NODISCARD mediaelch::scraper::MovieScrapeJob* loadMovie(
        mediaelch::scraper::MovieScrapeJob::Config config) override;

public:
    QSet<MovieScraperInfo> scraperNativelySupports() override { return {}; }
    void changeLanguage(mediaelch::Locale locale) override { Q_UNUSED(locale) }

public:
    Movie stub_movie;

private:
    ScraperMeta m_meta;
};

/// \brief Stub used to test the custom movie scraper's implementation.
class StubMovieScrapeJob : public mediaelch::scraper::MovieScrapeJob
{
    Q_OBJECT

public:
    explicit StubMovieScrapeJob(Config config, Movie& stubMovie, QObject* parent = nullptr);
    ~StubMovieScrapeJob() override = default;
    void doStart() override;
};


/// \brief Stub used to test the custom movie scraper's implementation.
class StubMovieSearchJob : public mediaelch::scraper::MovieSearchJob
{
    Q_OBJECT

public:
    explicit StubMovieSearchJob(Config config, QObject* parent = nullptr) : MovieSearchJob(std::move(config), parent) {}
    ~StubMovieSearchJob() override = default;
    void doStart() override;
};

} // namespace test
