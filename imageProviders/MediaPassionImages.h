#ifndef MEDIAPASSIONIMAGES_H
#define MEDIAPASSIONIMAGES_H

#include "data/ImageProviderInterface.h"
#include "movies/Movie.h"
#include "scrapers/MediaPassion.h"

class MediaPassionImages : public ImageProviderInterface
{
    Q_OBJECT
public:
    explicit MediaPassionImages(QObject *parent = nullptr);
    QString name() override;
    QUrl siteUrl() override;
    QString identifier() override;
    void movieImages(Movie *movie, QString tmdbId, QList<int> types) override;
    void moviePosters(QString tmdbId) override;
    void movieBackdrops(QString tmdbId) override;
    void movieLogos(QString tmdbId) override;
    void movieBanners(QString tmdbId) override;
    void movieThumbs(QString tmdbId) override;
    void movieClearArts(QString tmdbId) override;
    void movieCdArts(QString tmdbId) override;
    void concertImages(Concert *concert, QString tmdbId, QList<int> types) override;
    void concertPosters(QString tmdbId) override;
    void concertBackdrops(QString tmdbId) override;
    void concertLogos(QString tmdbId) override;
    void concertClearArts(QString tmdbId) override;
    void concertCdArts(QString tmdbId) override;
    void tvShowImages(TvShow *show, QString tvdbId, QList<int> types) override;
    void tvShowPosters(QString tvdbId) override;
    void tvShowBackdrops(QString tvdbId) override;
    void tvShowLogos(QString tvdbId) override;
    void tvShowClearArts(QString tvdbId) override;
    void tvShowCharacterArts(QString tvdbId) override;
    void tvShowBanners(QString tvdbId) override;
    void tvShowEpisodeThumb(QString tvdbId, int season, int episode) override;
    void tvShowSeason(QString tvdbId, int season) override;
    void tvShowSeasonBanners(QString tvdbId, int season) override;
    void tvShowSeasonBackdrops(QString tvdbId, int season) override;
    void tvShowThumbs(QString tvdbId) override;
    void tvShowSeasonThumbs(QString tvdbId, int season) override;
    void artistFanarts(QString mbId) override;
    void artistLogos(QString mbId) override;
    void artistThumbs(QString mbId) override;
    void albumCdArts(QString mbId) override;
    void albumThumbs(QString mbId) override;
    void artistImages(Artist *artist, QString mbId, QList<int> types) override;
    void albumImages(Album *album, QString mbId, QList<int> types) override;
    void albumBooklets(QString mbId) override;
    QList<int> provides() override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QWidget *settingsWidget() override;

public slots:
    void searchMovie(QString searchStr, int limit = 0) override;
    void searchConcert(QString searchStr, int limit = 0) override;
    void searchTvShow(QString searchStr, int limit = 0) override;
    void searchArtist(QString searchStr, int limit = 0) override;
    void searchAlbum(QString artistName, QString searchStr, int limit = 0) override;

signals:
    void sigSearchDone(QList<ScraperSearchResult>) override;
    void sigImagesLoaded(QList<Poster>) override;
    void sigImagesLoaded(Movie *, QMap<int, QList<Poster>>) override;
    void sigImagesLoaded(Concert *, QMap<int, QList<Poster>>) override;
    void sigImagesLoaded(TvShow *, QMap<int, QList<Poster>>) override;
    void sigImagesLoaded(Artist *, QMap<int, QList<Poster>>) override;
    void sigImagesLoaded(Album *, QMap<int, QList<Poster>>) override;

private slots:
    void onSearchMovieFinished(QList<ScraperSearchResult> results);
    void onLoadImagesFinished();

private:
    QList<int> m_provides;
    MediaPassion *m_mediaPassion;
    Movie *m_dummyMovie;
    int m_imageType;
    int m_searchResultLimit;
};

#endif // MEDIAPASSIONIMAGES_H
