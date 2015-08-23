#ifndef FANARTTV_H
#define FANARTTV_H

#include <QLineEdit>
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
    QUrl siteUrl();
    QString identifier();
    void movieImages(Movie *movie, QString tmdbId, QList<int> types);
    void moviePosters(QString tmdbId);
    void movieBackdrops(QString tmdbId);
    void movieLogos(QString tmdbId);
    void movieBanners(QString tmdbId);
    void movieThumbs(QString tmdbId);
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
    void tvShowEpisodeThumb(QString tvdbId, int season, int episode);
    void tvShowSeason(QString tvdbId, int season);
    void tvShowSeasonBanners(QString tvdbId, int season);
    void tvShowSeasonBackdrops(QString tvdbId, int season);
    void tvShowSeasonThumbs(QString tvdbId, int season);
    void tvShowThumbs(QString tvdbId);
    void artistFanarts(QString mbId);
    void artistLogos(QString mbId);
    void artistThumbs(QString mbId);
    void albumCdArts(QString mbId);
    void albumThumbs(QString mbId);
    void artistImages(Artist *artist, QString mbId, QList<int> types);
    void albumImages(Album *album, QString mbId, QList<int> types);
    void albumBooklets(QString mbId);
    QList<int> provides();
    bool hasSettings();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings);
    QWidget *settingsWidget();
    static void insertPoster(QList<Poster> &posters, Poster b, QString language, QString preferredDiscType);

public slots:
    void searchMovie(QString searchStr, int limit = 0);
    void searchConcert(QString searchStr, int limit = 0);
    void searchTvShow(QString searchStr, int limit = 0);
    void searchArtist(QString searchStr, int limit = 0);
    void searchAlbum(QString artistName, QString searchStr, int limit = 0);

signals:
    void sigSearchDone(QList<ScraperSearchResult>);
    void sigImagesLoaded(QList<Poster>);
    void sigImagesLoaded(Movie *, QMap<int, QList<Poster> >);
    void sigImagesLoaded(Concert *, QMap<int, QList<Poster> >);
    void sigImagesLoaded(TvShow *, QMap<int, QList<Poster> >);
    void sigImagesLoaded(Artist *, QMap<int, QList<Poster> >);
    void sigImagesLoaded(Album *, QMap<int, QList<Poster> >);

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
    QString m_personalApiKey;
    QNetworkAccessManager m_qnam;
    int m_searchResultLimit;
    TheTvDb *m_tvdb;
    TMDb *m_tmdb;
    QString m_language;
    QString m_preferredDiscType;
    QWidget *m_widget;
    QComboBox *m_box;
    QComboBox *m_discBox;
    QLineEdit *m_personalApiKeyEdit;

    QNetworkAccessManager *qnam();
    QList<Poster> parseMovieData(QString json, int type);
    void loadMovieData(QString tmdbId, int type);
    void loadMovieData(QString tmdbId, QList<int> types, Movie *movie);
    void loadConcertData(QString tmdbId, QList<int> types, Concert *concert);
    QList<Poster> parseTvShowData(QString json, int type, int season = -2);
    void loadTvShowData(QString tvdbId, int type, int season = -2);
    void loadTvShowData(QString tvdbId, QList<int> types, TvShow *show);
    QString keyParameter();
};

#endif // FANARTTV_H
