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

    QString name() override;
    QString identifier() override;
    void search(QString searchStr) override;
    void loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<int> infos) override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QList<int> scraperSupports() override;
    QList<int> scraperNativelySupports() override;
    QList<ScraperInterface *> scrapersNeedSearch(QList<int> infos, QMap<ScraperInterface *, QString> alreadyLoadedIds);
    ScraperInterface *titleScraper();
    QWidget *settingsWidget() override;
    bool isAdult() override;
    ScraperInterface *scraperForInfo(int info);

private slots:
    void onTitleSearchDone(QList<ScraperSearchResult> results);
    void onLoadTmdbFinished();

signals:
    void searchDone(QList<ScraperSearchResult>) override;

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
