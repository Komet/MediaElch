#pragma once

#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/videobuster/VideoBusterApi.h"

#include <QObject>

namespace mediaelch {
namespace scraper {

class VideoBuster : public MovieScraper
{
    Q_OBJECT
public:
    explicit VideoBuster(QObject* parent = nullptr);
    static constexpr const char* ID = "videobuster";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;

public:
    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;

private:
    ScraperMeta m_meta;
    VideoBusterApi m_api;
};

} // namespace scraper
} // namespace mediaelch
