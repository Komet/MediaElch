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

public:
    void search(QString searchStr) override;
    void loadData(QHash<MovieScraper*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;

    QSet<MovieScraperInfo> scraperNativelySupports() override;

    void changeLanguage(mediaelch::Locale locale) override;
    QVector<MovieScraper*> scrapersNeedSearch(QSet<MovieScraperInfo> infos,
        QHash<MovieScraper*, QString> alreadyLoadedIds);
    MovieScraper* titleScraper();
    QWidget* settingsWidget() override;
    MovieScraper* scraperForInfo(MovieScraperInfo info);

private slots:
    void onTitleSearchDone(QVector<ScraperSearchResult> results, ScraperError error);
    void onLoadTmdbFinished();

private:
    ScraperMeta m_meta;
    QVector<MovieScraper*> m_scrapers;
    mediaelch::network::NetworkManager m_network;

    QVector<MovieScraper*> scrapersForInfos(QSet<MovieScraperInfo> infos);
    ImageProvider* imageProviderForInfo(int info);
    QVector<ImageProvider*> imageProvidersForInfos(QSet<MovieScraperInfo> infos);

    QSet<MovieScraperInfo> infosForScraper(MovieScraper* scraper, QSet<MovieScraperInfo> selectedInfos);
    void loadAllData(QHash<MovieScraper*, QString> ids,
        Movie* movie,
        QSet<MovieScraperInfo> infos,
        QString tmdbId,
        QString imdbId);
    mediaelch::network::NetworkManager* network();
};

} // namespace scraper
} // namespace mediaelch
