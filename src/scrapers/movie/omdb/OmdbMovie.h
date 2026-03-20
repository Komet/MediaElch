#pragma once

#include "scrapers/movie/MovieScraper.h"
#include "scrapers/omdb/OmdbApi.h"

#include <QObject>

namespace mediaelch {
namespace scraper {

class OmdbMovieConfiguration;

class OmdbMovie final : public MovieScraper
{
    Q_OBJECT
public:
    static constexpr const char* ID = "omdb";

public:
    explicit OmdbMovie(OmdbMovieConfiguration& settings, QObject* parent = nullptr);
    ~OmdbMovie() override;

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;

public:
    QSet<MovieScraperInfo> scraperNativelySupports() override;
    void changeLanguage(mediaelch::Locale locale) override;

private:
    OmdbMovieConfiguration& m_settings;
    OmdbApi m_api;
    ScraperMeta m_meta;

    QSet<MovieScraperInfo> m_scraperNativelySupports;
};

} // namespace scraper
} // namespace mediaelch
