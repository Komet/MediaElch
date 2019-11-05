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
    void loadData(QMap<MovieScraperInterface*, QString> ids, Movie* movie, QVector<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QVector<MovieScraperInfos> scraperSupports() override;
    QVector<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QVector<MovieScraperInterface*> scrapersNeedSearch(QVector<MovieScraperInfos> infos,
        QMap<MovieScraperInterface*, QString> alreadyLoadedIds);
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

    QVector<MovieScraperInterface*> scrapersForInfos(QVector<MovieScraperInfos> infos);
    ImageProviderInterface* imageProviderForInfo(int info);
    QVector<ImageProviderInterface*> imageProvidersForInfos(QVector<MovieScraperInfos> infos);

    QVector<MovieScraperInfos> infosForScraper(MovieScraperInterface* scraper,
        QVector<MovieScraperInfos> selectedInfos);
    void loadAllData(QMap<MovieScraperInterface*, QString> ids,
        Movie* movie,
        QVector<MovieScraperInfos> infos,
        QString tmdbId,
        QString imdbId);
    QNetworkAccessManager* qnam();
};
