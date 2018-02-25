#ifndef CUSTOMMOVIESCRAPER_H
#define CUSTOMMOVIESCRAPER_H

#include <QObject>

#include "data/ImageProviderInterface.h"
#include "data/ScraperInterface.h"

class CustomMovieScraper : public ScraperInterface
{
    Q_OBJECT
public:
    explicit CustomMovieScraper(QObject *parent = nullptr);
    static CustomMovieScraper *instance(QObject *parent = nullptr);

    QString name();
    QString identifier();
    void search(QString searchStr);
    void loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<int> infos);
    bool hasSettings();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);
    QList<int> scraperSupports();
    QList<int> scraperNativelySupports();
    QList<ScraperInterface *> scrapersNeedSearch(QList<int> infos, QMap<ScraperInterface *, QString> alreadyLoadedIds);
    ScraperInterface *titleScraper();
    QWidget *settingsWidget();
    bool isAdult();
    ScraperInterface *scraperForInfo(int info);

private slots:
    void onTitleSearchDone(QList<ScraperSearchResult> results);
    void onLoadTmdbFinished();

signals:
    void searchDone(QList<ScraperSearchResult>);

private:
    QList<ScraperInterface *> m_scrapers;
    QNetworkAccessManager m_qnam;

    QList<ScraperInterface *> scrapersForInfos(QList<int> infos);
    ImageProviderInterface *imageProviderForInfo(int info);
    QList<ImageProviderInterface *> imageProvidersForInfos(QList<int> infos);

    QList<int> infosForScraper(ScraperInterface *scraper, QList<int> selectedInfos);
    void
    loadAllData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<int> infos, QString tmdbId, QString imdbId);
    QNetworkAccessManager *qnam();
};

#endif // CUSTOMMOVIESCRAPER_H
