#ifndef CUSTOMMOVIESCRAPER_H
#define CUSTOMMOVIESCRAPER_H

#include "data/ImageProviderInterface.h"
#include "data/ScraperInterface.h"

#include <QNetworkAccessManager>
#include <QObject>

class CustomMovieScraper : public ScraperInterface
{
    Q_OBJECT
public:
    explicit CustomMovieScraper(QObject *parent = nullptr);
    static CustomMovieScraper *instance(QObject *parent = nullptr);

    QString name() override;
    QString identifier() override;
    void search(QString searchStr) override;
    void loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos) override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QList<MovieScraperInfos> scraperSupports() override;
    QList<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    QList<ScraperInterface *> scrapersNeedSearch(QList<MovieScraperInfos> infos,
        QMap<ScraperInterface *, QString> alreadyLoadedIds);
    ScraperInterface *titleScraper();
    QWidget *settingsWidget() override;
    bool isAdult() override;
    ScraperInterface *scraperForInfo(MovieScraperInfos info);

private slots:
    void onTitleSearchDone(QList<ScraperSearchResult> results);
    void onLoadTmdbFinished();

signals:
    void searchDone(QList<ScraperSearchResult>) override;

private:
    QList<ScraperInterface *> m_scrapers;
    QNetworkAccessManager m_qnam;

    QList<ScraperInterface *> scrapersForInfos(QList<MovieScraperInfos> infos);
    ImageProviderInterface *imageProviderForInfo(int info);
    QList<ImageProviderInterface *> imageProvidersForInfos(QList<MovieScraperInfos> infos);

    QList<MovieScraperInfos> infosForScraper(ScraperInterface *scraper, QList<MovieScraperInfos> selectedInfos);
    void loadAllData(QMap<ScraperInterface *, QString> ids,
        Movie *movie,
        QList<MovieScraperInfos> infos,
        QString tmdbId,
        QString imdbId);
    QNetworkAccessManager *qnam();
};

#endif // CUSTOMMOVIESCRAPER_H
