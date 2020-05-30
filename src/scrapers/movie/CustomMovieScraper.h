#pragma once

#include "scrapers/image/ImageProviderInterface.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QNetworkAccessManager>
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
    void loadData(QHash<MovieScraperInterface*, QString> ids, Movie* movie, QSet<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QSet<MovieScraperInfos> scraperSupports() override;
    QSet<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QVector<MovieScraperInterface*> scrapersNeedSearch(QSet<MovieScraperInfos> infos,
        QHash<MovieScraperInterface*, QString> alreadyLoadedIds);
    MovieScraperInterface* titleScraper();
    QWidget* settingsWidget() override;
    bool isAdult() const override;
    MovieScraperInterface* scraperForInfo(MovieScraperInfos info);

private slots:
    void onTitleSearchDone(QVector<ScraperSearchResult> results, ScraperSearchError error);
    void onLoadTmdbFinished();

private:
    QVector<MovieScraperInterface*> m_scrapers;
    QNetworkAccessManager m_qnam;

    QVector<MovieScraperInterface*> scrapersForInfos(QSet<MovieScraperInfos> infos);
    ImageProviderInterface* imageProviderForInfo(int info);
    QVector<ImageProviderInterface*> imageProvidersForInfos(QSet<MovieScraperInfos> infos);

    QSet<MovieScraperInfos> infosForScraper(MovieScraperInterface* scraper, QSet<MovieScraperInfos> selectedInfos);
    void loadAllData(QHash<MovieScraperInterface*, QString> ids,
        Movie* movie,
        QSet<MovieScraperInfos> infos,
        QString tmdbId,
        QString imdbId);
    QNetworkAccessManager* qnam();
};
