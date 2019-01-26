#pragma once

#include "data/ImageProviderInterface.h"
#include "data/MovieScraperInterface.h"

#include <QNetworkAccessManager>
#include <QObject>

class CustomMovieScraper : public MovieScraperInterface
{
    Q_OBJECT
public:
    explicit CustomMovieScraper(QObject *parent = nullptr);
    static CustomMovieScraper *instance(QObject *parent = nullptr);

    QString name() const override;
    QString identifier() const override;
    void search(QString searchStr) override;
    void loadData(QMap<MovieScraperInterface *, QString> ids, Movie *movie, QVector<MovieScraperInfos> infos) override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings &settings) override;
    void saveSettings(ScraperSettings &settings) override;
    QVector<MovieScraperInfos> scraperSupports() override;
    QVector<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QVector<MovieScraperInterface *> scrapersNeedSearch(QVector<MovieScraperInfos> infos,
        QMap<MovieScraperInterface *, QString> alreadyLoadedIds);
    MovieScraperInterface *titleScraper();
    QWidget *settingsWidget() override;
    bool isAdult() const override;
    MovieScraperInterface *scraperForInfo(MovieScraperInfos info);

private slots:
    void onTitleSearchDone(QVector<ScraperSearchResult> results);
    void onLoadTmdbFinished();

signals:
    void searchDone(QVector<ScraperSearchResult>) override;

private:
    QVector<MovieScraperInterface *> m_scrapers;
    QNetworkAccessManager m_qnam;

    QVector<MovieScraperInterface *> scrapersForInfos(QVector<MovieScraperInfos> infos);
    ImageProviderInterface *imageProviderForInfo(int info);
    QVector<ImageProviderInterface *> imageProvidersForInfos(QVector<MovieScraperInfos> infos);

    QVector<MovieScraperInfos> infosForScraper(MovieScraperInterface *scraper,
        QVector<MovieScraperInfos> selectedInfos);
    void loadAllData(QMap<MovieScraperInterface *, QString> ids,
        Movie *movie,
        QVector<MovieScraperInfos> infos,
        QString tmdbId,
        QString imdbId);
    QNetworkAccessManager *qnam();
};
