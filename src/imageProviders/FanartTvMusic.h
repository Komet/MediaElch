#pragma once

#include "data/ImageProviderInterface.h"
#include "globals/Globals.h"

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>

class FanartTvMusic : public ImageProviderInterface
{
    Q_OBJECT
public:
    explicit FanartTvMusic(QObject *parent = nullptr);
    QString name() const override;
    QUrl siteUrl() const override;
    QString identifier() const override;
    void movieImages(Movie *movie, TmdbId tmdbId, QList<ImageType> types) override;
    void moviePosters(TmdbId tmdbId) override;
    void movieBackdrops(TmdbId tmdbId) override;
    void movieLogos(TmdbId tmdbId) override;
    void movieBanners(TmdbId tmdbId) override;
    void movieThumbs(TmdbId tmdbId) override;
    void movieClearArts(TmdbId tmdbId) override;
    void movieCdArts(TmdbId tmdbId) override;
    void concertImages(Concert *concert, TmdbId tmdbId, QList<ImageType> types) override;
    void concertPosters(TmdbId tmdbId) override;
    void concertBackdrops(TmdbId tmdbId) override;
    void concertLogos(TmdbId tmdbId) override;
    void concertClearArts(TmdbId tmdbId) override;
    void concertCdArts(TmdbId tmdbId) override;
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
    void tvShowThumbs(TvDbId tvdbId) override;
    void tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season) override;
    void artistFanarts(QString mbId) override;
    void artistLogos(QString mbId) override;
    void artistThumbs(QString mbId) override;
    void albumCdArts(QString mbId) override;
    void albumThumbs(QString mbId) override;
    void artistImages(Artist *artist, QString mbId, QList<ImageType> types) override;
    void albumImages(Album *album, QString mbId, QList<ImageType> types) override;
    void albumBooklets(QString mbId) override;
    QList<ImageType> provides() override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings &settings) override;
    void saveSettings(ScraperSettings &settings) override;
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
    void sigImagesLoaded(Movie *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(Concert *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(TvShow *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(Artist *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(Album *, QMap<ImageType, QList<Poster>>) override;

private slots:
    void onSearchArtistFinished();
    void onLoadArtistFinished();
    void onSearchAlbumFinished();
    void onLoadAlbumFinished();
    void onLoadAllArtistDataFinished();
    void onLoadAllAlbumDataFinished();

private:
    QList<ImageType> m_provides;
    QString m_apiKey;
    QString m_personalApiKey;
    QNetworkAccessManager m_qnam;
    int m_searchResultLimit;
    QString m_language;

    QNetworkAccessManager *qnam();
    QList<Poster> parseData(QString json, ImageType type);
    QString keyParameter();
};
