#ifndef FANARTTV_H
#define FANARTTV_H

#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

#include "data/ImageProviderInterface.h"
#include "globals/Globals.h"
#include "scrapers/TMDb.h"
#include "scrapers/TheTvDb.h"

/**
 * @brief The FanartTv class
 */
class FanartTv : public ImageProviderInterface
{
    Q_OBJECT
public:
    explicit FanartTv(QObject *parent = nullptr);
    QString name() override;
    QUrl siteUrl() override;
    QString identifier() override;
    void movieImages(Movie *movie, QString tmdbId, QList<ImageType> types) override;
    void moviePosters(QString tmdbId) override;
    void movieBackdrops(QString tmdbId) override;
    void movieLogos(QString tmdbId) override;
    void movieBanners(QString tmdbId) override;
    void movieThumbs(QString tmdbId) override;
    void movieClearArts(QString tmdbId) override;
    void movieCdArts(QString tmdbId) override;
    void concertImages(Concert *concert, QString tmdbId, QList<ImageType> types) override;
    void concertPosters(QString tmdbId) override;
    void concertBackdrops(QString tmdbId) override;
    void concertLogos(QString tmdbId) override;
    void concertClearArts(QString tmdbId) override;
    void concertCdArts(QString tmdbId) override;
    void tvShowImages(TvShow *show, QString tvdbId, QList<ImageType> types) override;
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
    void tvShowSeasonThumbs(QString tvdbId, int season) override;
    void tvShowThumbs(QString tvdbId) override;
    void artistFanarts(QString mbId) override;
    void artistLogos(QString mbId) override;
    void artistThumbs(QString mbId) override;
    void albumCdArts(QString mbId) override;
    void albumThumbs(QString mbId) override;
    void artistImages(Artist *artist, QString mbId, QList<ImageType> types) override;
    void albumImages(Album *album, QString mbId, QList<ImageType> types) override;
    void albumBooklets(QString mbId) override;
    QList<ImageType> provides() override;
    bool hasSettings() override;
    void loadSettings(QSettings &settings) override;
    void saveSettings(QSettings &settings) override;
    QWidget *settingsWidget() override;
    static void insertPoster(QList<Poster> &posters, Poster b, QString language, QString preferredDiscType);

public slots:
    void searchMovie(QString searchStr, int limit = 0) override;
    void searchConcert(QString searchStr, int limit = 0) override;
    void searchTvShow(QString searchStr, int limit = 0) override;
    void searchArtist(QString searchStr, int limit = 0) override;
    void searchAlbum(QString artistName, QString searchStr, int limit = 0) override;

signals:
    void sigSearchDone(QList<ScraperSearchResult>) override;
    void sigImagesLoaded(QList<Poster>) override;
    void sigImagesLoaded(Movie *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(Concert *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(TvShow *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(Artist *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(Album *, QMap<ImageType, QList<Poster>>) override;

private slots:
    void onSearchMovieFinished(QList<ScraperSearchResult> results);
    void onLoadMovieDataFinished();
    void onLoadAllMovieDataFinished();
    void onLoadAllConcertDataFinished();
    void onSearchTvShowFinished(QList<ScraperSearchResult> results);
    void onLoadTvShowDataFinished();
    void onLoadAllTvShowDataFinished();

private:
    QList<ImageType> m_provides;
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
    QList<Poster> parseMovieData(QString json, ImageType type);
    void loadMovieData(QString tmdbId, ImageType type);
    void loadMovieData(QString tmdbId, QList<ImageType> types, Movie *movie);
    void loadConcertData(QString tmdbId, QList<ImageType> types, Concert *concert);
    QList<Poster> parseTvShowData(QString json, ImageType type, int season = -2);
    void loadTvShowData(QString tvdbId, ImageType type, int season = -2);
    void loadTvShowData(QString tvdbId, QList<ImageType> types, TvShow *show);
    QString keyParameter();
};

#endif // FANARTTV_H
