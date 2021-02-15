#pragma once

#include "concerts/Concert.h"
#include "globals/Globals.h"
#include "globals/ScraperResult.h"
#include "movies/Movie.h"
#include "music/MusicBrainzId.h"
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
    /// \brief   Information object about the movie scraper.
    /// \details This object can be used to display details about the scraper.
    ///          For example in the "About" dialog for each scraper or similar.
    struct ScraperMeta
    {
        /// \brief Unique identifier used to store settings and more.
        /// \details The identifier must not be changed once set and is often the
        /// lowercase name of the data provider without spaces or other special characters.
        QString identifier;

        /// \brief Human readable name of the scraper. Often its title.
        QString name;

        /// \brief Short description of the scraper, i.e. a one-liner.
        QString description;

        /// \brief The data provider's website, e.g. https://kodi.tv
        QUrl website;

        /// \brief An URL to the provider's terms of service.
        QUrl termsOfService;

        /// \brief An URL to the data provider's data policy.
        QUrl privacyPolicy;

        /// \brief An URL to the data provider's contact page or forum.
        QUrl help;

        /// \brief Image types that are supported by the image provider.
        QSet<ImageType> supportedImageTypes;

        /// \brief A list of languages that are supported by the scraper.
        /// \see Locale::Locale
        QVector<Locale> supportedLanguages = {Locale::English};

        /// \brief Default locale for this scraper.
        Locale defaultLocale = Locale::English;
    };

public:
    ImageProvider(QObject* parent = nullptr) : QObject(parent) {}

    virtual const ScraperMeta& meta() const = 0;

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

    virtual void artistFanarts(MusicBrainzId mbId) = 0;
    virtual void artistLogos(MusicBrainzId mbId) = 0;
    virtual void artistThumbs(MusicBrainzId mbId) = 0;
    virtual void albumCdArts(MusicBrainzId mbId) = 0;
    virtual void albumThumbs(MusicBrainzId mbId) = 0;
    virtual void albumBooklets(MusicBrainzId mbId) = 0;
    virtual void artistImages(Artist* artist, MusicBrainzId mbId, QVector<ImageType> types) = 0;
    virtual void albumImages(Album* album, MusicBrainzId mbId, QVector<ImageType> types) = 0;

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
    void sigSearchDone(QVector<ScraperSearchResult>, mediaelch::ScraperError error);
    void sigImagesLoaded(QVector<Poster>, mediaelch::ScraperError error);
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
