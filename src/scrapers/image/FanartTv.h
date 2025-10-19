#pragma once

#include "globals/Globals.h"
#include "network/NetworkManager.h"
#include "scrapers/image/ImageProvider.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/tmdb/TmdbMovieConfiguration.h"
#include "scrapers/tv_show/TvScraper.h"

#include <QMap>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVector>
#include <memory>

namespace mediaelch {
namespace scraper {

class TheTvDb;
class TmdbMovie;
class FanartTvConfiguration;

class FanartTv : public ImageProvider
{
    Q_OBJECT
public:
    static QString ID;

public:
    explicit FanartTv(FanartTvConfiguration& settings, QObject* parent = nullptr);
    ~FanartTv() override = default;

    const ScraperMeta& meta() const override;

    void movieImages(Movie* movie, TmdbId tmdbId, QSet<ImageType> types) override;
    void moviePosters(TmdbId tmdbId) override;
    void movieBackdrops(TmdbId tmdbId) override;
    void movieLogos(TmdbId tmdbId) override;
    void movieBanners(TmdbId tmdbId) override;
    void movieThumbs(TmdbId tmdbId) override;
    void movieClearArts(TmdbId tmdbId) override;
    void movieCdArts(TmdbId tmdbId) override;

    void concertImages(Concert* concert, TmdbId tmdbId, QSet<ImageType> types) override;
    void concertPosters(TmdbId tmdbId) override;
    void concertBackdrops(TmdbId tmdbId) override;
    void concertLogos(TmdbId tmdbId) override;
    void concertClearArts(TmdbId tmdbId) override;
    void concertCdArts(TmdbId tmdbId) override;

    void tvShowImages(TvShow* show, TvDbId tvdbId, QSet<ImageType> types, const mediaelch::Locale& locale) override;
    void tvShowPosters(TvDbId tvdbId, const mediaelch::Locale& locale) override;
    void tvShowBackdrops(TvDbId tvdbId, const mediaelch::Locale& locale) override;
    void tvShowLogos(TvDbId tvdbId, const mediaelch::Locale& locale) override;
    void tvShowClearArts(TvDbId tvdbId, const mediaelch::Locale& locale) override;
    void tvShowCharacterArts(TvDbId tvdbId, const mediaelch::Locale& locale) override;
    void tvShowBanners(TvDbId tvdbId, const mediaelch::Locale& locale) override;
    void tvShowEpisodeThumb(TvDbId tvdbId,
        SeasonNumber season,
        EpisodeNumber episode,
        const mediaelch::Locale& locale) override;
    void tvShowSeason(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale) override;
    void tvShowSeasonBanners(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale) override;
    void tvShowSeasonBackdrops(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale) override;
    void tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season, const mediaelch::Locale& locale) override;
    void tvShowThumbs(TvDbId tvdbId, const mediaelch::Locale& locale) override;

    void artistFanarts(MusicBrainzId mbId) override;
    void artistLogos(MusicBrainzId mbId) override;
    void artistThumbs(MusicBrainzId mbId) override;
    void albumCdArts(MusicBrainzId mbId) override;
    void albumThumbs(MusicBrainzId mbId) override;
    void artistImages(Artist* artist, MusicBrainzId mbId, QSet<ImageType> types) override;
    void albumImages(Album* album, MusicBrainzId mbId, QSet<ImageType> types) override;
    void albumBooklets(MusicBrainzId mbId) override;

    static void insertPoster(QVector<Poster>& posters, const Poster& b, const QVector<QString>& languagePriority, const QString& preferredDiscType);

public slots:
    void searchMovie(QString searchStr, int limit = 0) override;
    void searchConcert(QString searchStr, int limit = 0) override;
    void searchTvShow(QString searchStr, mediaelch::Locale locale, int limit = 0) override;
    void searchArtist(QString searchStr, int limit = 0) override;
    void searchAlbum(QString artistName, QString searchStr, int limit = 0) override;

private slots:
    void onSearchMovieFinished(mediaelch::scraper::MovieSearchJob* searchJob);
    void onLoadMovieDataFinished();
    void onLoadAllMovieDataFinished();
    void onLoadAllConcertDataFinished();
    void onSearchTvShowFinished(mediaelch::scraper::ShowSearchJob* searchJob);
    void onLoadTvShowDataFinished();
    void onLoadAllTvShowDataFinished();

private:
    ScraperMeta m_meta;
    FanartTvConfiguration& m_settings;

    QString m_apiKey;
    mediaelch::network::NetworkManager m_network;
    int m_searchResultLimit = 0;
    mediaelch::scraper::TheTvDb* m_tvdb = nullptr;
    mediaelch::scraper::ShowSearchJob* m_currentSearchJob = nullptr;
    std::unique_ptr<mediaelch::scraper::TmdbMovieConfiguration> m_tmdbConfig;
    mediaelch::scraper::TmdbMovie* m_tmdb;


    mediaelch::network::NetworkManager* network();
    QVector<Poster> parseMovieData(QString json, ImageType type);
    void loadMovieData(TmdbId tmdbId, ImageType type);
    void loadMovieData(TmdbId tmdbId, QSet<ImageType> types, Movie* movie);
    void loadConcertData(TmdbId tmdbId, QSet<ImageType> types, Concert* concert);
    QVector<Poster> parseTvShowData(QString json, ImageType type, SeasonNumber season = SeasonNumber::NoSeason);
    void loadTvShowData(TvDbId tvdbId, ImageType type, SeasonNumber season = SeasonNumber::NoSeason);
    void loadTvShowData(TvDbId tvdbId, QSet<ImageType> types, TvShow* show);
    QString keyParameter();
};

} // namespace scraper
} // namespace mediaelch
