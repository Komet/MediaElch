#ifndef FANARTTV_H
#define FANARTTV_H

#include "data/ImageProviderInterface.h"
#include "globals/Globals.h"

#include <QComboBox>
#include <QLineEdit>
#include <QList>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>

class TMDb;
class TheTvDb;

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
    void tvShowImages(TvShow *show, TvDbId tvdbId, QList<ImageType> types) override;
    void tvShowPosters(TvDbId tvdbId) override;
    void tvShowBackdrops(TvDbId tvdbId) override;
    void tvShowLogos(TvDbId tvdbId) override;
    void tvShowClearArts(TvDbId tvdbId) override;
    void tvShowCharacterArts(TvDbId tvdbId) override;
    void tvShowBanners(TvDbId tvdbId) override;
    void tvShowEpisodeThumb(TvDbId tvdbId, SeasonNumber season, EpisodeNumber episode) override;
    void tvShowSeason(TvDbId tvdbId, SeasonNumber season) override;
    void tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season) override;
    void tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season) override;
    void tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season) override;
    void tvShowThumbs(TvDbId tvdbId) override;
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
    QList<Poster> parseTvShowData(QString json, ImageType type, SeasonNumber season = SeasonNumber::NoSeason);
    void loadTvShowData(TvDbId tvdbId, ImageType type, SeasonNumber season = SeasonNumber::NoSeason);
    void loadTvShowData(TvDbId tvdbId, QList<ImageType> types, TvShow *show);
    QString keyParameter();
};

#endif // FANARTTV_H
