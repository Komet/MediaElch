#pragma once

#include "movies/Movie.h"
#include "network/NetworkManager.h"
#include "scrapers/imdb/ImdbApi.h"
#include "scrapers/movie/MovieScraper.h"

#include <QNetworkReply>

class QCheckBox;

namespace mediaelch {
namespace scraper {

class ImdbMovie : public MovieScraper
{
    Q_OBJECT
public:
    explicit ImdbMovie(QObject* parent = nullptr);
    static constexpr const char* ID = "IMDb";

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;

public:
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;
    void changeLanguage(mediaelch::Locale locale) override;
    QWidget* settingsWidget() override;

private:
    ImdbApi m_api;
    ScraperMeta m_meta;
    QWidget* m_settingsWidget;
    QCheckBox* m_loadAllTagsWidget;

    bool m_loadAllTags = false;
    mediaelch::network::NetworkManager m_network;

    ScraperSearchResult parseIdFromMovieHtml(const QString& html);
};

} // namespace scraper
} // namespace mediaelch
