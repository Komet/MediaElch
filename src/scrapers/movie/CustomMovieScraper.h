#pragma once

#include "network/NetworkManager.h"
#include "scrapers/image/ImageProviderInterface.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QObject>

class CustomMovieScraper : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit CustomMovieScraper(QObject* parent = nullptr);
    static constexpr const char* scraperIdentifier = "custom-movie";
    static CustomMovieScraper* instance(QObject* parent = nullptr);

    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QHash<MovieScraperInterface*, QString> ids, Movie* movie, QSet<MovieScraperInfo> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfo> scraperSupports() override;
    QSet<MovieScraperInfo> scraperNativelySupports() override;
    QVector<mediaelch::Locale> supportedLanguages() override;
    void changeLanguage(mediaelch::Locale locale) override;
    mediaelch::Locale defaultLanguage() override;
    QVector<MovieScraperInterface*> scrapersNeedSearch(QSet<MovieScraperInfo> infos,
        QHash<MovieScraperInterface*, QString> alreadyLoadedIds);
    MovieScraperInterface* titleScraper();
    QWidget* settingsWidget() override;
    bool isAdult() const override;
    MovieScraperInterface* scraperForInfo(MovieScraperInfo info);

private slots:
    void onTitleSearchDone(QVector<ScraperSearchResult> results, ScraperSearchError error);
    void onLoadTmdbFinished();

private:
    QVector<MovieScraperInterface*> m_scrapers;
    mediaelch::network::NetworkManager m_network;

    QVector<MovieScraperInterface*> scrapersForInfos(QSet<MovieScraperInfo> infos);
    ImageProviderInterface* imageProviderForInfo(int info);
    QVector<ImageProviderInterface*> imageProvidersForInfos(QSet<MovieScraperInfo> infos);

    QSet<MovieScraperInfo> infosForScraper(MovieScraperInterface* scraper, QSet<MovieScraperInfo> selectedInfos);
    void loadAllData(QHash<MovieScraperInterface*, QString> ids,
        Movie* movie,
        QSet<MovieScraperInfo> infos,
        QString tmdbId,
        QString imdbId);
    mediaelch::network::NetworkManager* network();
};
