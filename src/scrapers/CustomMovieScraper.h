#ifndef CUSTOMMOVIESCRAPER_H
#define CUSTOMMOVIESCRAPER_H

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
    void loadData(QMap<MovieScraperInterface *, QString> ids, Movie *movie, QList<MovieScraperInfos> infos) override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QList<MovieScraperInfos> scraperSupports() override;
    QList<MovieScraperInfos> scraperNativelySupports() override;
    std::vector<ScraperLanguage> supportedLanguages() override;
    void changeLanguage(QString languageKey) override;
    QString defaultLanguageKey() override;
    QList<MovieScraperInterface *> scrapersNeedSearch(QList<MovieScraperInfos> infos,
        QMap<MovieScraperInterface *, QString> alreadyLoadedIds);
    MovieScraperInterface *titleScraper();
    QWidget *settingsWidget() override;
    bool isAdult() const override;
    MovieScraperInterface *scraperForInfo(MovieScraperInfos info);

private slots:
    void onTitleSearchDone(QList<ScraperSearchResult> results);
    void onLoadTmdbFinished();

signals:
    void searchDone(QList<ScraperSearchResult>) override;

private:
    QList<MovieScraperInterface *> m_scrapers;
    QNetworkAccessManager m_qnam;

    QList<MovieScraperInterface *> scrapersForInfos(QList<MovieScraperInfos> infos);
    ImageProviderInterface *imageProviderForInfo(int info);
    QList<ImageProviderInterface *> imageProvidersForInfos(QList<MovieScraperInfos> infos);

    QList<MovieScraperInfos> infosForScraper(MovieScraperInterface *scraper, QList<MovieScraperInfos> selectedInfos);
    void loadAllData(QMap<MovieScraperInterface *, QString> ids,
        Movie *movie,
        QList<MovieScraperInfos> infos,
        QString tmdbId,
        QString imdbId);
    QNetworkAccessManager *qnam();
};

#endif // CUSTOMMOVIESCRAPER_H
