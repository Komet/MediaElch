#ifndef THETVDBIMAGES_H
#define THETVDBIMAGES_H

#include "data/ImageProviderInterface.h"
#include "globals/Globals.h"

#include <QList>
#include <QObject>


class TvShow;
class TvShowEpisode;
class TheTvDb;

/**
 * @brief The TheTvDbImages class
 */
class TheTvDbImages : public ImageProviderInterface
{
    Q_OBJECT
public:
    explicit TheTvDbImages(QObject *parent = nullptr);
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
    void tvShowEpisodeThumb(QString tvdbId, int season, EpisodeNumber episode) override;
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
    void artistImages(Artist *artist, QString mbId, QList<ImageType> types) override;
    void albumImages(Album *album, QString mbId, QList<ImageType> types) override;
    void albumBooklets(QString mbId) override;
    QList<ImageType> provides() override;
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
    void sigImagesLoaded(Movie *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(Concert *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(TvShow *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(Artist *, QMap<ImageType, QList<Poster>>) override;
    void sigImagesLoaded(Album *, QMap<ImageType, QList<Poster>>) override;

private slots:
    void onSearchTvShowFinished(QList<ScraperSearchResult> results);
    void onLoadTvShowDataFinished();

private:
    QList<ImageType> m_provides;
    ImageType m_currentType;
    int m_searchResultLimit;
    TheTvDb *m_tvdb;
    TvShow *m_dummyShow;
    TvShowEpisode *m_dummyEpisode;
    int m_season;

    void loadTvShowData(QString tvdbId, ImageType type);
};

#endif // THETVDBIMAGES_H
