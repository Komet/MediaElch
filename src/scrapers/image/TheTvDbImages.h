#pragma once

#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "scrapers/image/ImageProviderInterface.h"

#include <QObject>
#include <QVector>


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
    explicit TheTvDbImages(QObject* parent = nullptr);
    QString name() const override;
    QUrl siteUrl() const override;
    QString identifier() const override;
    void movieImages(Movie* movie, TmdbId tmdbId, QVector<ImageType> types) override;
    void moviePosters(TmdbId tmdbId) override;
    void movieBackdrops(TmdbId tmdbId) override;
    void movieLogos(TmdbId tmdbId) override;
    void movieBanners(TmdbId tmdbId) override;
    void movieThumbs(TmdbId tmdbId) override;
    void movieClearArts(TmdbId tmdbId) override;
    void movieCdArts(TmdbId tmdbId) override;
    void concertImages(Concert* concert, TmdbId tmdbId, QVector<ImageType> types) override;
    void concertPosters(TmdbId tmdbId) override;
    void concertBackdrops(TmdbId tmdbId) override;
    void concertLogos(TmdbId tmdbId) override;
    void concertClearArts(TmdbId tmdbId) override;
    void concertCdArts(TmdbId tmdbId) override;
    void tvShowImages(TvShow* show, TvDbId tvdbId, QVector<ImageType> types) override;
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
    void artistImages(Artist* artist, QString mbId, QVector<ImageType> types) override;
    void albumImages(Album* album, QString mbId, QVector<ImageType> types) override;
    void albumBooklets(QString mbId) override;
    QVector<ImageType> provides() override;
    bool hasSettings() const override;
    void loadSettings(const ScraperSettings& settings) override;
    void saveSettings(ScraperSettings& settings) override;
    QWidget* settingsWidget() override;

public slots:
    void searchMovie(QString searchStr, int limit = 0) override;
    void searchConcert(QString searchStr, int limit = 0) override;
    void searchTvShow(QString searchStr, int limit = 0) override;
    void searchArtist(QString searchStr, int limit = 0) override;
    void searchAlbum(QString artistName, QString searchStr, int limit = 0) override;

private slots:
    void onSearchTvShowFinished(QVector<ScraperSearchResult> results);
    void onLoadTvShowDataFinished();

private:
    QVector<ImageType> m_provides;
    ImageType m_currentType = ImageType::None;
    int m_searchResultLimit = 0;
    TheTvDb* m_tvdb = nullptr;
    TvShow* m_dummyShow = nullptr;
    TvShowEpisode* m_dummyEpisode = nullptr;
    SeasonNumber m_season;

    void loadTvShowData(TvDbId tvdbId, ImageType type);
};
