#pragma once

#include "data/movie/Movie.h"
#include "network/NetworkManager.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/movie/MovieScraper.h"

namespace mediaelch {
namespace scraper {

class ImdbMovieConfiguration;

class ImdbMovie : public MovieScraper
{
    Q_OBJECT
public:
    explicit ImdbMovie(ImdbMovieConfiguration& settings, QObject* parent = nullptr);
    ~ImdbMovie() override;
    static constexpr const char* ID = "IMDb";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;

public:
    QSet<MovieScraperInfo> scraperNativelySupports() override;
    void changeLanguage(mediaelch::Locale locale) override;

private:
    ImdbMovieConfiguration& m_settings;
    ImdbApi m_api;
    ScraperMeta m_meta;

    mediaelch::network::NetworkManager m_network;

    ScraperSearchResult parseIdFromMovieHtml(const QString& html);
};

} // namespace scraper
} // namespace mediaelch
