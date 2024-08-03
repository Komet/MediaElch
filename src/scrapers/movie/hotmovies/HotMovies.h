#pragma once

#include "network/NetworkManager.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/hotmovies/HotMoviesApi.h"

#include <QObject>

namespace mediaelch {
namespace scraper {

class HotMovies : public MovieScraper
{
    Q_OBJECT
public:
    explicit HotMovies(QObject* parent = nullptr);
    static constexpr const char* ID = "hotmovies";

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
    HotMoviesApi m_api;
    mediaelch::network::NetworkManager m_network;

private:
    mediaelch::network::NetworkManager* network();
    void parseAndAssignInfos(QString html, Movie* movie, QSet<MovieScraperInfo> infos);
    QString decodeAndTrim(const QString& htmlEncodedString);
};

} // namespace scraper
} // namespace mediaelch
