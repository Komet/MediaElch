#pragma once

#include "network/NetworkManager.h"
#include "scrapers/image/ImageProvider.h"
#include "scrapers/movie/MovieScraper.h"

#include <QObject>


namespace mediaelch {
namespace scraper {

class CustomMovieScraper : public MovieScraper
{
    Q_OBJECT
public:
    explicit CustomMovieScraper(QObject* parent = nullptr);
    static constexpr const char* ID = "custom-movie";
    static CustomMovieScraper* instance(QObject* parent = nullptr);

    const ScraperMeta& meta() const override;

    void initialize() override;
    bool isInitialized() const override;

    ELCH_NODISCARD MovieSearchJob* search(MovieSearchJob::Config config) override;
    /// \brief   Returns a movie scrape job that needs to be customized before being executed.
    /// \details Because the custom movie scraper does not use a single scraper
    ///          but multiple ones, multiple movie identifiers are required.
    ///          This is why the returned CustomMovieScrapeJob has customization endpoints.
    /// \see     CustomMovieScrapeJob
    ELCH_NODISCARD MovieScrapeJob* loadMovie(MovieScrapeJob::Config config) override;

public:
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;
    QVector<MovieScraper*> scrapersNeedSearch(QSet<MovieScraperInfo> infos,
        QHash<MovieScraper*, MovieIdentifier> alreadyLoadedIds);
    MovieScraper* titleScraper();
    QWidget* settingsWidget() override;
    MovieScraper* scraperForInfo(MovieScraperInfo info);

private slots:
    void onLoadTmdbFinished();

private:
    ScraperMeta m_meta;
    QVector<MovieScraper*> m_scrapers;
    mediaelch::network::NetworkManager m_network;

    QVector<MovieScraper*> scrapersForInfos(QSet<MovieScraperInfo> infos);
    ImageProvider* imageProviderForInfo(int info);
    QVector<ImageProvider*> imageProvidersForInfos(QSet<MovieScraperInfo> infos);

    QSet<MovieScraperInfo> infosForScraper(MovieScraper* scraper, QSet<MovieScraperInfo> selectedInfos);
    void loadAllData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
        Movie* movie,
        const QSet<MovieScraperInfo>& infos,
        TmdbId tmdbId,
        ImdbId imdbId);
    mediaelch::network::NetworkManager* network();
};

} // namespace scraper
} // namespace mediaelch
