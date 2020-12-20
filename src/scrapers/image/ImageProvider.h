#pragma once

#include "concerts/Concert.h"
#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "movies/Movie.h"
#include "scrapers/ScraperInterface.h"
#include "settings/ScraperSettings.h"
#include "tv_shows/EpisodeNumber.h"
#include "tv_shows/SeasonNumber.h"
#include "tv_shows/TvShow.h"

#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>

namespace mediaelch {
namespace scraper {

class ImageProvider : public QObject, public ScraperInterface
{
    Q_OBJECT

public:
    ImageProvider(QObject* parent = nullptr) : QObject(parent) {}

    virtual QString name() const = 0;
    virtual QString identifier() const = 0;
    virtual QUrl siteUrl() const = 0;
    virtual mediaelch::Locale defaultLanguage() = 0;
    virtual const QVector<mediaelch::Locale>& supportedLanguages() = 0;
    virtual void movieImages(Movie* movie, TmdbId tmdbId, QVector<ImageType> types) = 0;
    virtual void moviePosters(TmdbId tmdbId) = 0;
    virtual void movieBackdrops(TmdbId tmdbId) = 0;
    virtual void movieLogos(TmdbId tmdbId) = 0;
    virtual void movieBanners(TmdbId tmdbId) = 0;
    virtual void movieThumbs(TmdbId tmdbId) = 0;
    virtual void movieClearArts(TmdbId tmdbId) = 0;
    virtual void movieCdArts(TmdbId tmdbId) = 0;
    virtual void concertImages(Concert* concert, TmdbId tmdbId, QVector<ImageType> types) = 0;
    virtual void concertPosters(TmdbId tmdbId) = 0;
    virtual void concertBackdrops(TmdbId tmdbId) = 0;
    virtual void concertLogos(TmdbId tmdbId) = 0;
    virtual void concertClearArts(TmdbId tmdbId) = 0;
    virtual void concertCdArts(TmdbId tmdbId) = 0;
    virtual void
    tvShowImages(TvShow* show, TvDbId tvdbId, QVector<ImageType> types, const mediaelch::Locale& locale) = 0;
    virtual void tvShowPosters(TvDbId tvdbId, const mediaelch::Locale& locale) = 0;
    virtual void tvShowBackdrops(TvDbId tvdbId, const mediaelch::Locale& locale) = 0;
    virtual void tvShowLogos(TvDbId tvdbId, const mediaelch::Locale& locale) = 0;
    virtual void tvShowClearArts(TvDbId tvdbId, const mediaelch::Locale& locale) = 0;
    virtual void tvShowCharacterArts(TvDbId tvdbId, const mediaelch::Locale& locale) = 0;
    virtual void tvShowBanners(TvDbId tvdbId, const mediaelch::Locale& locale) = 0;
    virtual void
    tvShowEpisodeThumb(TvDbId tvdbId, SeasonNumber season, EpisodeNumber episode, const mediaelch::Locale& locale) = 0;
    virtual void tvShowSeason(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale) = 0;
    virtual void tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale) = 0;
    virtual void tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale) = 0;
    virtual void tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale) = 0;
    virtual void tvShowThumbs(TvDbId tvdbId, const mediaelch::Locale& locale) = 0;
    virtual void artistFanarts(QString mbId) = 0;
    virtual void artistLogos(QString mbId) = 0;
    virtual void artistThumbs(QString mbId) = 0;
    virtual void albumCdArts(QString mbId) = 0;
    virtual void albumThumbs(QString mbId) = 0;
    virtual void albumBooklets(QString mbId) = 0;
    virtual void artistImages(Artist* artist, QString mbId, QVector<ImageType> types) = 0;
    virtual void albumImages(Album* album, QString mbId, QVector<ImageType> types) = 0;
    virtual QSet<ImageType> provides() = 0;
    bool hasSettings() const override = 0;
    void loadSettings(ScraperSettings& settings) override = 0;
    void saveSettings(ScraperSettings& settings) override = 0;
    virtual QWidget* settingsWidget() = 0;

public slots:
    virtual void searchMovie(QString searchStr, int limit) = 0;
    virtual void searchConcert(QString searchStr, int limit) = 0;
    virtual void searchTvShow(QString searchStr, mediaelch::Locale locale, int limit) = 0;
    virtual void searchArtist(QString searchStr, int limit) = 0;
    virtual void searchAlbum(QString artistName, QString searchStr, int limit) = 0;

signals:
    void sigSearchDone(QVector<ScraperSearchResult>, ScraperError error);
    void sigImagesLoaded(QVector<Poster>, ScraperError error);
    void sigMovieImagesLoaded(Movie*, QMap<ImageType, QVector<Poster>>);
    void sigConcertImagesLoaded(Concert*, QMap<ImageType, QVector<Poster>>);
    void sigTvShowImagesLoaded(TvShow*, QMap<ImageType, QVector<Poster>>);
    void sigArtistImagesLoaded(Artist*, QMap<ImageType, QVector<Poster>>);
    void sigAlbumImagesLoaded(Album*, QMap<ImageType, QVector<Poster>>);
};

} // namespace scraper
} // namespace mediaelch


Q_DECLARE_METATYPE(mediaelch::scraper::ImageProvider*)
Q_DECLARE_OPAQUE_POINTER(mediaelch::scraper::ImageProvider*)
