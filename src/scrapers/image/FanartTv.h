#pragma once

#include "globals/Globals.h"
#include "scrapers/image/ImageProviderInterface.h"
#include "scrapers/movie/MovieScraperInterface.h"

#include <QComboBox>
#include <QLineEdit>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVector>

class TMDb;
class TheTvDb;

/**
 * @brief The FanartTv class
 */
class FanartTv : public ImageProviderInterface
{
    Q_OBJECT
public:
    explicit FanartTv(QObject* parent = nullptr);
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
    void tvShowSeasonThumbs(TvDbId tvdbId, SeasonNumber season) override;
    void tvShowThumbs(TvDbId tvdbId) override;
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
    static void insertPoster(QVector<Poster>& posters, Poster b, QString language, QString preferredDiscType);

public slots:
    void searchMovie(QString searchStr, int limit = 0) override;
    void searchConcert(QString searchStr, int limit = 0) override;
    void searchTvShow(QString searchStr, int limit = 0) override;
    void searchArtist(QString searchStr, int limit = 0) override;
    void searchAlbum(QString artistName, QString searchStr, int limit = 0) override;

private slots:
    void onSearchMovieFinished(QVector<ScraperSearchResult> results, ScraperSearchError error);
    void onLoadMovieDataFinished();
    void onLoadAllMovieDataFinished();
    void onLoadAllConcertDataFinished();
    void onSearchTvShowFinished(QVector<ScraperSearchResult> results);
    void onLoadTvShowDataFinished();
    void onLoadAllTvShowDataFinished();

private:
    QVector<ImageType> m_provides;
    QString m_apiKey;
    QString m_personalApiKey;
    QNetworkAccessManager m_qnam;
    int m_searchResultLimit;
    TheTvDb* m_tvdb;
    TMDb* m_tmdb;
    QString m_language;
    QString m_preferredDiscType;
    QWidget* m_widget;
    QComboBox* m_box;
    QComboBox* m_discBox;
    QLineEdit* m_personalApiKeyEdit;

    QNetworkAccessManager* qnam();
    QVector<Poster> parseMovieData(QString json, ImageType type);
    void loadMovieData(TmdbId tmdbId, ImageType type);
    void loadMovieData(TmdbId tmdbId, QVector<ImageType> types, Movie* movie);
    void loadConcertData(TmdbId tmdbId, QVector<ImageType> types, Concert* concert);
    QVector<Poster> parseTvShowData(QString json, ImageType type, SeasonNumber season = SeasonNumber::NoSeason);
    void loadTvShowData(TvDbId tvdbId, ImageType type, SeasonNumber season = SeasonNumber::NoSeason);
    void loadTvShowData(TvDbId tvdbId, QVector<ImageType> types, TvShow* show);
    QString keyParameter();
};
