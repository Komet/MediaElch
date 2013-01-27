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
    void movieImages(Movie *movie, QString tmdbId, QList<int> types);
    void moviePosters(QString tmdbId);
    void movieBackdrops(QString tmdbId);
    void movieLogos(QString tmdbId);
    void movieClearArts(QString tmdbId);
    void movieCdArts(QString tmdbId);
    void concertImages(Concert *concert, QString tmdbId, QList<int> types);
    void concertPosters(QString tmdbId);
    void concertBackdrops(QString tmdbId);
    void concertLogos(QString tmdbId);
    void concertClearArts(QString tmdbId);
    void concertCdArts(QString tmdbId);
    void tvShowImages(TvShow *show, QString tvdbId, QList<int> types);
    void tvShowPosters(QString tvdbId);
    void tvShowBackdrops(QString tvdbId);
    void tvShowLogos(QString tvdbId);
    void tvShowClearArts(QString tvdbId);
    void tvShowCharacterArts(QString tvdbId);
    void tvShowBanners(QString tvdbId);
    void tvShowThumb(QString tvdbId, int season, int episode);
    void tvShowSeason(QString tvdbId, int season);
    void tvShowSeasonBanners(QString tvdbId, int season);
    void tvShowSeasonBackdrops(QString tvdbId, int season);
    QList<int> provides();

public slots:
    void searchMovie(QString searchStr, int limit = 0);
    void searchConcert(QString searchStr, int limit = 0);
    void searchTvShow(QString searchStr, int limit = 0);

signals:
    void sigSearchDone(QList<ScraperSearchResult>);
    void sigImagesLoaded(QList<Poster>);
    void sigImagesLoaded(Movie *, QMap<int, QList<Poster> >);
    void sigImagesLoaded(Concert *, QMap<int, QList<Poster> >);
    void sigImagesLoaded(TvShow *, QMap<int, QList<Poster> >);

private slots:
    void onSearchMovieFinished(QList<ScraperSearchResult> results);
    void onLoadMovieDataFinished();
    void onLoadAllMovieDataFinished();
    void onLoadAllConcertDataFinished();
    void onSearchTvShowFinished(QList<ScraperSearchResult> results);
    void onLoadTvShowDataFinished();
    void onLoadAllTvShowDataFinished();

private:
    QList<int> m_provides;
    QString m_apiKey;
    QNetworkAccessManager m_qnam;
    int m_searchResultLimit;
    TheTvDb *m_tvdb;
    TMDb *m_tmdb;

    QNetworkAccessManager *qnam();
    QList<Poster> parseMovieData(QString json, int type);
    void loadMovieData(QString tmdbId, int type);
    void loadMovieData(QString tmdbId, QList<int> types, Movie *movie);
    void loadConcertData(QString tmdbId, QList<int> types, Concert *concert);
    QList<Poster> parseTvShowData(QString json, int type);
    void loadTvShowData(QString tvdbId, int type);
    void loadTvShowData(QString tvdbId, QList<int> types, TvShow *show);
};

#endif // FANARTTV_H
