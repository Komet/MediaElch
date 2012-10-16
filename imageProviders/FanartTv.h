#ifndef FANARTTV_H
#define FANARTTV_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include "globals/Globals.h"
#include "data/ImageProviderInterface.h"
#include "scrapers/TMDb.h"
#include "scrapers/TheTvDb.h"

/**
 * @brief The FanartTv class
 */
class FanartTv : public ImageProviderInterface
{
    Q_OBJECT
public:
    explicit FanartTv(QObject *parent = 0);
    QString name();
    void moviePosters(QString tmdbId);
    void movieBackdrops(QString tmdbId);
    void movieLogos(QString tmdbId);
    void movieClearArts(QString tmdbId);
    void movieCdArts(QString tmdbId);
    void concertPosters(QString tmdbId);
    void concertBackdrops(QString tmdbId);
    void concertLogos(QString tmdbId);
    void concertClearArts(QString tmdbId);
    void concertCdArts(QString tmdbId);
    void tvShowPosters(QString tvdbId);
    void tvShowBackdrops(QString tvdbId);
    void tvShowLogos(QString tvdbId);
    void tvShowClearArts(QString tvdbId);
    void tvShowBanners(QString tvdbId);
    void tvShowThumb(QString tvdbId, int season, int episode);
    void tvShowSeason(QString tvdbId, int season);
    QList<int> provides();

public slots:
    void searchMovie(QString searchStr, int limit = 0);
    void searchConcert(QString searchStr, int limit = 0);
    void searchTvShow(QString searchStr, int limit = 0);

signals:
    void sigSearchDone(QList<ScraperSearchResult>);
    void sigImagesLoaded(QList<Poster>);

private slots:
    void onSearchMovieFinished(QList<ScraperSearchResult> results);
    void onLoadMovieDataFinished();
    void onSearchTvShowFinished(QList<ScraperSearchResult> results);
    void onLoadTvShowDataFinished();

private:
    QList<int> m_provides;
    QString m_apiKey;
    QNetworkAccessManager m_qnam;
    QNetworkReply *m_loadReply;
    int m_currentType;
    int m_searchResultLimit;
    TheTvDb *m_tvdb;
    TMDb *m_tmdb;

    QNetworkAccessManager *qnam();
    QList<Poster> parseMovieData(QString json, int type);
    void loadMovieData(QString tmdbId, int type);
    QList<Poster> parseTvShowData(QString json, int type);
    void loadTvShowData(QString tvdbId, int type);
};

#endif // FANARTTV_H
