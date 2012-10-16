#ifndef TMDBIMAGES_H
#define TMDBIMAGES_H

#include "data/ImageProviderInterface.h"
#include "data/Movie.h"
#include "scrapers/TMDb.h"

/**
 * @brief The TMDbImages class
 */
class TMDbImages : public ImageProviderInterface
{
    Q_OBJECT
public:
    explicit TMDbImages(QObject *parent = 0);
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
    void tvShowCharacterArts(QString tvdbId);
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
    void onLoadImagesFinished();

private:
    QList<int> m_provides;
    int m_searchResultLimit;
    TMDb *m_tmdb;
    Movie *m_dummyMovie;
    int m_imageType;
};

#endif // TMDBIMAGES_H
